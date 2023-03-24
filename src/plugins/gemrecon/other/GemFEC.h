#ifndef GEMFEC_H
#define GEMFEC_H
#include "TH1I.h"
//#include <QString>
#include <vector>
#include <list>
#include "GemView.h"
#include "GemMapping.h"

class GemView;
class GemFEC
{
 public:
  GemFEC();
  GemFEC(GemView* const p, int FECid, int nChannel, list<int> * APV_id_list);
  ~GemFEC();
  void ClearHist();
  void DrawHist();
  int GetFECid() const {return fFECid; }
  TH1I * GetHist(int i) { return fChannelHist[i]; }
  int IsChannelActive( int i) { return fChannelIsActive.at(i); }
  void SetFECid (int id) { fFECid = id; }
  void SetChannelIsActive(int i, int is){ fChannelIsActive.at(i) = is; }
  void SetNChannel(int i) { fNChannel = i; }
  void SetNBins(int i) { fNBins = i; }
  int GetAPVIDFromThisFEC(int i);
  int GetNActiveChannel() { return fAPVID.size(); }
  void RefreshGraphics();
  TString GetIPAddress() const { return fIPAddress; }
  void SetIPAddress(const char* name) { fIPAddress = name; } 
 private:
  int          fFECid;
  int          fNChannel;
  int          fNBins;
  int          fAPVChannel;
  TString      fIPAddress;
  TH1I**       fChannelHist;
  std::vector<int>  fChannelIsActive;
  std::vector<int>  fAPVID;
  GemMapping* fMapping;
};
#endif
