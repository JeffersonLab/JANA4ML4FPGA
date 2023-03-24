#include "GEMCluster.h"

//====================================================================================================================
GEMCluster::GEMCluster(Int_t eventNb, Int_t clusterId, Int_t minClusterSize, Int_t maxClusterSize, TString isCentralOrAllStripsADCs, Float_t minADCs) {
  fNbOfHits = 0;  

  fClusterAllStripsADCs = 0; 
  fClusterCentralStripADCs = 0;
  fClusterTimeBinADC = 0 ;
  fClusterPeakTimeBin = -1 ;
  fposition = 0;
  fclusterCentralStripNb = 0;
  fclusterCentralStripPosition = -999;
  fEventNb = eventNb ;
  fClusterId = clusterId ;
  fMinADCvalue    = minADCs;
  fMinClusterSize = minClusterSize;
  fMaxClusterSize = maxClusterSize;
  fIsCentralOrAllStripsADCs = isCentralOrAllStripsADCs ;
  fPlane     = "GEM1X" ;
  fArrayOfHits = new TObjArray(maxClusterSize);
  fIsGoodCluster = kTRUE ;
  fStartTimeSamples = 0 ;
  fStopTimeSamples = 9;

  //  printf("=======GEMCluster\n") ;
}

//====================================================================================================================
GEMCluster::~GEMCluster() {
  fArrayOfHits->Clear();
  delete fArrayOfHits;
}

//============================================================================================
void GEMCluster::ClusterTiming() {
  TObjArray &temp = *fArrayOfHits;
  Int_t nbofhits =  GetNbOfHits() ;
  Float_t adc = 0;
  for (int i = 0; i < nbofhits; i++) {	
    if (! ((GEMHit*)temp[i])->IsCleanHit() )  continue ;
    map <Int_t, Float_t>  timeBinADCs = ((GEMHit*)temp[i])->GetTimeBinADCs() ;
    if ( ((GEMHit*)temp[i])->GetHitADCs() > adc) { 
      adc = ((GEMHit*)temp[i])->GetHitADCs() ;
      fClusterPeakTimeBin = ((GEMHit*)temp[i])->GetSignalPeakTimeBin() ;
      fclusterCentralStripNb = ((GEMHit*)temp[i])->GetStripNb();
    }
    //    printf("GEMCluster::ClusterTiming() ==> fClusterPeakTimeBin=%d \n", fClusterPeakTimeBin) ;
  }
}

//==================================================================================================
void GEMCluster::ClusterPosition() {  // Calculate the fposition and the total fClusterAllStripsADCs
  float hitposition, q;
  TObjArray &temp = *fArrayOfHits;
  Int_t nbofhits = GetNbOfHits() ;
  for (int i = 0; i < nbofhits; i++) {	
   q  = ((GEMHit*)temp[i])->GetHitADCs() ;

   hitposition = ((GEMHit*)temp[i])->GetStripPosition() ;

   fClusterAllStripsADCs += q ;
   fposition += q * hitposition ;
   if (q > fClusterCentralStripADCs) {
     fClusterPeakTimeBin = ((GEMHit*)temp[i])->GetSignalPeakTimeBin() ;
     fClusterCentralStripADCs = q ;
   }
  }
  fposition /= fClusterAllStripsADCs;
  //  printf("   GEMCluster::ClusterPosition => clusterPosition = %f\n", fposition) ;
}

//====================================================================================================================
void GEMCluster::AddHit(GEMHit *hit) {
  fArrayOfHits->AddLast(hit); 
}

//====================================================================================================================
void GEMCluster::ClearArrayOfHits() {
  fArrayOfHits->Clear();
}

//====================================================================================================================
Bool_t GEMCluster::IsGoodCluster() {
  fIsGoodCluster = kTRUE ;

  if ( (fNbOfHits > fMaxClusterSize) || (fNbOfHits < fMinClusterSize) ) {
    fIsGoodCluster = kFALSE ;
    ClearArrayOfHits() ;
    fNbOfHits = fArrayOfHits->GetEntries() ;
  } 

  if (fClusterCentralStripADCs < fMinADCvalue) {
    fIsGoodCluster = kFALSE ;
    ClearArrayOfHits() ;
    fNbOfHits = fArrayOfHits->GetEntries() ;
  }

  if ( ( fClusterPeakTimeBin < fStartTimeSamples) ||  ( fClusterPeakTimeBin  > fStopTimeSamples) ) {
    fIsGoodCluster = kFALSE ;
    //   printf("   GEMCluster::IsGoodCluster => plane = %s, peak = %d, start = %d stop = %d\n",fPlane.Data(),fClusterPeakTimeBin, fStartTimeSamples, fStopTimeSamples) ;
    ClearArrayOfHits() ;
    fNbOfHits = fArrayOfHits->GetEntries() ;
  }


  /**
  if ( (fPlane == "TRDGEM1X") && (fposition > 0) ) {
    //printf("   GEMCluster::IsGoodCluster => plane = %s, clustPos = %f, clustADC = %f \n",fPlane.Data(),fposition,fClusterCentralStripADCs) ;
    fIsGoodCluster = kFALSE ;
    ClearArrayOfHits() ;
    fNbOfHits = fArrayOfHits->GetEntries() ;
  }
  */

  return fIsGoodCluster ;
}

//====================================================================================================================
int GEMCluster::Compare(const TObject *obj, TString isCentralOrAllStripsADCs) const {
  int compare = (fClusterCentralStripADCs  < ((GEMCluster*)obj)->GetClusterADCs(isCentralOrAllStripsADCs)) ? 1 : -1;
  return compare ;
}


//====================================================================================================================
void GEMCluster::ComputeCluster() {
  fNbOfHits = fArrayOfHits->GetEntries() ;
  ClusterPosition() ;
  ClusterTiming() ;
}
