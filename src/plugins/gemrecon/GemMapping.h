// Mapping class is implemented as a singleton

#ifndef __PRDMAPPING__
#define __PRDMAPPING__
/*******************************************************************************
*  AMORE FOR PRD - PRD                                                         *
*  GemMapping                                                                  *
*  PRD Module Class                                                            *
*  Author: Kondo GNANVO 18/08/2010                                             *
*******************************************************************************/

#if !defined(__CINT__) || defined(__MAKECINT__)

#include <sstream>
#include <map>
#include <list>
#include <vector>
#include <set>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "TList.h"
#include "TObject.h"
#include "TString.h"
#include "TSystem.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TString.h"

#endif

using namespace std;

class GemMapping /*: public TObject*/ {

public:

    ~GemMapping() {
        Clear();
        ClearMapOfList(fDetectorListFromDetectorTypeMap);
        ClearMapOfList(fDetectorListFromReadoutBoardMap);
        ClearMapOfList(fPlaneListFromDetectorMap);
        ClearMapOfList(fFECIDListFromPlaneMap);
        ClearMapOfList(fAPVIDListFromFECIDMap);
        ClearMapOfList(fAPVIDListFromPlaneMap);
        ClearMapOfList(fAPVIDListFromDetectorMap);
        ClearMapOfList(fActiveADCchannelsMap);
        ClearMapOfList(fADCchannelsFromFECMap);

        ClearMapOfList(fAPVToPadChannelMap);
        ClearMapOfList(fPadDetectorMap);
        ClearMapOfList(f1DStripsPlaneMap);
        ClearMapOfList(fCartesianPlaneMap);
        ClearMapOfList(fCMSGEMDetectorMap);
        ClearMapOfList(fUVangleReadoutMap);
    }

    template<typename M>
    void ClearMapOfList(M &amap);

    static GemMapping *GetInstance() {
        if (instance == 0) instance = new GemMapping();
        return instance;
    }

    void PrintMapping();

    void SaveMapping(const char *mappingCfgFilename);

    void LoadMapping(const char *mappingCfgFilename);

    set<int> GetBankIDSet();

    TString GetFECIPFromFECID(Int_t id);

    Int_t GetFECIDFromFECIP(TString);

    void LoadAPVtoPadMapping(const char *mappingCfgFilename);

    void SetAPVtoPadMapping(Int_t fecId, Int_t adcCh, Int_t padId, Int_t apvCh);;

    void
    SetAPVMap(TString detPlane, Int_t fecId, Int_t adcCh, Int_t apvNo, Int_t apvOrient, Int_t apvIndex, Int_t apvHdr,
              TString defaultAPV);

    void Set1DStripsReadoutMap(TString readoutBoard, TString detType, TString detName, Int_t detID, TString plane,
                               Float_t size, Int_t connectors, Int_t orient);

    void SetCartesianStripsReadoutMap(TString readoutBoard, TString detectorType, TString detector, Int_t detID,
                                      TString planeX, Float_t sizeX, Int_t connectorsX, Int_t orientX, TString planeY,
                                      Float_t sizeY, Int_t connectorsY, Int_t orientY);

    void SetUVStripsReadoutMap(TString readoutBoard, TString detType, TString detector, Int_t detID, Float_t length,
                               Float_t outerR, Float_t innerR, TString planeTop, Int_t conectTop, Int_t orientTop,
                               TString planeBot, Int_t connectBot, Int_t orientBot);

    void SetPadsReadoutMap(TString readoutBoard, TString detType, TString detName, Int_t detID, TString padPlane,
                           Float_t sizeX, Float_t sizeY, Float_t nbOfPadX, Float_t nbOfPadY, Float_t nbOfConnectors);

    void
    SetCMSGEMReadoutMap(TString readoutBoard, TString detectorType, TString detector, Int_t detID, TString EtaSector,
                        Float_t etaSectorPos, Float_t etaSectorSize, Float_t nbOfSectorConnectors,
                        Int_t apvOrientOnEtaSector);

    Int_t APVchannelCorrection(Int_t chNo);

    Int_t PRadStripMapping(Int_t apvID, Int_t chNo);

    Int_t CMSStripMapping(Int_t chNo);;

    Int_t StandardMapping(Int_t chNo);

    Int_t EICStripMapping(Int_t chNo);

    Int_t HMSStripMapping(Int_t chNo);

    Int_t GetStripMapping(Int_t apvID, Int_t chNo);

    void ComputeActiveADCchannelsMap();

    vector<Int_t> GetActiveADCchannels(Int_t fecID) { return fActiveADCchannelsMap[fecID]; }

    void Clear();

    map<Int_t, Int_t> GetAPVNoFromIDMap() { return fAPVNoFromIDMap; }

