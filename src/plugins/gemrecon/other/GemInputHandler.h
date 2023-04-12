#ifndef GEMINPUTHANDLER_H
#define GEMINPUTHANDLER_H

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stdio.h> // for getchar()
#include "TCanvas.h"
#include <TH1F.h>
#include <TH2F.h>
#include <cstdlib>
//#include "et.h"
#include "evioUtil.hxx"
#include "evioFileChannel.hxx"
//#include <QProgressBar>

#include "GemMapping.h"
#include "GemException.h"
#include "GEMRawDecoder.h"
#include "GEMPedestal.h"
#include "SFclust.h"

using namespace evio;

#define ET_CHUNK_SIZE 50

class GemInputHandler {
public:

    GemInputHandler(TCanvas *c);//offline file analysis
    GemInputHandler(const char *ipAddr, int tcpPort, const char *etFile,
                    size_t size = 1048576) throw(GemException);//online monitoring
    ~GemInputHandler();

    void CreateStation(std::string stName, int mode) throw(GemException);

    void AttachStation() throw(GemException);

    void DetachStation();

    bool Read() throw(GemException);

    void *GetBuffer() { return (void *) fBuffer; };

    size_t GetBufferLength() { return fBufferSize; };

    int QuickCheckETEvent();

    int newEvent();

    void SetZeroSupCut(Int_t zeroSupCut) { fZeroSupCut = zeroSupCut; }

    void SetCommonModeCut(Int_t comModeCut) { fComModeCut = comModeCut; }

    void SetMinADCvalue(Int_t minADCvalue) { fMinADCvalue = minADCvalue; }

    void SetMinClusterSize(Int_t minClusterSize) { fMinClusterSize = minClusterSize; }

    void SetMaxClusterSize(Int_t maxClusterSize) { fMaxClusterSize = maxClusterSize; }

    void SetMaxClusterMult(Int_t maxClusterMult) { fMaxClusterMult = maxClusterMult; }

    void SetNbOfTimeSamples(Int_t nbOfTimeSamples) { fNbOfTimeSamples = nbOfTimeSamples; }

    void SetStopTimeSample(Int_t stopTimeSamples) { fStopTimeSample = stopTimeSamples; }

    void SetStartTimeSample(Int_t startTimeSamples) { fStartTimeSample = startTimeSamples; }

    void SetHitPeakOrSumADCs(TString peakOrSum) { fIsHitPeakOrSumADCs = peakOrSum; }

    void SetCentralOrAllStripsADCs(TString val) { fIsCentralOrAllStripsADCs = val; }

    int ProcessRawDataFromFile(char *filename, int iEntry);

    int ProcessETEvent(TH1F **hist, TH2F **hist2d);

    int ProcessSingleEventFromFile(char *filename, int iEntry, TH1F **hist, TH2F **hist2d);

    //  int ProcessPedestals(TString dataFileName, unsigned int minEntry, unsigned int maxEntry, QProgressBar* theBar) ;
    int ProcessPedestals(TString dataFileName, unsigned int minEntry, unsigned int maxEntry);

    int ProcessMultiEventsFromFile(TString filename, unsigned int minEntry, unsigned int maxEntry, TH1F **ADCHist,
                                   TH1F **HitHist, TH1F **ClusterHist, TH1F **ClusterInfoHist, TH2F **pos2DHist,
                                   TH2F **chargeSharingHist, TH1F **chargeRatioHist, TH2F **timeBinPosHist,
                                   TH2F **adcTimeBinPosHist);

    //  int ProcessMultiEventsFromFile(TString filename, unsigned int minEntry, unsigned int maxEntry, TH1F** ADCHist, TH1F** HitHist, TH1F** ClusterHist, TH1F** ClusterInfoHist, TH2F** pos2DHist, TH2F** chargeSharingHist, TH1F** chargeRatioHist, TH2F** timeBinPosHist, TH2F** adcTimeBinPosHist, QProgressBar* theBar);
    int ProcessSearchEventFromFile(char *filename, TH1F **hist, TH2F **hist2d);

    int ProcessSingleEventFromBank(TH1F **hist, TH2F **hist2d, evioDOMNodeP bankPtr, vector<SFclust> &gemclust);

    void Restart() { fEventFound = false; }

    bool IsEventFound() const { return fEventFound; }
    //  float GetCurrentProgress() const { return fOfflineProgress; }

    void InitPedestal(GEMPedestal *pedestal) {
        fPedestal = pedestal;
        fPedestal->LoadPedestal();
    }

    map<int, map<int, vector<int> > > fCurrentEvent;


    typedef struct {
        unsigned int length;
        unsigned char num;
        unsigned char type;
        unsigned short tag;
    } PRadEventHeader;

    TCanvas *c3;;

private:

    int parseEventByHeader(PRadEventHeader *header);

    unsigned int fSrsStart, fSrsEnd;
    //  float   fOfflineProgress;
    TString fIsHitPeakOrSumADCs, fIsCentralOrAllStripsADCs;
    Float_t fMinADCvalue;
    int fEventNumber, fLastEvent, fZeroSupCut, fComModeCut, fNbOfTimeSamples, fStopTimeSample, fStartTimeSample;
    int fMinClusterSize, fMaxClusterSize, fMaxClusterMult;

    //member variables used in online mode
    /*
    et_att_id  attachID;
    et_stat_id stationID;
    et_sys_id  etID;
    et_event  *etEvent;
    */
    uint32_t *fBuffer;
    size_t fBufferSize;
    bool fEventFound, fIsFirstDecodedData;

    GemMapping *fMapping;
    GEMPedestal *fPedestal;

    map<TString, map<Int_t, vector<Float_t> > > fDetectorClusterMap;
    map<int, std::vector<int> > fGEMEvent;

    enum PRadEventType {
        PhysicsType1 = 0x01,
        PhysicsType2 = 0x02,
        PhysicsGEMType = 0x81,
        PreStartEvent = 0x11,
        GoEvent = 0x12,
        EndEvent = 0x20,
    };

    enum PRadHeaderType {
        UnsignedInt32bit = 0x01,
        EvioBank = 0x10,
        EvioBank_B = 0x0e,
    };

    enum PRadROCID {
        PRadTS = 2,
        PRadROC_4 = 4,
        PRadROC_5 = 5,
        PRadROC_6 = 6,
        PRadGEMROC_1 = 7,
    };

    enum PRadBankID {
        TI_BANK = 0x4,
        FASTBUS_BANK = 0x7,
        EVINFO_BANK = 0xc000,
        GEMMONITOR_BANK_1 = 0xe11f,
        GEMMONITOR_BANK_2 = 0xa,
        GEMMONITOR_BANK_3 = 0xb,
        GEMMONITOR_BANK_4 = 0xc,
        GEMMONITOR_BANK_5 = 0xd,
        GEMMONITOR_BANK_6 = 0xe,
        GEMMONITOR_BANK_7 = 0xf,
        GEMMONITOR_BANK_8 = 0x10,
    };
};

#endif







