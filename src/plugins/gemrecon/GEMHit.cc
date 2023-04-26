#include "GEMHit.h"
//ClassImp(GEMHit);

//============================================================================================
GEMHit::GEMHit(Int_t hitID, Int_t apvID, Int_t chNo, Int_t zeroSupCut, TString isPeakOrSum, Int_t nbOfTB, Int_t stopTB,
               Int_t startTB) {
    //printf("GEMHit::GEMHit ==> enter \n") ;
    fHitID = hitID;
    fTimeBinADCs.clear();
    fHitADCs = 0;
    fPeakADCs = 0.0;
    fSumADCs = 0.0;
    fSignalPeakTimeBin = 0;

    NCH = 128;

    fIsHitPeakOrSumADCs = isPeakOrSum;
    fZeroSupCut = zeroSupCut;

    fStopTimeBin = stopTB;
    fStartTimeBin = startTB;
    fNbOfTimeBins = nbOfTB;
    //printf("GEMHit::GEMHit ==> enter fNbOfTB=%d, fStartTB=%d, fStopTB=%d \n", fNbOfTimeBins,fStartTimeBin,fStopTimeBin) ;

    for (Int_t i = 0; i < fNbOfTimeBins; i++) fTimeBinADCs[i] = 0;

    GemMapping *mapping = GemMapping::GetInstance();
    fAPVID = apvID;
    fAPVIndexOnPlane = mapping->GetAPVIndexOnPlane(fAPVID);
    fAPVOrientation = mapping->GetAPVOrientation(fAPVID);
    fPlane = mapping->GetPlaneFromAPVID(fAPVID);
    fDetector = mapping->GetDetectorFromPlane(fPlane);
    fDetectorID = mapping->GetDetectorID(fDetector);
    fDetectorType = mapping->GetDetectorTypeFromDetector(fDetector);
    fReadoutBoard = mapping->GetReadoutBoardFromDetector(fDetector);

    fPlaneID = mapping->GetPlaneID(fPlane);
    fPlaneSize = mapping->GetPlaneSize(fPlane);
    fNbOfAPVsOnPlane = mapping->GetNbOfAPVsOnPlane(fPlane);

    fAPVChNo = chNo;
    SetStripNo();
    TimingFindPeakTimeBin();
    ComputePosition();
    //  printf("GEMHit::GEMHit ==> exit \n") ;

}

//============================================================================================
GEMHit::~GEMHit() {
    fTimeBinADCs.clear();
}

//============================================================================================
Bool_t GEMHit::IsCleanHit() {
    Bool_t cleanHit = kTRUE;
    if (fHitADCs == 0) cleanHit = kFALSE;
    if ((fSignalPeakTimeBin < fStartTimeBin) || (fSignalPeakTimeBin > fStopTimeBin)) cleanHit = kFALSE;
    //  printf("GEMHit::IsCleanHit() ==> fSignalPeakTimeBin=%d, fStartTimeBin=%d, fStopTimeBin=%d \n",fSignalPeakTimeBin, fStartTimeBin,fStopTimeBin  ) ;
    return cleanHit;
}

//============================================================================================
void GEMHit::TimingFindPeakTimeBin() {
    Float_t currentMax = 0.0;
    std::map<Int_t, Float_t>::const_iterator max_itr;
    for (max_itr = fTimeBinADCs.begin(); max_itr != fTimeBinADCs.end(); ++max_itr) {
        if (max_itr->second > currentMax) {
            currentMax = max_itr->second;
            fSignalPeakTimeBin = max_itr->first;
        }
    }
}

//============================================================================================
void GEMHit::AddTimeBinADCs(Int_t timebin, Float_t charges, Float_t pedestal) {
    //  if (fAPVID % 2 == 1) charges = 1.2 * charges ;

    if ((fZeroSupCut > 0) && (charges > 0)) {
        fTimeBinADCs[timebin] = charges;
        fSumADCs += charges;
        if (charges > fHitADCs) fPeakADCs = charges;
        if (fIsHitPeakOrSumADCs == "sumADCs") fHitADCs = fSumADCs;
        else fHitADCs = fPeakADCs;
    } else {
        fTimeBinADCs[timebin] = charges;
        if (timebin == 0) fHitADCs = charges;
        else {
            // Averaging on the number of time sample
            fHitADCs *= timebin;
            fHitADCs += charges;
            fHitADCs /= (timebin + 1);
        }

    }
    //  if (fAPVID % 2 == 1) fHitADCs = 1.2 * fHitADCs ;
}