    map<Int_t, Int_t> GetAPVIDFromAPVNoMap() { return fAPVIDFromAPVNoMap; }

    map<Int_t, Int_t> GetAPVGainFromIDMap() { return fAPVGainFromIDMap; }

    map<Int_t, Int_t> GetAPVOrientationFromIDMap() { return fAPVOrientationFromIDMap; }

    map<Int_t, UInt_t> GetAPVHeaderLevelFromIDMap() { return fAPVHeaderLevelFromIDMap; }

    map<Int_t, Int_t> GetAPVIndexOnPlaneFromIDMap() { return fAPVIndexOnPlaneFromIDMap; }

    map<Int_t, TString> GetAPVFromIDMap() { return fAPVFromIDMap; }

    map<Int_t, TString> GetDetectorFromIDMap() { return fDetectorFromIDMap; }

    map<Int_t, TString> GetDetectorFromAPVIDMap() { return fDetectorFromAPVIDMap; }

    map<TString, Int_t> GetPlaneIDFromPlaneMap() { return fPlaneIDFromPlaneMap; }

    map<TString, vector<Float_t> > GetPadDetectorMap() { return fPadDetectorMap; }

    map<TString, vector<Float_t> > GetCartesianPlaneMap() { return fCartesianPlaneMap; }

    map<TString, list<Int_t> > GetAPVIDListFromDetectorMap() { return fAPVIDListFromDetectorMap; }

    map<TString, list<Int_t> > GetAPVIDListFromPlaneMap() { return fAPVIDListFromPlaneMap; }

    list<Int_t> GetAPVIDListFromDetector(TString detName) {
        fAPVIDListFromDetectorMap[detName].unique();
        return fAPVIDListFromDetectorMap[detName];
    }

    list<Int_t> GetAPVIDListFromPlane(TString planeName) {
        fAPVIDListFromPlaneMap[planeName].unique();
        return fAPVIDListFromPlaneMap[planeName];
    }

    list<Int_t> GetFECIDListFromPlane(TString planeName) {
        fFECIDListFromPlaneMap[planeName].unique();
        return fFECIDListFromPlaneMap[planeName];
    }

    list<Int_t> GetAPVIDListFromFECID(Int_t fecId) {
        fAPVIDListFromFECIDMap[fecId].unique();
        return fAPVIDListFromFECIDMap[fecId];
    }

    list<TString> GetPlaneListFromDetector(TString detName) {
        fPlaneListFromDetectorMap[detName].unique();
        return fPlaneListFromDetectorMap[detName];
    }

    Int_t GetFECIDFromAPVID(Int_t apvID) { return (apvID >> 4) & 0xF; }

    Int_t GetADCChannelFromAPVID(Int_t apvID) { return apvID & 0xF; }

    Int_t GetAPVNoFromID(Int_t apvID) { return fAPVNoFromIDMap[apvID]; }

    Int_t GetAPVIDFromAPVNo(Int_t apvID) { return fAPVIDFromAPVNoMap[apvID]; }

    Int_t GetAPVIndexOnPlane(Int_t apvID) { return fAPVIndexOnPlaneFromIDMap[apvID]; }

    Int_t GetAPVOrientation(Int_t apvID) { return fAPVOrientationFromIDMap[apvID]; }

    Int_t GetAPVIDFromName(TString apvName) { return fAPVIDFromNameMap[apvName]; }

    UInt_t GetAPVHeaderLevelFromID(Int_t apvID) { return fAPVHeaderLevelFromIDMap[apvID]; }

    Int_t GetDetectorID(TString detName) { return fDetectorIDFromDetectorMap[detName]; }

    TString GetDetectorFromID(Int_t detID) { return fDetectorFromIDMap[detID]; }

    TString GetDetectorTypeFromDetector(TString detectorName) { return fDetectorTypeFromDetectorMap[detectorName]; }

    TString GetReadoutBoardFromDetector(TString detectorName) { return fReadoutBoardFromDetectorMap[detectorName]; }

    TString GetDetectorFromPlane(TString planeName) { return fDetectorFromPlaneMap[planeName]; }

    vector<Float_t> GetCartesianReadoutMap(TString plane) { return fCartesianPlaneMap[plane]; }

    vector<Float_t> GetUVangleReadoutMap(TString plane) { return fUVangleReadoutMap[plane]; }

    vector<Float_t> Get1DStripsReadoutMap(TString plane) { return f1DStripsPlaneMap[plane]; }

    vector<Float_t> GetCMSGEMReadoutMap(TString etaSector) { return fCMSGEMDetectorMap[etaSector]; }

    Int_t GetPlaneID(TString planeName);

