#include "GEMAnalyzer.h"
#include "RolStruct.h"
#include "GEMPedestal.h"

#include <iostream>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
// ctor

GEMAnalyzer::GEMAnalyzer()
{
    // place holder
}

////////////////////////////////////////////////////////////////////////////////
// dtor

GEMAnalyzer::~GEMAnalyzer()
{
    Clear();
}

////////////////////////////////////////////////////////////////////////////////
// init

void GEMAnalyzer::Init()
{
    if(fFile.size() == 0) {
        std::cout<<"GEMAnalyzer Error: the file to analyze is not set."
                 <<std::endl;
        return;
    }

    // set up evio file reader
    pFileReader = new EvioFileReader();
    pFileReader -> SetFileOpenMode("r"); // random access
    pFileReader -> SetFile(fFile);
    pFileReader -> OpenFile();

    // set up event parser
    pEventParser = new EventParser();

    // init decoder
#ifdef USE_VME
    pRawEventDecoder = new MPDVMERawEventDecoder();
    // register decoder
    pEventParser -> RegisterRawDecoder(static_cast<int>(Bank_TagID::MPD_VME), pRawEventDecoder);
#else
    pRawEventDecoder = new MPDSSPRawEventDecoder();
    // register decoder
    pEventParser -> RegisterRawDecoder(static_cast<int>(Bank_TagID::MPD_SSP), pRawEventDecoder);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// set file to analyzer

void GEMAnalyzer::SetFile(const char* path)
{
    fFile = path;
}

////////////////////////////////////////////////////////////////////////////////
// analzyer event 

void GEMAnalyzer::AnalyzeEvent([[maybe_unused]] int event)
{
    ClearPreviousEvent();

    const uint32_t *pBuf;
    uint32_t fBufLen;

    //if((pFileReader->ReadEventNum(&pBuf, &fBufLen, event)) != S_SUCCESS)
    if((pFileReader->ReadNoCopy(&pBuf, &fBufLen)) != S_SUCCESS)
    {
        std::cout<<"Error: cannot open event."<<std::endl;
        return;
    }

    pEventParser -> ParseEvent(pBuf, fBufLen);

    [[maybe_unused]] auto & decoded_data = pRawEventDecoder->GetAPV();

    FillHistos(decoded_data);
}

////////////////////////////////////////////////////////////////////////////////
// get the analyzed histos

const std::unordered_map<APVAddress, TH1I*> & GEMAnalyzer::GetHistos() const
{
    return rawHistos;
}

////////////////////////////////////////////////////////////////////////////////
// clear

void GEMAnalyzer::Clear()
{
    for(auto &i: rawHistos)
        if(i.second) i.second->Delete();

    rawData.clear();

    delete pEventParser;
    delete pRawEventDecoder;
}

////////////////////////////////////////////////////////////////////////////////
// clear previous event

void GEMAnalyzer::ClearPreviousEvent()
{
    for(auto &i: rawHistos)
        if(i.second) i.second -> Reset();

    rawData.clear();
}

////////////////////////////////////////////////////////////////////////////////
// close file

void GEMAnalyzer::CloseFile()
{
    pFileReader->CloseFile();
}

////////////////////////////////////////////////////////////////////////////////
// fill event histos

void GEMAnalyzer::FillHistos(const std::unordered_map<APVAddress, std::vector<int>> &event_data)
{
    int nAPV = 0;
    for(auto &i: event_data)
    {
        if(rawHistos.find(i.first) == rawHistos.end()){
            int nBins = static_cast<int>(i.second.size());
            rawHistos[i.first] = new TH1I(
                    Form("h%d_crate%d_mpd%d_adc%d", nAPV, i.first.crate_id, i.first.mpd_id, i.first.adc_ch), 
                    Form("crate_%d_mpd_%d_adc_ch_%d", i.first.crate_id, i.first.mpd_id, i.first.adc_ch), 
                    nBins, 0, nBins);
        }
        else
            rawHistos[i.first] -> Reset();

        nAPV++;

        int ts = 0;
        for(auto &j: i.second)
        {
            rawHistos[i.first] -> SetBinContent(ts, j);
            rawData[i.first].push_back(j);
            ts++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// fill event histos

const std::unordered_map<APVAddress, std::vector<int>> & GEMAnalyzer::GetData() const
{
    return rawData;
}

////////////////////////////////////////////////////////////////////////////////
// set max number of events to analyze

void GEMAnalyzer::SetMaxEvents(uint32_t num)
{
    nEvents = num;
}

 
////////////////////////////////////////////////////////////////////////////////
// generate pedestal

void GEMAnalyzer::GeneratePedestal(const char* save_path)
{
    GEMPedestal *pedestal = new GEMPedestal();

    pedestal->SetDataFile(fFile.c_str());
    pedestal->SetNumberOfEvents(nEvents);
    pedestal->CalculatePedestal();
    pedestal->SavePedestalHisto(save_path);
}
