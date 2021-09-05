#ifndef GEM_DATA_HANDLER_H
#define GEM_DATA_HANDLER_H

#include <string>
#include <vector>
#include <deque>
#include <thread>
#include "GEMStruct.h"
#include "EventParser.h"
#include "EvioFileReader.h"
#include "GEMAPV.h"

class GEMSystem;
class GEMRootHitTree;
class GEMRootClusterTree;
class MPDVMERawEventDecoder;
class MPDSSPRawEventDecoder;

class GEMDataHandler
{
public:
    GEMDataHandler();

    // copy/move constructors
    GEMDataHandler(const GEMDataHandler &that);
    GEMDataHandler(GEMDataHandler &&that);

    // destructor
    ~GEMDataHandler();

    // copy/move assignment
    GEMDataHandler &operator=(const GEMDataHandler &rhs);
    GEMDataHandler &operator=(GEMDataHandler &&rhs);

    // set systems
    void SetGEMSystem(GEMSystem *gem){gem_sys = gem;}
    GEMSystem *GetGEMSystem() const {return gem_sys;}

    // file reading and writing
    void Decode(const void *buffer);
    // read from multiple evio splits
    int ReadFromSplitEvio(const std::string &path, int split_start = 0,
            int split_end = -1, bool verbose = false);
    // read from single evio
    int ReadFromEvio(const std::string &path, int split=-1, bool verbose = false);
    // interface member
    void Replay(const std::string &r_path, int split_start = 0, int split_end = -1,
            const std::string &pedestal_input_file = "",
            const std::string &common_mode_input_file = "",
            const std::string &pedestal_output_file = "", 
            const std::string &commonMode_output_file = "");
    void Reset();

    // data handler
    void Clear();
    void StartOfNewEvent(const unsigned char &tag);
    void EndofThisEvent(const int &ev);
    void EndProcess(EventData *data);
    void FillHistograms(const EventData &data);

    // feeding data
    void FeedDataSRS(const GEMRawData &gemData);
    void FeedDataMPD(const APVAddress &addr, const std::vector<int> &raw_data, const uint32_t &flags);
    void FeedData(const std::vector<GEMZeroSupData> &gemData);

    // event storage
    unsigned int GetEventCount() const {return event_data.size();}
    const EventData &GetEvent(const unsigned int &index) const;
    const std::deque<EventData> &GetEventData() const {return event_data;}

    // analysis tools
    int FindEvent(int event_number) const;

    // test functions
    void ReplayEvent_test(const uint32_t *pBuf, const uint32_t &fBufLen, const int &ev_number);
    void SetMode();
    void SetPedestalMode(bool m){pedestalMode = m; replayMode = !m; onlineMode = !m;}
    void SetReplayMode(bool m){replayMode = m; pedestalMode = !m; onlineMode = !m;}
    void SetOnlineMode(bool m){onlineMode = m; pedestalMode = !m; onlineMode = !m;}
    void TurnOffClustering(){bReplayCluster = false;}
    void TurnOnClustering(){bReplayCluster = true;}

    // helpers
    std::string ParseOutputFileName(const std::string &input_file_name, const char* prefix="Rootfiles/hit");

private:
    void waitEventProcess();

private:
    EvioFileReader *evio_reader;
    EventParser *event_parser;
    GEMSystem *gem_sys;
    std::thread end_thread;
    bool pedestalMode = false;
    bool replayMode = true;
    bool onlineMode = false;

    // decoders
    MPDVMERawEventDecoder *mpd_vme_decoder = nullptr;
    MPDSSPRawEventDecoder *mpd_ssp_decoder = nullptr;

    // data related
    std::deque<EventData> event_data;
    EventData *new_event;
    EventData *proc_event;

    // pedestal generate
    std::string pedestal_output_file = "database/gem_ped.dat";
    std::string commonMode_output_file = "database/CommonModeRange.txt";

    // replay data to root hit tree
    GEMRootHitTree *root_hit_tree = nullptr;
    std::string replay_hit_output_file = "";
    int fEventNumber = 0;

    // replay data to root cluster tree
    GEMRootClusterTree *root_cluster_tree = nullptr;
    std::string replay_cluster_output_file = "";
    bool bReplayCluster = false;
};

#endif
