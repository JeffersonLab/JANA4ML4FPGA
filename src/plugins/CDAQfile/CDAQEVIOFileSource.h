//
// Created by xmei@jlab.org on 2/9/23.
//

#ifndef JANA4ML4FPGA_JEVENTSOURCEEVIOFILE_H
#define JANA4ML4FPGA_JEVENTSOURCEEVIOFILE_H

#include <string>
#include <atomic>
#include <chrono>
#include <cinttypes>

#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <JANA/Compatibility/jerror.h>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JFactory.h>
#include <JANA/Compatibility/JStreamLog.h>

#include "../../libraries/evio/HDEVIO.h"
#include "../../libraries/evio/DModuleType.h"

// put a real path for simplicity
/*
 * [hdtrdops@gluon200 ~]$ ls /gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_00
hd_rawdata_002539_000.evio  hd_rawdata_002539_003.evio  hd_rawdata_002539_006.evio
hd_rawdata_002539_001.evio  hd_rawdata_002539_004.evio  hd_rawdata_002539_007.evio
hd_rawdata_002539_002.evio  hd_rawdata_002539_005.evio
 */
#define TEST_FILEPATH "/gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_007.evio"



// This class is copied from /rawdataparser/JEventSource_EVIOpp but remove some contents.

class CDAQEVIOFileSource : public JEventSource {

public:
    enum EVIOSourceType{
        kNoSource,
        kFileSource,
        kETSource
    };

    enum EmulationModeType{
        kEmulationNone,
        kEmulationAlways,
        kEmulationAuto
    };

    bool DONE;
    bool DISPATCHER_END;
    std::chrono::high_resolution_clock::time_point tstart;
    std::chrono::high_resolution_clock::time_point tend;

    uint32_t BLOCKS_TO_SKIP;
    uint32_t MAX_PARSED_EVENTS;
    mutex PARSED_EVENTS_MUTEX;

    std::atomic<uint_fast64_t> NEVENTS_PROCESSED;
    std::atomic<uint_fast64_t> NDISPATCHER_STALLED;
    std::atomic<uint_fast64_t> NPARSER_STALLED;
    std::atomic<uint_fast64_t> NEVENTBUFF_STALLED;

    uint64_t MAX_EVENT_RECYCLES;
    uint64_t MAX_OBJECT_RECYCLES;

    EVIOSourceType source_type;
    HDEVIO *hdevio;
    bool et_quit_next_timeout;

    // vector<DEVIOWorkerThread*> worker_threads;
    thread *dispatcher_thread;

    JStreamLog evioout;

    uint32_t F250_EMULATION_MODE; // (EmulationModeType)
    uint32_t F125_EMULATION_MODE; // (EmulationModeType)
    uint32_t F250_EMULATION_VERSION;

    bool RECORD_CALL_STACK;
    set<uint32_t> ROCIDS_TO_PARSE;

    bool     PARSE;
    bool     PARSE_F250;
    bool     PARSE_F125;
    bool     PARSE_F1TDC;
    bool     PARSE_CAEN1290TDC;
    bool     PARSE_CONFIG;
    bool     PARSE_BOR;
    bool     PARSE_EPICS;
    bool     PARSE_EVENTTAG;
    bool     PARSE_TRIGGER;
    bool     PARSE_SSP;
    bool     PARSE_GEMSRS;
    int      NSAMPLES_GEMSRS;
    bool     APPLY_TRANSLATION_TABLE;
    int      ET_STATION_NEVENTS;
    bool     ET_STATION_CREATE_BLOCKING;
    bool     LOOP_FOREVER;
    uint32_t USER_RUN_NUMBER;
    int      VERBOSE;
    int      VERBOSE_ET;
    float    TIMEOUT;
    uint32_t NTHREADS;
    bool     PRINT_STATS;
    bool     SWAP;
    bool     LINK;
    bool     LINK_TRIGGERTIME;
    bool     LINK_BORCONFIG;
    bool     LINK_CONFIG;
    bool     IGNORE_EMPTY_BOR;
    bool     TREAT_TRUNCATED_AS_ERROR;
    std::string   SYSTEMS_TO_PARSE;
    int      SYSTEMS_TO_PARSE_FORCE;

    uint32_t jobtype;
    bool IS_CDAQ_FILE = false;

    CDAQEVIOFileSource(std::string resource_name, JApplication* app);

    virtual ~CDAQEVIOFileSource();

    void Open() override;

    void GetEvent(std::shared_ptr<JEvent>);  //TODO

    static std::string GetDescription();

private:
    void SetEVIODefaultConfigParams(JApplication* app);

    void OpenEVIOFile(std::string filename);

    uint64_t SearchFileForRunNumber(void);
};

template <>
double JEventSourceGeneratorT<CDAQEVIOFileSource>::CheckOpenable(std::string);

#endif //JANA4ML4FPGA_JEVENTSOURCEEVIOFILE_H
