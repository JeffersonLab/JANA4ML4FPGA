#ifndef GEMHIT_H
#define GEMHIT_H
/*******************************************************************************
*  AMORE FOR PRD - PRD                                                         *
*  GEMHit                                                                      *
*  PRD Module Class                                                            *
*  Author: Kondo GNANVO 12/27/2015                                             *
*          Xinzhan Bai  03/20/2016                                             *
*******************************************************************************/

#if !defined(__CINT__) || defined(__MAKECINT__)

#include <map>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include "TList.h"
#include "TObject.h"
#include "TMath.h"
#include "TGraph.h"
#include "TVector.h"
#include <stdlib.h>
#include "TH1.h"
#include "TF1.h"
#include "TMath.h"
#include "GemMapping.h"

#endif


class GEMHit : public TObject {

public:

    GEMHit();

    GEMHit(Int_t hitID, Int_t apvID, Int_t chNo, Int_t zeroSupCut, TString isPeakOrSum, Int_t nbOfTB, Int_t stopTB,
           Int_t startTB);

    ~GEMHit();

    Bool_t IsSortable() const { return kTRUE; }

    //=== Sort hit according to the strip number
    Int_t Compare(const TObject *obj) const { return (fStripNo > ((GEMHit *) obj)->GetStripNo()) ? 1 : -1; }

    void ComputePosition();

    void AddTimeBinADCs(Int_t timebin, Float_t charges, Float_t pedestal);

    void ClearTimeBinADCs() { fTimeBinADCs.clear(); }

    void TimingFindPeakTimeBin();

    Bool_t IsCleanHit();

    Int_t GetAPVID() { return fAPVID; }

    Int_t GetAPVOrientation() { return fAPVOrientation; }

    Int_t GetAPVIndexOnPlane() { return fAPVIndexOnPlane; }

    Int_t GetNbAPVsFromPlane() { return fNbOfAPVsOnPlane; }

    Float_t GetHitADCs() { return fHitADCs; }

    std::map<Int_t, Float_t> GetTimeBinADCs() { return fTimeBinADCs; }

    Int_t StripMapping(Int_t chNo);

    Int_t APVchannelCorrection(Int_t chNo);

    Int_t PRadStripMapping(Int_t chNo);

    Int_t GetSignalPeakTimeBin() {
        TimingFindPeakTimeBin();
        return fSignalPeakTimeBin;
    }

    TString GetPlane() { return fPlane; }

    Float_t GetPlaneSize() { return fPlaneSize; }

    TString GetDetector() { return fDetector; }

    TString GetDetectorType() { return fDetectorType; }

    TString GetReadoutBoard() { return fReadoutBoard; }

    TString GetHitPeakOrSumADCs() { return fIsHitPeakOrSumADCs; }

    void SetStripNo();

    Int_t GetStripNo() { return fStripNo; }

    Int_t GetStripNb() { return fStripNo; }

    Int_t GetAbsoluteStripNo() { return fAbsoluteStripNo; }

    Int_t GetAbsoluteStripNb() { return fAbsoluteStripNo; }

    Float_t GetStripPosition() {
        ComputePosition();
        return fStripPosition;
    }

private:

    std::map<Int_t, Float_t> fTimeBinADCs;

    uint8_t NCH, fNbOfTimeBins, fStartTimeBin, fStopTimeBin, fSignalPeakTimeBin, fZeroSupCut;
    uint8_t fAPVIndexOnPlane, fNbOfAPVsOnPlane, fAPVOrientation, fDetectorID, fPlaneID;
    Int_t fAPVID, fHitID, fAPVChNo, fStripNo, fAbsoluteStripNo;

    Float_t fHitADCs, fPeakADCs, fSumADCs, fStripPosition, fPlaneSize, fHitPedestalNoise;
    TString fPlane, fReadoutBoard, fDetectorType, fDetector, fIsHitPeakOrSumADCs;

    //ClassDef(GEMHit,2)
};

#endif
