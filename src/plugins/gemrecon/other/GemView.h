#ifndef GEMVIEW_H
#define GEMVIEW_H

#include <map>
#include <vector>
#include "hardcode.h"

#include "TCanvas.h"
#include "TStyle.h"
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TRandom.h"
#include "TList.h"
//#include "TQtWidget.h"
#include "TAxis.h"
#include "TPaveText.h"
#include "TRandom3.h"

//#include "PRadHistCanvas.h"
#include "TFile.h"

#include "GemHistContainer.h"
#include "GemConfiguration.h"
#include "GEMRawDecoder.h"
#include "GemInputHandler.h"
#include "GemMapping.h"
#include "GEMPedestal.h"
#include "GemFEC.h"
#include "assert.h"

class TObject;

class TCanvas;

class GemHistContainer;

class GemFEC;

class GemView {
public:
    GemView(int runnum);

    virtual ~GemView();

    int IsOnlineMode() const { return fConfig.GetOnlineMode(); }

    // TODO
    //  int ProcessSingleEvent(evioDOMNodeP bankPtr, vector <SFclust> &gemclust);

    int GemMonitorLoop();

    void SpinCurrentEvent(int i);

    void ChangeTimerInterval(int i);

    void ChangeAutoMode();

    int AutoGemMonitorLoop();

    void ParserInnerLoop();

    void WriteRootFile(TString outputRootFile);

    void SearchEvents();

    void AnalyzeMultiEvents();

    void AnalyzeMultipleFiles(TString inputDataFile, TString outputRootFile);

    void ProducePedestals();

    //private:

    Int_t fNbADCBins;

    void Clear();

    void ResetMultipleFilesHistos();

    void fillRawDataHisto();

    void DrawSingleEventDataHisto();

    void DrawMultipleEventDataHisto();

    void SetMonitor(int sec);

    void InitConfig(int runnum);

    void InitGUI();

    void InitToolBox();

    void InitRootGraphics();

    void InitHistForZeroSup();

    void InitHistForRawData();

    void InitHistForMultipleFiles();

    void DrawSingleEvent1DHistos(int planeId, TH1F *hist, Int_t nbOfStrips);

    void DrawSingleEvent1DHistos(int planeId, TH1F *hist, Int_t nbOfStrips, Float_t size);

    void DrawSingleEvent2DHistos(int planeId, TH2F *hist, TString plotStyle, Int_t nbOfStrips);

    void DrawSingleEvent2DHistos(int planeId, TH2F *hist, TString plotStyle, Float_t size, Int_t nbOfStrips);

    void DrawMultiEventsADCHistos(int id, TH1F *adcHist);

    void
    DrawMultiEvents1DHistos(int id, TH1F *hitHist, TH1F *clusterHist, TH1F *clusterInfoHist, TH2F *adcTimeBinPosHist);

    void DrawMultiEvents2DHistos(int detId, TH2F *pos2DHist);

    void DrawMultiEvents2DHistos(int detId, TH2F *chargeSharingHist, TH1F *chRatioHist);

    void DeleteHistForMultipleFiles();


    //  int fWidth, fHeight; //width and height of the QMainWindow
    int fWidth, fHeight, fCurrentEntry, fNbOfTimeSamples;
    TString fRunType, fRootFileName;

    GemFEC **fFEC;
    GemHistContainer **fRootWidget;
    GemConfiguration fConfig;
    GEMPedestal *fPedestal;
    GemMapping *fMapping;
    GemInputHandler *fHandler;

    TCanvas *c3;


    UInt_t *fBuffer;
private:

    TH1F **f1DSingleEventHist, **fADCHist, **fHitHist, **fClusterHist, **fClusterInfoHist, **fChargeRatioHist;
    TH2F **fTimeBinPosHist, **fADCTimeBinPosHist, **f2DPlotsHist, **f2DSingleEventHist, **fChargeSharingHist;

};

#endif // HelloClick_H
