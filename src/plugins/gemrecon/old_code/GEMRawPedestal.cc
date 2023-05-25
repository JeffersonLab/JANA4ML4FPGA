#include "GEMRawPedestal.h"
#include "plugins/gemrecon/GemMapping.h"
#include <TMath.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <cassert>

using namespace std;

static string trim(const string &str, const string &w = " \t\n\r") {
    const auto strBegin = str.find_first_not_of(w);
    if (strBegin == string::npos) return ""; // no content
    const auto strEnd = str.find_last_not_of(w);
    const auto strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
}

GEMRawPedestal::GEMRawPedestal(Int_t nbOfTimeSamples) {
    mapping = GemMapping::GetInstance();
    mSrsSingleEvent.clear();
    vSingleApvData.clear();
    mStripOffset.clear();
    mStripRMS.clear();
    fApvHeaderLevel = 1500.0;
    fNbOfTimeSamples = nbOfTimeSamples;
    fAPVStatus = "normal";
    fAPVID = 0xffff;
    NCH = 128;
    fFECID = 999;
    fADCCh = 999;
}

GEMRawPedestal::GEMRawPedestal(map<int, vector<int> > r_map, Int_t nbOfTimeSamples) {
    //  printf(" GEMRawPedestal::GEMRawPedestal(map<int, vector<int> > r_map, Int_t nbOfTimeSamples) ==> enter \n") ;
    mapping = GemMapping::GetInstance();
    mSrsSingleEvent.clear();
    mSrsSingleEvent = r_map;
    vSingleApvData.clear();
    mStripOffset.clear();
    mStripRMS.clear();

    fApvHeaderLevel = 1500.0;
    fNbOfTimeSamples = nbOfTimeSamples;
    fAPVStatus = "normal";
    fAPVID = 0xffff;
    NCH = 128;

    fFECID = 999;
    fADCCh = 999;
    ComputeEventPedestal();
}

void GEMRawPedestal::ComputeEventPedestal(map<int, vector<int> > r_map) {
    // printf(" GEMRawPedestal::ComputeEventPedestal(map<int, vector<int> > r_map) ==> enter \n") ;
    ClearMap();
    vSingleApvData.clear();
    mSrsSingleEvent = r_map;
    ComputeEventPedestal();
    //  printf(" GEMRawPedestal::ComputeEventPedestal() ==> leave \n") ;
}

GEMRawPedestal::~GEMRawPedestal() {
    ClearMap();
    vSingleApvData.clear();
}

void GEMRawPedestal::ClearMap() {
    ClearSingleEvent();
    ClearStripOffset();
    ClearStripRMS();
}

void GEMRawPedestal::ClearSingleEvent() {
    for (auto &i: mSrsSingleEvent) i.second.clear();
    mSrsSingleEvent.clear();
}

void GEMRawPedestal::ClearStripOffset() {
    for (auto &i: mStripOffset) i.second.clear();
    mStripOffset.clear();
}

void GEMRawPedestal::ClearStripRMS() {
    for (auto &i: mStripRMS) i.second.clear();
    mStripRMS.clear();
}

void GEMRawPedestal::ComputeEventPedestal() {
    // cout << " GEMRawPedestal::ComputeEventPedestal()  enter " << endl;
    for (auto &i: mSrsSingleEvent) {
        fAPVID = i.first;
        fADCCh = fAPVID & 0xf;
        fFECID = (fAPVID >> 4) & 0xf;
        fApvHeaderLevel = mapping->GetAPVHeaderLevelFromID(fAPVID);
        fAPVStatus = mapping->GetAPVstatus(fAPVID);
        vSingleApvData = i.second;
        //cout << " GEMRawPedestal::ComputeEventPedestal()  vSingleApvData = " << (i.second).at((i.second).size()-1)<< endl;
        ApvEventDecode();
        ComputeApvPedestal(fAPVID);
    }
}

void GEMRawPedestal::ApvEventDecode() {
    if (!CheckApvTimeSample()) return;
    //cout << " ApvEventDecode()  enter " << endl;
    int idata = 0;
    int startDataFlag = 0;
    int size = vSingleApvData.size();
    vCommonModeOffset.clear();
    vCommonModeOffset_split.clear();
    mApvTimeBinData.clear();
    string apv_status = fAPVStatus.Data();
    apv_status = trim(apv_status);

    std::list<float> commonModeOffset;
    std::list<float> commonModeOffset_split;

    int nTB = 0;
    while (idata < size) {
        ///---------------------- --------------------------------//
        ///   look for apv header  => 3 consecutive words < header//
        ///   ----------------------------------------------------//
        if (vSingleApvData[idata] < fApvHeaderLevel) {
            idata++;
            if (vSingleApvData[idata] < fApvHeaderLevel) {
                idata++;
                if (vSingleApvData[idata] < fApvHeaderLevel) {
                    if (idata + 138 < size) {
                        idata += 10; //8 address + 1 error bit
                        startDataFlag = 1;
                        nTB++;
                        continue;
                    }
                }
            }
        }

        //--------------------------------------
        //       meaningful data
        //--------------------------------------
        if (startDataFlag) {
            commonModeOffset.clear();
            commonModeOffset_split.clear();
            float commonMode = 0;
            float commonMode_split = 0;

            for (int chNo = 0; chNo < NCH; ++chNo) {
                float rawdata = vSingleApvData[idata];

                if ((apv_status != "normal") && (mapping->PRadStripMapping(fAPVID, chNo) < 16))
                    commonModeOffset_split.push_back(rawdata);
                else commonModeOffset.push_back(rawdata);

                mApvTimeBinData.insert(pair<int, float>(chNo, rawdata));
                idata++;
            }

            commonModeOffset.sort();
            commonModeOffset_split.sort();
            commonMode = TMath::Mean(commonModeOffset.begin(), commonModeOffset.end());
            commonMode_split = TMath::Mean(commonModeOffset_split.begin(), commonModeOffset_split.end());

            vCommonModeOffset.push_back(commonMode);
            vCommonModeOffset_split.push_back(commonMode_split);
            startDataFlag = 0;
            continue;
        }
        idata++;
    }
}

