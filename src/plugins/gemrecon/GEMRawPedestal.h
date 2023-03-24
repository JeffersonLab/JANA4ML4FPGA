#ifndef __GEMRAWPEDESTAL_H__
#define __GEMRAWPEDESTAL_H__

#include <TString.h>
#include <map>
#include <unordered_map>
#include <vector>

class GemMapping;

class GEMRawPedestal {
public:
    GEMRawPedestal(Int_t nbOfTimeSamples);

    GEMRawPedestal(std::map<int, std::vector<int> >, Int_t nbOfTimeSamples);

    ~GEMRawPedestal();

    void ClearMap();

    void ClearSingleEvent();

    void ClearStripRMS();

    void ClearStripOffset();

    void ApvEventDecode();

    void ComputeApvPedestal(int);

    void ComputeEventPedestal();

    void ComputeEventPedestal(std::map<int, std::vector<int> >);

    void PrintEventPedestal(); // for debug
    float GetStripNoise(int apvindex, int channelID);

    float GetStripOffset(int apvindex, int channelID);

    int CheckApvTimeSample();

private:
    float fApvHeaderLevel;
    int NCH;
    int fNbOfTimeSamples;

    int fAPVID;
    TString fAPVStatus;
    int fFECID;
    int fADCCh;

    GemMapping *mapping;

    std::map<int, std::vector<int> > mSrsSingleEvent;
    std::vector<int> vSingleApvData;
    std::map<int, std::vector<float> > mStripOffset;
    std::map<int, std::vector<float> > mStripRMS;

    std::vector<float> vCommonModeOffset;
    std::vector<float> vCommonModeOffset_split;
    std::multimap<int, float> mApvTimeBinData;
};

#endif
