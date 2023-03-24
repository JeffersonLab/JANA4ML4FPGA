#include "GemFEC.h"
#include "TCanvas.h"
#include <vector>
#include "TAxis.h"
//====================================================================
GemFEC::GemFEC() {}
//====================================================================
GemFEC::GemFEC(GemView* const p, int FECid, int nChannel, list<int> * APV_id_list) {

}
//====================================================================
GemFEC::~GemFEC()
{
  for (int i=0; i<fNChannel; i++){
    delete fChannelHist[i];	
  } 
}

//====================================================================
void GemFEC::ClearHist() {
  for (int i=0; i<fNChannel; i++) fChannelHist[i]->Reset("M");
  /*
  TCanvas *c1 = this->GetCanvas();
  c1->GetListOfPrimitives()->Clear();
  c1->Modified();
  c1->Update();
  */
}

//====================================================================
void GemFEC::DrawHist() {

  /*
  TCanvas *c1 = this->GetCanvas();
  //  c1->SetFillColor(10);
  c1->Divide(4,4);
  for (int i=0; i<fNChannel; i++) {
    c1->cd(i+1);
    TPad* pad = (TPad*)c1->GetPad(i+1);
    pad->SetLeftMargin(0.13);
    fChannelHist[i]->Draw();
  }
  c1->Modified();
  c1->Update();
  this->Refresh();
  */
}

//====================================================================
int GemFEC::GetAPVIDFromThisFEC(int i) {
  if (i > (int)fAPVID.size() - 1){
    cout<<"FEC "<<fFECid<<" does not has that many active channels"<<endl;
    return -1;
  }else{
    return fAPVID.at(i);
  }
}

//====================================================================
void GemFEC::RefreshGraphics() {
  /*
  TCanvas *c1 = this->GetCanvas();
  c1->Modified();
  c1->Update();
  this->Refresh();
  */
}

















