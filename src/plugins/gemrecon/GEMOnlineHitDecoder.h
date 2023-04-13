#ifndef _GEMONLINEHITDECODER_H__
#define _GEMONLINEHITDECODER_H__

#include "GEMRawDecoder.h"
#include "GemMapping.h"
#include "GEMHit.h"
#include "GEMPedestal.h"
#include "GEMCluster.h"

#include <stdint.h>

#include "TString.h"
#include "TObject.h"
#include <TStyle.h>
#include <TSystem.h>
#include <TROOT.h>
#include "TH1F.h"
#include "TH2F.h"
#include "TList.h"
#include "TSystem.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TMath.h"

#include <map>
#include <list>
#include <vector>
#include <numeric>
#include "SFclust.h"

//vector <SFclust> clusters;

class GEMOnlineHitDecoder {
public:

    GEMOnlineHitDecoder(Int_t iEntry, Int_t nbOfTS, Int_t startTS, Int_t stopTS, Int_t zeroSupCut, Int_t comModeCut,
                        TString isPeakOrSum, TString centralOrAllStrips, Float_t minADCs, Int_t minClustSize,
                        Int_t maxClustSize, Int_t maxClustMult);

    ~GEMOnlineHitDecoder();

public:

    int GetClusters(TString plane, std::vector<SFclust> &clust);

    void ProcessEvent(std::map<int, std::map<int, std::vector<int> > >, GEMPedestal *);

    void GetListOfClustersFromPlanes();

    void GetHit(TString plane, TH1F *theHist);

    void GetClusterHit(TString plane, TH1F *theHist);

    void GetTimeBinClusterHit(TString plane, TH2F *the2dHist);

    void GetListOfHitsFromPlanes();

    void GetListOfHitsCleanFromPlanes();

    void FillADCvsDriftTimeAndPositionForLargestCluster(TString plane, TH2F *the2dHist, TH2F *adcHist);

    void FillADCvsDriftTimeAndPositionForAllClusters(TString plane, TH2F *the2dHist, TH2F *adcHist);

    void FillHitHistos(TString plane, TH1F *posHist, TH1F *adcUniHist);

    void FillClusterHistos(TString plane, TH1F *posHist, TH1F *adcHist, TH1F *adcDistHist, TH1F *clusterSizeHist,
                           TH1F *clusterMultHist);

    void Fill2DClusterHistos(TString det, TH2F *pos2DHist, TH2F *adc2DHist, TH2F *tSampleHist, TH2F *chShareingHist,
                             TH1F *chRatioHist);

    void
    FillPos2D(TH2F *posHist, TH2F *adcHist, TH2F *tSampleHist, Float_t xpos, Float_t ypos, Float_t adc, Float_t timing);

    void AverageADCgain(TH1F *posHist, TH1F *adcUniHist, Float_t pos, Float_t adc);

    void APVEventDecoder();

    void APVEventSplitChannelsDecoder();

    void ComputeClusters();

    void DeleteClustersInPlaneMap();

    Bool_t IsADCchannelActive();

    Bool_t IsGoodEventFound() {
        Bool_t isGoodEventFound = kFALSE;
        if (fEventNb % 5 == 0) isGoodEventFound = kTRUE;
        return isGoodEventFound;
    }

private:

    uint32_t *buf;
    int fSize, fEventNb, fAPVID, fAPVHeaderLevel, fComModeCut, fStartData;
    uint8_t fMaxClusterMult, fMinClusterSize, fMaxClusterSize;
    uint8_t fNbOfTimeSamples, fStartTimeSamples, fStopTimeSamples;
    uint8_t NCH, fFECID, fADCChannel, fAPVKey, fZeroSupCut;
    TString fIsHitMaxOrTotalADCs, fIsCentralOrAllStripsADCs, fAPVStatus;
    Float_t fMinADCvalue, fAPVBaseline;

    GemMapping *fMapping;
    std::set<int> FECs;

    std::map<Int_t, GEMHit *> fListOfHits, fListOfHitsClean;
    std::map<TString, std::list<GEMHit *> > fListOfHitsFromPlane, fListOfHitsCleanFromPlane;
    std::map<TString, std::list<GEMCluster *> > fListOfClustersCleanFromPlane;
    std::map<int, std::map<int, std::vector<int> > > mSrsSingleEvent;

    std::vector<int> fRawData16bits, fActiveADCchannels;
    std::vector<Float_t> fPedestalNoises, fPedestalNoises_1stSet, fPedestalNoises_2ndSet;
    std::vector<Float_t> fPedestalOffsets, fPedestalOffsets_1stSet, fPedestalOffsets_2ndSet;

    Bool_t fIsGoodClusterEvent;

};

#endif