    Int_t GetEtaSector(TString planeName);

    Float_t GetPlaneSize(TString planeName);

    Int_t GetNbOfAPVsOnPlane(TString planeName);

    Int_t GetPlaneOrientation(TString planeName);

    TString GetPlaneFromAPVID(Int_t apvID) { return fPlaneFromAPVIDMap[apvID]; }

    TString GetAPVstatus(Int_t apvID) { return fAPVstatusMap[apvID]; }

    TString GetAPVFromID(Int_t apvID) { return fAPVFromIDMap[apvID]; }

    TString GetAPV(TString detPlane, Int_t fecId, Int_t adcChannel, Int_t apvNo, Int_t apvIndex);

    vector<Int_t> GetPadChannelsMapping(Int_t apvID) { return fAPVToPadChannelMap[apvID]; }

    map<Int_t, vector<Int_t> > GetPadChannelsMapping() { return fAPVToPadChannelMap; }

    Int_t GetNbOfAPVs() { return fAPVNoFromIDMap.size(); }

    Int_t GetNbOfFECs() { return fAPVIDListFromFECIDMap.size(); }

    Int_t GetNbOfPlane() { return fDetectorFromPlaneMap.size(); }

    Int_t GetNbOfDetectors() { return fReadoutBoardFromDetectorMap.size(); }

    Int_t GetNChannelEachFEC(Int_t i) { return fADCchannelsFromFECMap[i].size(); }

    Float_t GetTrapezoidDetLength(TString detector) { return (fUVangleReadoutMap[detector])[0]; }

    Float_t GetTrapezoidDetInnerRadius(TString detector) { return (fUVangleReadoutMap[detector])[1]; }

    Float_t GetTrapezoidDetOuterRadius(TString detector) { return (fUVangleReadoutMap[detector])[2]; }

    Bool_t IsAPVIDMapped(Int_t apvID) {
        map<Int_t, TString>::iterator itr;
        itr = fAPVFromIDMap.find(apvID);
        if (itr != fAPVFromIDMap.end())
            return kTRUE;
        else
            return kFALSE;
    }


private:

    GemMapping() { fNbOfAPVs = 0; }

    static GemMapping *instance;
    Int_t fNbOfAPVs;

    map<Int_t, UInt_t> fAPVHeaderLevelFromIDMap;
    map<Int_t, Int_t> fAPVNoFromIDMap, fAPVIDFromAPVNoMap, fAPVIndexOnPlaneFromIDMap, fAPVOrientationFromIDMap;
    map<TString, Int_t> fNbOfAPVsFromDetectorMap;

    map<Int_t, Int_t> fAPVGainFromIDMap;
    map<TString, Int_t> fAPVIDFromNameMap;
    map<Int_t, TString> fAPVFromIDMap;
    map<Int_t, TString> fAPVstatusMap;

    map<TString, Int_t> fPlaneIDFromPlaneMap;
    map<Int_t, TString> fPlaneFromAPVIDMap;

    map<Int_t, TString> fReadoutBoardFromIDMap;

    map<TString, TString> fDetectorTypeFromDetectorMap;
    map<TString, TString> fReadoutBoardFromDetectorMap;
    map<TString, Int_t> fDetectorIDFromDetectorMap;
    map<Int_t, TString> fDetectorFromIDMap;
    map<Int_t, TString> fDetectorFromAPVIDMap;
    map<TString, TString> fDetectorFromPlaneMap;

    map<Int_t, list<Int_t> > fAPVIDListFromFECIDMap;

    map<TString, list<Int_t> > fFECIDListFromPlaneMap;
    map<TString, list<Int_t> > fAPVIDListFromDetectorMap;
    map<TString, list<Int_t> > fAPVIDListFromPlaneMap;

    map<TString, list<TString> > fDetectorListFromDetectorTypeMap;
    map<TString, list<TString> > fDetectorListFromReadoutBoardMap;
    map<TString, list<TString> > fPlaneListFromDetectorMap;

    map<Int_t, vector<Int_t> > fAPVToPadChannelMap;
    map<Int_t, vector<Int_t> > fActiveADCchannelsMap;

    map<TString, vector<Float_t> > fADCchannelsFromFECMap;
    map<TString, vector<Float_t> > fPadDetectorMap;
    map<TString, vector<Float_t> > fUVangleReadoutMap;
    map<TString, vector<Float_t> > f1DStripsPlaneMap;
    map<TString, vector<Float_t> > fCartesianPlaneMap;
    map<TString, vector<Float_t> > fCMSGEMDetectorMap;

    map<Int_t, TString> fFECIPFromFECID;
    map<TString, Int_t> fFECIDFromFECIP;
};

#endif



