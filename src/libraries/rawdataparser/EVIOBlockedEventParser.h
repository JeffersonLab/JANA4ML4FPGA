

#pragma once


#include <vector>
#include <memory>

#include <JANA/Utils/JEventPool.h>
#include <rawdataparser/EVIOBlockedEvent.h>

// #include <rawdataparser/DStatusBits.h>
#include <rawdataparser/daq_param_type.h>
#include <rawdataparser/DModuleType.h>


#include <rawdataparser/Df250Config.h>
#include <rawdataparser/Df250PulseIntegral.h>
// #include <rawdataparser/Df250StreamingRawData.h>
#include <rawdataparser/Df250WindowSum.h>
// #include <rawdataparser/Df250PulseRawData.h>
#include <rawdataparser/Df250TriggerTime.h>
#include <rawdataparser/Df250PulseTime.h>
#include <rawdataparser/Df250PulsePedestal.h>
#include <rawdataparser/Df250PulseData.h>
// #include <rawdataparser/Df250WindowRawData.h>
#include <rawdataparser/Df125Config.h>
#include <rawdataparser/Df125TriggerTime.h>
#include <rawdataparser/Df125PulseIntegral.h>
#include <rawdataparser/Df125PulseTime.h>
#include <rawdataparser/Df125PulsePedestal.h>
// #include <rawdataparser/Df125PulseRawData.h>
#include <rawdataparser/Df125WindowRawData.h>
#include <rawdataparser/Df125CDCPulse.h>
#include <rawdataparser/Df125FDCPulse.h>
#include <rawdataparser/DF1TDCConfig.h>
// #include <rawdataparser/DF1TDCHit.h>
// #include <rawdataparser/DF1TDCTriggerTime.h>
#include <rawdataparser/DCAEN1290TDCConfig.h>
// #include <rawdataparser/DCAEN1290TDCHit.h>
// #include <rawdataparser/DCODAEventInfo.h>
#include <rawdataparser/DCODAControlEvent.h>
#include <rawdataparser/DCODAROCInfo.h>
// #include <rawdataparser/Df250Scaler.h>
// #include <rawdataparser/DL1Info.h>
#include <rawdataparser/DEPICSvalue.h>
// #include <rawdataparser/DEventTag.h>
// #include <rawdataparser/Df250BORConfig.h>
// #include <rawdataparser/Df125BORConfig.h>
// #include <rawdataparser/DF1TDCBORConfig.h>
// #include <rawdataparser/DCAEN1290TDCBORConfig.h>
// #include <rawdataparser/DTSGBORConfig.h>
// #include <rawdataparser/DDIRCTriggerTime.h>
// #include <rawdataparser/DDIRCTDCHit.h>
// #include <rawdataparser/DDIRCADCHit.h>
#include <rawdataparser/DGEMSRSWindowRawData.h>
// #include <rawdataparser/DBORptrs.h>

class EVIOBlockedEventParser{

    std::vector <std::shared_ptr<JEvent>> events;
    size_t ievent_idx; // keeps track of the next event in the "events" vector to use while parsing

    int VERBOSE = 1;
    bool PARSE_EPICS = true;
    bool PARSE_TRIGGER = true;
    bool PARSE_CONFIG = true;
    bool PARSE_GEMSRS = true;
    bool PARSE_F250 = true;
    bool PARSE_F125 = true;

    const int32_t NSAMPLES_GEMSRS     = 9;

public:

    EVIOBlockedEventParser();
    ~EVIOBlockedEventParser();

    std::vector <std::shared_ptr<JEvent>> ParseEVIOBlockedEvent(EVIOBlockedEvent &block, JEventPool &pool);

    void ParseBank(uint32_t *istart, uint32_t *iend);
    void ParseEPICSbank(uint32_t* &iptr, uint32_t *iend);
    void ParseBORbank(uint32_t* &iptr, uint32_t *iend);
    void ParseControlEvent(uint32_t* &iptr, uint32_t *iend);
    void ParsePhysicsBank(uint32_t* &iptr, uint32_t *iend);
    void ParseCDAQBank(uint32_t* &iptr, uint32_t *iend);
    void ParseDataBank(uint32_t* &iptr, uint32_t *iend);

    void ParseCAEN1190(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);
    void ParseModuleConfiguration(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);
    void ParseEventTagBank(uint32_t *&iptr, uint32_t *iend);
    void ParseJLabModuleData(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);
    void ParseSSPBank(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);
    void ParseTSscalerBank(uint32_t *&iptr, uint32_t *iend);
    void Parsef250scalerBank(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);
    void ParseRawTriggerBank(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);
    void ParseDGEMSRSBank(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);

    void MakeDGEMSRSWindowRawData(JEvent *event, uint32_t rocid, uint32_t slot, uint32_t itrigger, uint32_t apv_id, vector<int>rawData16bits);
    void Parsef250Bank(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);
    void Parsef125Bank(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);
    void MakeDf125WindowRawData(JEvent *event, uint32_t rocid, uint32_t slot, uint32_t itrigger, uint32_t* &iptr);
    void ParseF1TDCBank(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);
    void ParseTIBank(uint32_t rocid, uint32_t *&iptr, uint32_t *iend);

    void DumpBinary(const uint32_t *iptr, const uint32_t *iend, uint32_t MaxWords, const uint32_t *imark);
};