int GEMRawPedestal::CheckApvTimeSample() {
    //----------------------------------------------------------//
    //  check time samples, if not right, discard current data  //
    //----------------------------------------------------------//
    vector<int> iheader;
    iheader.clear();
    int size = vSingleApvData.size();
    //cout << "CheckApvTimeSample():: vSingleApvData.size()=" << size << " fApvHeaderLevel=" << fApvHeaderLevel <<endl;
    int i = 0;
    int idata = 0;
    int nbTimeSample = 0;

    //for (int i =0 ; i<20; i++ ) {
    //    cout << "CheckApvTimeSample():: vSingleApvData.size()=" << size << " fApvHeaderLevel=" << fApvHeaderLevel << " vSingleApvData["<<i<<"]="  << vSingleApvData[i]<< endl;
    //}
    while (i < size) {
        if (vSingleApvData[i] < fApvHeaderLevel) {
            i++;
            if (vSingleApvData[i] < fApvHeaderLevel) {
                i++;
                if (vSingleApvData[i] < fApvHeaderLevel) {
                    if (i + 138 < size) {
                        i += 10; //8 address + 1 error bit
                        idata = i;
                        iheader.push_back(i);
                        i += 128;
                        nbTimeSample++;
                        //cout<<"event["<<i<<"]:"<<" timesample="<<nbTimeSample<<endl;
                        continue;
                    }
                }
            }
        }
        i++;
    }
    //cout<<nbTimeSample<<endl;
    if (nbTimeSample != fNbOfTimeSamples) {
        //printf("    = GEMRawPedestal::CheckApvTimeSample() => WARNING !! Mismatched nb of time samples for FEC[%d], ADC[%d]: Expected=%d, Computed=%d\n", fFECID, fADCCh, fNbOfTimeSamples, nbTimeSample) ;
        return 0;
    }
    return 1;
}

void GEMRawPedestal::ComputeApvPedestal(int apvindex) {
    // compute offset and rms for each strip
    vector<float> timeBinPedestalOffset;
    vector<float> timeBinPedestalNoise;
    string apv_status = fAPVStatus.Data();
    apv_status = trim(apv_status);

    //cout << "ComputeApvPedestal(int apvindex)  " << endl;

    pair<multimap<int, float>::iterator, multimap<int, float>::iterator> stripTimeBinRawData;
    for (int stripNo = 0; stripNo < NCH; stripNo++) {
        stripTimeBinRawData = mApvTimeBinData.equal_range(stripNo);

        multimap<int, float>::iterator timebin_it;
        int timebin = 0;
        for (timebin_it = stripTimeBinRawData.first; timebin_it != stripTimeBinRawData.second; ++timebin_it) {
            float rawdata = timebin_it->second;
            float commonModeOffset = vCommonModeOffset[timebin];

            if ((apv_status != "normal") && (mapping->PRadStripMapping(fAPVID, stripNo) < 16)) {
                commonModeOffset = vCommonModeOffset_split[timebin];
            }
            timeBinPedestalNoise.push_back(rawdata - commonModeOffset);
            timeBinPedestalOffset.push_back(rawdata);
            timebin++;
        }

        float meanPedestalNoise = TMath::Mean(timeBinPedestalNoise.begin(), timeBinPedestalNoise.end());
        float meanPedestalOffset = TMath::Mean(timeBinPedestalOffset.begin(), timeBinPedestalOffset.end());

        // Kondo Way
        mStripOffset[apvindex].push_back(meanPedestalOffset);
        mStripRMS[apvindex].push_back(meanPedestalNoise);

        timeBinPedestalNoise.clear();
        timeBinPedestalOffset.clear();
    }
}

float GEMRawPedestal::GetStripNoise(int apvindex, int channelID) {
    assert(mStripRMS[apvindex].size() == NCH);
    return mStripRMS[apvindex][channelID];
}

float GEMRawPedestal::GetStripOffset(int apvindex, int channelID) {
    assert(mStripOffset[apvindex].size() == NCH);
    return mStripOffset[apvindex][channelID];
}

//debug member function
void GEMRawPedestal::PrintEventPedestal() {
    for (auto &i: mSrsSingleEvent) {
        int apvid = i.first;
        int adcch = apvid & 0xf;
        int fecid = (apvid >> 4) & 0xf;
        for (int i = 0; i < NCH; i++) {
            cout << " ======================================" << endl;
            cout << "FEC:  " << fecid << "  adc_ch:  " << adcch << endl;
            for (int i = 0; i < NCH; i++) {
                cout << i << ":  Noise: " << GetStripNoise(apvid, i);
                cout << ":  Offset:" << GetStripOffset(apvid, i) << endl;
            }
            cout << " ======================================" << endl;
        }
    }
}
