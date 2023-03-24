#ifndef GEMHISTCONTAINER_H
#define GEMHISTCONTAINER_H

#include "TCanvas.h"
#include "TStyle.h"
#include "TH1I.h"
#include "TH2F.h"
#include "TF1.h"
#include "TRandom.h"
#include "TList.h"
#include "TAxis.h"
#include "TPaveText.h"
#include "TRandom3.h"
#include "GemView.h"
class GemView;
class GemHistContainer
{
 public:
  GemHistContainer(GemView* const p, int id);
  ~GemHistContainer();
  
  void drawHist();
  TH1I * getHist();
  int  GetFECID() const { return fFECID; }
  TString GetPlane() const { return fPlane; }
  int  GetIndex() const { return fIndex; }
  int  GetAPVID() const { return fAPVID; }
  void Activate() { fIsActive = true; }
  bool IsActive() const { return fIsActive; }
  void RefreshGraphics();
  
  void SetBasicInfo(TString plane, int fecid, int apv_index, int apv_id);
  void RegisterHist(TH1I *hist);

 protected:
 private:
  GemView* fParent;
  //GemAPV *fAPV;
  TH1I *fRootHist;
  int fModuleID;
  int fMaxBin;
  int fFECID;
  TString fPlane;
  int fIndex;
  int fAPVID;
  bool fIsActive;
};

#endif
