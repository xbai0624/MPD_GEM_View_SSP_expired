#ifndef GEM_ANALYZER_H_
#define GEM_ANALYZER_H_

#include "EvioFileReader.h"
#include "EventParser.h"
#include "MPDVMERawEventDecoder.h"
#include "MPDSSPRawEventDecoder.h"
#include "MPDDataStruct.h"
#include "hardcode.h"

#include <TH1I.h>

#include <string>
#include <unordered_map>

class GEMAnalyzer
{
public:
    GEMAnalyzer();
    ~GEMAnalyzer();

    void Init();
    void AnalyzeEvent(int event);
    const std::unordered_map<APVAddress, TH1I*> & GetHistos() const;
    const std::unordered_map<APVAddress, std::vector<int>> & GetData() const;
    void FillHistos(const std::unordered_map<APVAddress, std::vector<int>> &);
    void Clear();
    void ClearPreviousEvent();
    void GeneratePedestal(const char*);

    // setters
    void SetFile(const char* path);
    void SetMaxEvents(uint32_t);
    void CloseFile();

private:
    EvioFileReader *pFileReader;
    EventParser *pEventParser;
#ifdef USE_VME
    MPDVMERawEventDecoder *pRawEventDecoder;
#else
    MPDSSPRawEventDecoder *pRawEventDecoder;
#endif

    std::string fFile;
    uint32_t nEvents = 5000;
    std::unordered_map<APVAddress, std::vector<int>> rawData;
    std::unordered_map<APVAddress, TH1I*> rawHistos;
};

#endif
