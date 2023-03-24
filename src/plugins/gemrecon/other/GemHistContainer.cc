#include "stdio.h"
#include "hardcode.h"
#include "assert.h"
#include "GemHistContainer.h"
#include <iostream>
using namespace std;

//====================================================================
GemHistContainer::GemHistContainer(GemView* const p, int id)
{

}

//====================================================================
GemHistContainer::~GemHistContainer() {}


//_______________________________________________________________
void GemHistContainer::RegisterHist(TH1I *hist) {
  if (fIsActive) fRootHist = hist;
}

//====================================================================
TH1I * GemHistContainer::getHist() {
  if (fIsActive) return fRootHist;
  else return 0;
}

//====================================================================
void GemHistContainer::drawHist() {
  /*
  gStyle->SetOptStat(0);
  if (fIsActive){
    TCanvas *c1 = this->GetCanvas();
    c1->cd();
    c1->SetFillColor(10);
    fRootHist->Draw();
    c1->Modified();
    c1->Update();
    this->Refresh();
  }
  */
}

//====================================================================

//====================================================================_
void GemHistContainer::SetBasicInfo(TString plane, int fecid, int apv_index, int apv_id) {
  fFECID = fecid;
  fPlane = plane;
  fIndex = apv_index;
  fAPVID = apv_id;
}

//====================================================================_
void GemHistContainer::RefreshGraphics() {
  /*
  TCanvas *c1 = this->GetCanvas();
  c1->Modified();
  c1->Update();
  this->Refresh();
  */
}






















