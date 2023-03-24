#ifndef GEMCLUSTER_H
#define GEMCLUSTER_H
/*******************************************************************************
*  AMORE FOR GEM - GEM                                                         *
*  GEMCluster                                                                  *
*  GEM Module Class                                                            *
*  Author: Kondo GNANVO 18/08/2010                                             *
*******************************************************************************/

#if !defined(__CINT__) || defined(__MAKECINT__)

#include <iostream>
#include "GEMHit.h"

#include <TMath.h>
#include <TObject.h>
#include <TObjArray.h>
#include "TFile.h"
#endif 

using namespace std;

class GEMCluster: public TObject{

 public:
  GEMCluster(Int_t eventNb, Int_t clusterId, Int_t minClusterSize, Int_t maxClusterSize, TString isCentralOrAllStripsADCs, Float_t minADCs) ;
 ~GEMCluster();

  Bool_t IsSortable() const {return kTRUE;}
  TObjArray* GetArrayOfHits() {return fArrayOfHits;}

  GEMHit * GetHit(Int_t i) { 
     TObjArray &temp = * fArrayOfHits ;
     return (GEMHit *) temp[i];
  }

  void SetMinClusterSize(Int_t min) {fMinClusterSize = min ; }
  void SetMaxClusterSize(Int_t max) {fMaxClusterSize = max ; }

  void AddHit(GEMHit * h) ;

  int Compare(const TObject *obj, TString isCentralOrAllStripsADCs) const ;

  Int_t & GetNbOfHits() { return fNbOfHits ;  }

  TString GetPlane()     {return fPlane;}
  void SetPlane(TString planename) {fPlane = planename;}

  Float_t GetMinADCvalue() {return fMinADCvalue;}
  void SetMinADCvalue(Float_t minADCs) {fMinADCvalue = minADCs;}

  void SetStopTimeSamples(Int_t stopTimeSamples) {fStopTimeSamples = stopTimeSamples;}
  void SetStartTimeSamples(Int_t startTimeSamples) {fStartTimeSamples = startTimeSamples;}

  Float_t GetClusterPosition()             {return fposition;}
  Float_t GetClusterCentralStripPosition() {return fclusterCentralStripPosition;}

  Int_t GetClusterPeakTimeBin()      {return fClusterPeakTimeBin ;}
  Int_t GetClusterCentralStripNb()    {return fclusterCentralStripNb;}

  void ClearArrayOfHits();
  Bool_t IsGoodCluster() ;

  void ClusterTiming() ;
  void ClusterPosition() ;
  void ComputeCluster() ;

  void SetClusterADCs(Float_t adc) {
    if (fIsCentralOrAllStripsADCs == "centralStripADCs") fClusterCentralStripADCs = adc ;
    else                                                 fClusterAllStripsADCs = adc; 
  }

  Float_t GetClusterADCs() { 
     if (fIsCentralOrAllStripsADCs == "centralStripADCs") return fClusterCentralStripADCs;
     else                                                 return fClusterAllStripsADCs;
   }

  Float_t GetClusterADCs(TString isCentralOrAllStripsADCs) { 
    if (isCentralOrAllStripsADCs == "allStripsADCs") return fClusterAllStripsADCs;
     else                                            return fClusterCentralStripADCs ;
   }


 private:

  TObjArray * fArrayOfHits;  // APV hits table
  Float_t fMinADCvalue, fClusterCentralStripADCs, fClusterTimeBinADC, fClusterAllStripsADCs;
  Float_t fposition, fclusterCentralStripPosition  ;
  Int_t fEventNb, fNbOfHits, fClusterId,  fclusterCentralStripNb ;
  uint8_t fClusterPeakTimeBin, fMinClusterSize, fMaxClusterSize,  fStartTimeSamples, fStopTimeSamples;
  TString fIsCentralOrAllStripsADCs, fPlane;
  Bool_t fIsGoodCluster ;

}; 
#endif
