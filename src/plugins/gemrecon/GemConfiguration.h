#ifndef __PRDCONFIGURATION__
#define __PRDCONFIGURATION__
/*******************************************************************************
*  AMORE FOR PRD - PRD                                                         *
*  PRDConfiguration                                                            *
*  PRD Module Class                                                            *
*  Author: Kondo GNANVO 18/08/2010                                             *
*******************************************************************************/

#if !defined(__CINT__) || defined(__MAKECINT__)

#include <fstream>
#include <iostream>
#include "TObject.h"
#include "TH1F.h"
#include "TSystem.h"
#endif

// Class containing all configuration parameters for configuration from file(s)


class GemConfiguration/* : public TObject */{

 public:
  GemConfiguration();
  ~GemConfiguration();


  bool FileExists(const std::string& name) const;

  int GetCycleWait() const { return fCycleWait; }
  void SetCycleWait(int time) { fCycleWait = time; }

  char * GetInputFileName() const { return gSystem->ExpandPathName(fInputFileName.Data()); }
  void SetInputFileName(const char * name) { fInputFileName = name; }

  char * GetOutputFileName() const { return gSystem->ExpandPathName(fOutputFileName.Data()); }
  void SetOutputFileName(const char * name) { fOutputFileName = name; }

  char * GetPedFileName() const { return gSystem->ExpandPathName(fPedFileName.Data()); }
  void SetPedFileName(const char * name) { fPedFileName = name; }

  char * GetInputMapName() const { return gSystem->ExpandPathName(fInputMapName.Data()); }
  void SetInputMapName(const char * name) { fInputMapName = name; }

  char * GetETIPAddress() const { return gSystem->ExpandPathName(fETIPAddress.Data()); }
  void SetETIPAddress(const char * name) { fETIPAddress = name; }

  char * GetETStationName() const { return gSystem->ExpandPathName(fETStationName.Data()); }
  void SetETStationName(const char * name) { fETStationName = name; }

  char * GetCODAFileDir() const { return gSystem->ExpandPathName(fCODAFileDir.Data()); }
  void SetCODAFileDir(const char * name) { fCODAFileDir = name; }

  char * GetRunType() const { return gSystem->ExpandPathName(fRunType.Data()); }
  void SetRunType(const char * name) { fRunType = name; }

  char * GetHitPeakOrSumADCs() const { return gSystem->ExpandPathName(fIsHitPeakOrSumADCs.Data()); }
  void SetHitPeakOrSumADCs(const char * name) { fIsHitPeakOrSumADCs = name; }

  char * GetCentralOrAllStripsADCs() const { return gSystem->ExpandPathName(fIsCentralOrAllStripsADCs.Data()); }
  void SetCentralOrAllStripsADCs(const char * name) { fIsCentralOrAllStripsADCs = name; }

  int GetOnlineMode() const { return fIsOnlineMode; }
  void SetOnlineMode(int set) { fIsOnlineMode = set; }
 
  int GetNCol() const { return fNCol; }
  void SetNCol(int n) { fNCol = n; }

  int GetTCPPort() const { return fTCPPort; }
  void SetTCPPort(int n) { fTCPPort = n; }

  int GetETMode() const { return fETMode; }
  void SetETMode(int n) { fETMode = n; }

  int GetZeroSupCut() const { return fZeroSupCut; }
  void SetZeroSupCut(int n) { fZeroSupCut = n; }

  int GetComModeCut() const { return fComModeCut; }
  void SetComModeCut(int n) { fComModeCut = n; }

  int GetMinADCvalue() const { return fMinADCvalue; }
  void SetMinADCvalue(int set) { fMinADCvalue = set; }

  int GetMaxADCvalue() const { return fMaxADCvalue; }
  void SetMaxADCvalue(int set) { fMaxADCvalue = set; }

  int GetMinClusterSize() const { return fMinClusterSize; }
  void SetMinClusterSize(int set) { fMinClusterSize = set; }

  int GetMaxClusterSize() const { return fMaxClusterSize; }
  void SetMaxClusterSize(int set) { fMaxClusterSize = set; }

  int GetMaxClusterMult() const { return fMaxClusterMult; }
  void SetMaxClusterMult(int set) { fMaxClusterMult = set; }

  int GetNbOfTimeSamples() const { return fNbOfTimeSamples; }
  void SetNbOfTimeSamples(int set) { fNbOfTimeSamples = set; }

  int GetStartTimeSamples() const { return fStartTimeSamples; }
  void SetStartTimeSamples(int set) { fStartTimeSamples = set; }

  int GetStopTimeSamples() const { return fStopTimeSamples; }
  void SetStopTimeSamples(int set) { fStopTimeSamples = set; }

  int GetFirstEvent() const { return fFirstEvent; }
  void SetFirstEvent(int set) { fFirstEvent = set; }

  int GetLastEvent() const { return fLastEvent; }
  void SetLastEvent(int set) { fLastEvent = set; }

  void Load(const std::string& filename);
  void Save(const std::string& filename) const;
  void Dump() const;

  GemConfiguration & operator=(const GemConfiguration &rhs);

 private:
  void BookArrays();
  void SetDefaults();

  TString fETIPAddress, fETStationName, fInputFileName, fOutputFileName, fPedFileName, fInputMapName;
  TString  fCODAFileDir, fRunType, fIsHitPeakOrSumADCs, fIsCentralOrAllStripsADCs;
  int fMaxADCvalue, fMinADCvalue, fMinClusterSize, fMaxClusterSize, fMaxClusterMult, fCycleWait, fIsOnlineMode;
  int fNCol, fTCPPort, fETMode, fZeroSupCut, fComModeCut, fNbOfTimeSamples, fStartTimeSamples, fStopTimeSamples, fLastEvent, fFirstEvent;
  //ClassDef(GemConfiguration, 1)
};

#endif