//============================================================================================
void GEMHit::ComputePosition() {
    Float_t pitch = fPlaneSize / (NCH * fNbOfAPVsOnPlane);
    if ((fDetectorType == "PRADGEM") && (fPlane.find("X") != std::string::npos)) {
        pitch = 0.40000;
        fStripPosition = -0.5 * (fPlaneSize - pitch) + (pitch * fStripNo);
    } else {
        fStripPosition = -0.5 * (fPlaneSize - pitch) + (pitch * fStripNo);
    }
    //  printf("GEMHit::ComputePosition ==> fPlane = %s  fPlaneSize = %f,  fStripNo = %d, fStripPosition = %f \n",fPlane.Data(), fPlaneSize, fStripNo, fStripPosition) ;
}

//============================================================================================
void GEMHit::SetStripNo() {
    fAbsoluteStripNo = StripMapping(fAPVChNo);
    if ((fDetectorType == "PRADGEM") && (fPlane.find("X") != std::string::npos)) {
        Int_t nbAPVsOnPlane = fNbOfAPVsOnPlane - 1;
        if (fAPVIndexOnPlane > fNbOfAPVsOnPlane) fStripNo = -100000000;
        if (fAPVOrientation == 0) fAbsoluteStripNo = 127 - fAbsoluteStripNo;
        fStripNo = (fAbsoluteStripNo - 16) + (NCH * (fAPVIndexOnPlane % (nbAPVsOnPlane)));

        if (fAPVIndexOnPlane == 11) {
            Int_t apvIndexOnPlane = fAPVIndexOnPlane - 1;
            if (apvIndexOnPlane > fNbOfAPVsOnPlane) fStripNo = -100000000;
            if (fAPVOrientation == 0) fAbsoluteStripNo = 127 - fAbsoluteStripNo;
            fStripNo = (fAbsoluteStripNo - 32/*16*/) + (NCH * (apvIndexOnPlane % nbAPVsOnPlane));
        }
    } else {
        if (fAPVIndexOnPlane > fNbOfAPVsOnPlane) fStripNo = -100000000;
        if (fAPVOrientation == 0) fAbsoluteStripNo = 127 - fAbsoluteStripNo;
        fStripNo = fAbsoluteStripNo + (NCH * (fAPVIndexOnPlane % fNbOfAPVsOnPlane));
    }
    //printf("GEMHit::ComputePosition ==> fPlane=%s, fAPVID=%d,  fAPVchNo=%d,  fStripNo=%d \n", fPlane.Data(), fAPVID, fAPVChNo, fStripNo) ;
}

//=====================================================
Int_t GEMHit::StripMapping(Int_t chNo) {
    chNo = APVchannelCorrection(chNo);
    if (fDetectorType == "PRADGEM") chNo = PRadStripMapping(chNo);
    return chNo;
}

//=====================================================
Int_t GEMHit::APVchannelCorrection(Int_t chNo) {
    chNo = (32 * (chNo % 4)) + (8 * (Int_t) (chNo / 4)) - (31 * (Int_t) (chNo / 16));
    return chNo;
}

//=====================================================
Int_t GEMHit::PRadStripMapping(Int_t chNo) {
    printf("   GEMHit::PRadStripMapping ==>\n");
    if ((fDetectorType == "PRADGEM") && ((fPlane.find("X") != std::string::npos)) && (fAPVIndexOnPlane == 11)) {
        if (chNo % 2 == 0) chNo = (chNo / 2) + 48;
        else if (chNo < 96) chNo = (95 - chNo) / 2;
        else chNo = 127 + (97 - chNo) / 2;
    } else { // NON (fDetectorType == "PRADGEM") && (fPlane.Contains("Y")) && (fAPVIndexOnPlane == 11)
        if (chNo % 2 == 0) chNo = (chNo / 2) + 32;
        else if (chNo < 64) chNo = (63 - chNo) / 2;
        else chNo = 127 + (65 - chNo) / 2;
    }
    //   printf("PRDPedestal::PRadStripsFMapping ==>  APVID=%d, chNo=%d, stripNo=%d, \n",fAPVID, chno, chNo) ;
    return chNo;
}
