/* includeles */
#include <iostream>
#include <sstream>   
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <evioUtil.hxx>
#include <unistd.h>
#include <vector>
#include "evioFileChannel.hxx"
 
using namespace std;
using namespace evio;

// root include files
#include "TApplication.h"  // needed to display canvas
#include "TSystem.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH1F.h" 
#include "TH2F.h"
#include "TH3F.h"
#include "TF1.h"
#include "TMath.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TPaveStats.h"
#include "TStyle.h"
#include "TProfile.h"
#include "TCutG.h"
#include "TLine.h"



#define PI 3.14159265
#define ROLLOVER_COUNT       65460         // get this when configure TDC
#define TDC_LOOKBACK         15794         // get this when configure TDC 
#define TDC_v2BIN_SIZE       0.0580   // ns/LSB
#define TDC_v3BIN_SIZE       0.1160   // ns/LSB

//=========================================================================
//                 SRS 
//=========================================================================

#include "plugins/gemrecon/old_code/GEMRawDecoder.h"
#include "plugins/gemrecon/GemInputHandler.h"
#include "plugins/gemrecon/GemFEC.h"
#include "plugins/gemrecon/GemView.h"
//#include "SFclust.h"
GemView  * gemView;
vector <SFclust> gemclust;

//=========================================================================

int DEBUG =0;

float tref;
float ave_time;
float sang=0.*PI/180.; // strip angle w.r.t. wire

int print_flg=0;

int DODisplay = 0;
int VERBOSE=10;
int DOVis = 0;
int ROCSlots[20];
int WindowSize;
int PSWindowSize;
int PSCWindowSize;

float TDCData[4][20][48][200];
int TDChits[4][20][48];
float TDCDataST[1][20][32][200];
int TDChitsST[1][20][32];
int SThits;

float ADCSamples[1][20][72][1000]; //FDC
float ADCPedestal[1][20][72]; // pedestal calculated offline
int slot_special=0;
int ch_special=0;
int samp_special=0;

float PSADCSamples[20][16][2000]; //PS
float PSADCPedestal[20][16]; // pedestal calculated offline
float PSCADCSamples[16][2000]; //PSC
float PSCADCPedestal[16]; // pedestal calculated offline


float fADCPedestal[20][20][72]; //pedestal calculated by the firmware
float fADCpeak[15][15][72][200]; //same as in fADCdata class, but calculated by the firmware
float fADCcharge[15][15][72][200]; //same as in fADCdata class, but calculated by the firmware
int fADCnhit[15][15][72]; //same as in fADCdata class, but calculated by the firmware
float fADCtime[15][15][72][200]; //same as in fADCdata class, but calculated by the firmware
  long int trig_trd[10];
  int trd_cnt;
  long int trig_st[10];
  int st_cnt;
  long int trig_ps[10];
  int ps_cnt;
int evntno_st=0;
int evntno_trd=0;
int evntno_ps=0;
int tgpoint=0;


void analyzeEvent(evioDOMTree &eventTree);
void analyzeBank(evioDOMNodeP bankPtr);
void analyzeSRS(evioDOMNodeP bankPtr);
void CountTriggers();

      float fADC[20][72][2000];
      float Ped[20][72];
      TFile* ROOTfile;
      TTree* fdcFeTree;

// trigger stuff

int NEventsInBlock;
uint64_t EVENTNUMBER;
uint64_t EVTCounts;
long TheEventNum;
uint64_t TRIGGERTIMES;
uint32_t TRIGGERTYPES;
uint32_t TRIGGER_MASK_GT;
uint32_t TRIGGER_MASK_FP;
int TheRunNumber;

 ofstream to_sergey_rad;
 ofstream to_sergey_norad;

#define gem_x_slot 1
#define gem_y_slot 4
#define gem_y_ch0 24

int GetXSlot(int gch){
return gem_x_slot+(int)gch/72;
}

int GetXChan(int gch){
    int fch=gch%72; //inversed fADC chan
    int card=fch/24;
    int cch=fch%24; //inversed card chan
    return 23-cch+card*24;
}

int GetYSlot(int gch){
return gem_y_slot+(int)gch/72;
}
int GetYChan(int gch){
    int fch=gch%72; //inversed fADC chan
    int card=fch/24;
    int cch=fch%24; //inversed card chan
    return 23-cch+card*24+gem_y_ch0;
}

int main(int argc, char *argv[]) {  
  


  char CrateName[128] = "ROCFDC";
  int version = 1;
  int ROC = 2;
  int SLOT = 3;
  int CHANNEL = 1;
  int DISPLAY = 11;

  char InputFile[128];
  int ROCNum[10] = {2,3,5,6,7,8,9,10,11,12};

  for (int i=0;i<10;i++){
  trig_trd[i]=0; 
  trig_st[i]=0; 
  trig_ps[i]=0; 
  }
  //}
  trd_cnt=0; 
  st_cnt=0; 
  ps_cnt=0; 


 to_sergey_rad.open("to_sergey_dedx_new_rad.dat",ios::out);
 to_sergey_norad.open("to_sergey_dedx_new_norad.dat",ios::out);

  TApplication theApp("App", &argc, argv);

  if (argc>1){
    
    for (int i=1;i<argc;i++){
      
      if (!strcmp(argv[i],"-ND")){
	DISPLAY = 0;
      }

      if (!strcmp(argv[i],"-V")){
        version = atoi(argv[i+1]);
        i++;
      }
      if (!strcmp(argv[i],"-R")){
        ROC = atoi(argv[i+1]);
        i++;
      }
      if (!strcmp(argv[i],"-S")){
        SLOT = atoi(argv[i+1]);
        i++;
      }
      if (!strcmp(argv[i],"-C")){
        CHANNEL = atoi(argv[i+1]);
        i++;
      }
      if (!strcmp(argv[i],"-F")){
        sprintf(CrateName,"%s",argv[i+1]);
        i++;
      }

      if (!strcmp(argv[i],"h")){
	cout<<"-ND        do not display individual events"<<endl;
        cout<<"-R #     ROC number (default 2)"<<endl;
        cout<<"-S  #    SLOT number (default 3)"<<endl;
        cout<<"-C #     CHANNEL number (default 1)"<<endl;
        cout<<"-V #     data version number (default 1)"<<endl;
	cout<<"-F string  data label (default ROCFDC) \"string\"   "<<endl;
	return 0;
      }
      
    }
  }


  sprintf(InputFile,"%s",argv[1]);


  // cedi  
  //char DataDir[128] = "/gluonraid1/Users/hdfdcops/fdcdata";
  //char DataDir[128] = "/gluonraid1/Users/hdfdcops/tagdata/DATA/";
  //char DataDir[128] = "/gluonraid1/rawdata/volatile/RunPeriod-2015-12/rawdata/Run004450";
  //char DataDir[128] = "/gluonraid2/rawdata/active/RunPeriod-2016-02/rawdata/Run010480";
  //char DataDir[128] = "/gluonraid2/rawdata/volatile/RunPeriod-2016-02/rawdata/Run010483";
  //char DataDir[128] = "/gluonraid2/rawdata/volatile/RunPeriod-2016-02/rawdata/Run010534";
  //char DataDir[128] = "/gluonraid1/rawdata/active/RunPeriod-2016-02/rawdata/Run010909";
  //char DataDir[128] = "/gluonraid1/rawdata/volatile/RunPeriod-2016-02/rawdata/Run010909";
  //char DataDir[128] = "/gluonraid1/rawdata/volatile/RunPeriod-2016-02/rawdata/Run011517";
  //char DataDir[128] = "/gluonraid3/data4/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030295";
  //char DataDir[128] = "/gluonraid3/data3/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030362";
  //char DataDir[128] = "/gluonraid3/data1/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030364";
  //char DataDir[128] = "/gluonraid3/data3/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030446";
  //char DataDir[128] = "/gluonwork1/Subsystems/FDC/TRD";
  //char DataDir[128] = "/gluonraid3/data4/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030647";
  //char DataDir[128] = "/gluonraid3/data2/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030689";
  //char DataDir[128] = "/gluonraid3/data4/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030691";
  //char DataDir[128] = "/gluonraid3/data4/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030767";
  //char DataDir[128] = "/gluonraid3/data1/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030768";
  //char DataDir[128] = "/gluonraid3/data2/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030665";
  //char DataDir[128] = "/gluonraid3/data2/rawdata/active/RunPeriod-2017-01/rawdata/Run030561";
  //char DataDir[128] = "/gluonraid3/data3/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030866";
  //char DataDir[128] = "/gluonraid3/data1/rawdata/volatile/RunPeriod-2017-01/rawdata/Run030864";
  //char DataDir[128] = "/gluonraid3/data4/rawdata/trd/DATA";                     
  char DataDir[128] = "DATA";                     
  //char DataDir[128] = "/gluonwork1/Subsystems/FDC/TRD";
  //char DataDir[128] = "/gluonwork1/Subsystems/FDC/Run011055/";
  float tmax1=452.;
  float dmax1=40.;
  float tmax2=112.;
  float dmax2=35.;
  //char DataDir[128] = "/home/furletov/ttt/daq_pro_vers/DATA/";
  //char DataDir[128] = "/gluonfs1/home/hdfdcops/code/ADCtests/readout";
  char fnam[228];
  char root_fname[228];
  ifstream INF;
  
  char lin[50000];
  //string lin;
  int NSlot = -1;
  int NCon = 0;
  int EventNew = 1;
  float x[100000];
  float y[100000];
  float dx[100000];
  float dy[100000];

  //cokXe float thresh=800.;
  //cokforAr? float thresh=500.;
  float thresh=200.;
  float sthresh=40.;
  float bthresh=40.;
  int Slot=SLOT;
      int EVENT;
      short EvtSize;
      int SlotNum; 
      int chan;
      int samples;
      int isamp=0;

         int trig=0;
         float umax=0.;
         float uped=0.;
         int uchmax=0;
         float ucent=0;
         float ufit=0;
         float umcent=0;
         float umfit=0;
         float usig=0;
         int uwid=0;
         int usmax=0;
         float dmax=0.;
         float dped=0.;
         int dchmax=0;
         float dcent=0;
         float dfit=0;
         float dmcent=0;
         float dmfit=0;
         float dsig=0;
         int dwid=0;
         int dsmax=0;
         float wmax=0.;
         float wped=0.;
         int wchmax=0;
         float wcent=0;
         float wfit=0;
         float wmcent=0;
         float wmfit=0;
         float wsig=0;
         int wwid=0;
         int wsmax=0;
         int pssmax=0;
         float w2max=0.;
         float w2ped=0.;
         int w2chmax=0;
         float pschmax=0;
         int pscchmax=0;
         float w2cent=0;
         float w2fit=0;
         float w2mcent=0;
         float w2mfit=0;
         float w2sig=0;
         int w2wid=0;
         int w2smax=0;
         float ucharge=0.;
         float dcharge=0.;
         float wcharge=0.;
         float w2charge=0.;
         float pscharge=0.;
         float psccharge=0.;
         int wnumber=0;
         int w2number=0;
         float umcharge=0.;
         float dmcharge=0.;
         float wmcharge=0.;
         float w2mcharge=0.;
         int usize=0;
         int dsize=0;
         int wsize=0;
         int w2size=0;

         int wnhit;
         float wthit[1000];
         float wahit[1000];
         float wmhit[1000];
         int unhit;
         float uthit[1000];
         float uahit[1000];

         int w2nhit;
         float w2thit[1000];
         float w2ahit[1000];
         float w2mhit[1000];
         int dnhit;
         float dthit[1000];
         float dahit[1000];

	 cout << " Create  myc " << endl;  

	 sleep(1);
	 TCanvas *myc;
	 myc = new TCanvas("myc", "Event", 50,50, 1800, 900);
	 //myc->Clear("D");
	 myc->Divide(4,4);

	 cout << " Create  myc done " << endl;  

  //=========================================================================
  //                 SRS 
  //=========================================================================
  cout << " Create  GemView  " << endl;
  gemView = new GemView();
  cout << " Create  GemView  done .." << endl;  
  //=========================================================================

  sprintf(fnam,"%s/%s",DataDir,InputFile);
  cout<<"path: "<<fnam<<" " << DataDir << " " << InputFile <<endl;
  int OK=0;
  cout<<"open file "<<fnam<<endl;
      if (FILE *file = fopen(fnam,"r")){
        fclose(file);
        OK++;
        cout<<"File exists!"<<endl;
      } else {
        OK=0;
      }
      if (OK){

    
    //cok sprintf(root_fname,"%s.%s",InputFile,"psroot");
    sprintf(root_fname,"%s.%s",InputFile,"disproot");
    cout<<" opening root file "<<root_fname<<endl;
    ROOTfile = new TFile(root_fname,"RECREATE"," 55Fe root tree results ");
    ROOTfile->cd();
    fdcFeTree = new TTree( "fdctest", "FDC response" );
    fdcFeTree->Branch( "ev", &EVENT, "ev/I" );
    fdcFeTree->Branch( "max_u", &umax, "max_u/F" );
    fdcFeTree->Branch( "max_d", &dmax, "max_d/F" );
    fdcFeTree->Branch( "max_w", &wmax, "max_w/F" );
    fdcFeTree->Branch( "max_w2", &w2max, "max_w2/F" );
    fdcFeTree->Branch( "ped_u", &uped, "ped_u/F" );
    fdcFeTree->Branch( "ped_d", &dped, "ped_d/F" );
    fdcFeTree->Branch( "ped_w", &wped, "ped_w/F" );
    fdcFeTree->Branch( "ped_w2", &w2ped, "ped_w2/F" );
    fdcFeTree->Branch( "q_u", &ucharge, "q_u/F" );
    fdcFeTree->Branch( "q_d", &dcharge, "q_d/F" );
    fdcFeTree->Branch( "q_w", &wcharge, "q_w/F" );
    fdcFeTree->Branch( "q_w2", &w2charge, "q_w2/F" );
    fdcFeTree->Branch( "w2_n", &w2number, "w2_n/I" );
    fdcFeTree->Branch( "qm_u", &umcharge, "qm_u/F" );
    fdcFeTree->Branch( "qm_d", &dmcharge, "qm_d/F" );
    fdcFeTree->Branch( "qm_w", &wmcharge, "qm_w/F" );
    fdcFeTree->Branch( "qm_w2", &w2mcharge, "qm_w2/F" );
    fdcFeTree->Branch( "ucent", &ucent, "ucent/F" );
    fdcFeTree->Branch( "ufit", &ufit, "ufit/F" );
    fdcFeTree->Branch( "umcent", &umcent, "umcent/F" );
    fdcFeTree->Branch( "umfit", &umfit, "umfit/F" );
    fdcFeTree->Branch( "usig", &usig, "usig/F" );
    fdcFeTree->Branch( "uwid", &uwid, "uwid/I" );
    fdcFeTree->Branch( "dcent", &dcent, "dcent/F" );
    fdcFeTree->Branch( "dfit", &dfit, "dfit/F" );
    fdcFeTree->Branch( "dmcent", &dmcent, "dmcent/F" );
    fdcFeTree->Branch( "dmfit", &dmfit, "dmfit/F" );
    fdcFeTree->Branch( "dsig", &dsig, "dsig/F" );
    fdcFeTree->Branch( "dwid", &dwid, "dwid/I" );
    fdcFeTree->Branch( "wcent", &wcent, "wcent/F" );
    fdcFeTree->Branch( "wfit", &wfit, "wfit/F" );
    fdcFeTree->Branch( "wmcent", &wmcent, "wmcent/F" );
    fdcFeTree->Branch( "wmfit", &wmfit, "wmfit/F" );
    fdcFeTree->Branch( "wsig", &wsig, "wsig/F" );
    fdcFeTree->Branch( "wwid", &wwid, "wwid/I" );
    fdcFeTree->Branch( "w2cent", &w2cent, "w2cent/F" );
    fdcFeTree->Branch( "w2fit", &w2fit, "w2fit/F" );
    fdcFeTree->Branch( "w2mcent", &w2mcent, "w2mcent/F" );
    fdcFeTree->Branch( "w2mfit", &w2mfit, "w2mfit/F" );
    fdcFeTree->Branch( "w2sig", &w2sig, "w2sig/F" );
    fdcFeTree->Branch( "w2wid", &w2wid, "w2wid/I" );
    fdcFeTree->Branch( "t_u", &usmax, "t_u/I" );
    fdcFeTree->Branch( "t_d", &dsmax, "t_d/I" );
    fdcFeTree->Branch( "t_w", &wsmax, "t_w/I" );
    fdcFeTree->Branch( "t_w2", &w2smax, "t_w2/I" );
    fdcFeTree->Branch( "uch", &uchmax, "uch/I" );
    fdcFeTree->Branch( "dch", &dchmax, "dch/I" );
    fdcFeTree->Branch( "wch", &wchmax, "wch/I" );
    fdcFeTree->Branch( "w2ch", &w2chmax, "w2ch/I" );
    fdcFeTree->Branch( "usz", &usize, "usz/I" );
    fdcFeTree->Branch( "dsz", &dsize, "dsz/I" );
    fdcFeTree->Branch( "wsz", &wsize, "wsz/I" );
    fdcFeTree->Branch( "w2sz", &w2size, "w2sz/I" );
    fdcFeTree->Branch( "q_ps", &pscharge, "q_ps/F" );
    fdcFeTree->Branch( "psch", &pschmax,"psch/F" );
    fdcFeTree->Branch( "t_ps", &pssmax, "t_ps/I" );
    fdcFeTree->Branch( "q_psc", &psccharge, "q_psc/F" );
    fdcFeTree->Branch( "pscch", &pscchmax,"pscch/I" );
    fdcFeTree->Branch( "trig", &trig,"trig/I" );
    fdcFeTree->Branch( "wnhit", &wnhit, "wnhit/I" );
    fdcFeTree->Branch( "wthit", &wthit, "wthit[wnhit]/F" );
    fdcFeTree->Branch( "wahit", &wahit, "wahit[wnhit]/F" );
    fdcFeTree->Branch( "wmhit", &wmhit, "wmhit[wnhit]/F" );
    fdcFeTree->Branch( "unhit", &unhit, "unhit/I" );
    fdcFeTree->Branch( "uthit", &uthit, "uthit[unhit]/F" );
    fdcFeTree->Branch( "uahit", &uahit, "uahit[unhit]/F" );

    //-----------------  SF book --------------------------

    TH1D *hp0x = new TH1D("hp0x","p0x",100,-100.,100.);
    TH1D *hp1x = new TH1D("hp1x","p1x",100,-2.,2.);
    TH1D *hp0y = new TH1D("hp0y","p0y",100,-100.,100.);
    TH1D *hp1y = new TH1D("hp1y","p1y",100,-2.,2.);

    TH2F *zwct_plot2  = new TH2F("zwct_plot2"," p0x p0y  WC ",300,-80.,40, 300, 0., 20.2);
    TH1F *zwct_plot  = new TH1F("zwct_plot"," zero nc fx0 WC ",300,-80.,40.);
    TH2F *nc2totrad    = new TH2F("nc2totrad","nc vx tot rad WC ", 50, -0.5, 49.5,25, -0.5, 24.5 );
    TH2F *nc2totno     = new TH2F("nc2totno","nc vs tot no rad WC " , 50, -0.5, 49.5,25, -0.5, 24.5);

    TH2F *swct_plot  = new TH2F("swct_plot","WC ",24,-12.05*10.,11.95*10.,300,-0.5/10.,299.5/10.);
    TH2F *swcty_plot = new TH2F("swcty_plot","WC ",48,-24.05*5.,23.95*5.,300,-0.5/10.,299.5/10.);
    TH2F *sct_plot   = new TH2F("sct_plot","GEM ",600,-300.5/2.5,299.5/2.5,300,-0.5/10.,299.5/10.);
    TH2F *scty_plot  = new TH2F("scty_plot","GEM ",600,-300.5/2.5,299.5/2.5,300,-0.5/10.,299.5/10.);

    TH2F *chi2xy = new TH2F("chi2xy","#chi^{2} x y ", 100, 0., 100., 100, 0., 100.);
    TH2F *wc2gemx = new TH2F("wc2gemx","x WC vs GEM ",300, -0.5/10.,299.5/10.,  300, -0.5/10.,299.5/10.);
    TH2F *wc2gemy = new TH2F("wc2gemy","y WC vs GEM ",600, -300.5/2.5,299.5/2.5,  600, -300.5/2.5,299.5/2.5);
    
    TH2F *xcom_plot = new TH2F("ct_plot","GEM ",1000,-1000.5/10.,299.5/10.,600,-300.5/2.5,299.5/2.5);

    TH1D *nclrad = new TH1D("nclrad"," nclust rad WC",15,-0.5,14.5);
    TH1D *nclno  = new TH1D("nclno"," nclust no rad WC",15,-0.5,14.5);

    TH1D *nclradg = new TH1D("nclradg"," nclust rad GEM",15,-0.5,14.5);
    TH1D *nclnog  = new TH1D("nclnog"," nclust no rad GEM",15,-0.5,14.5);

    TH1D *caradg = new TH1D("caradg"," clust amp rad GEM",100,-0.5,4096.);
    TH1D *canog  = new TH1D("canog"," clust amp no rad GEM",100,-0.5,4096.);


    //---------------------- SF end ------------------------
    //---------------------- SRS hist ----------------------
    /*
    TH1F **f1DSingleEventHist;
    TH2F  **f2DSingleEventHist;
    f1DSingleEventHist    = new TH1F * [nbOfPlaneHists] ;
    f2DSingleEventHist    = new TH2F * [nbOfPlaneHists] ;
    */
    //---------------------- SRS end ----------------------

    TH1D *FHist = new TH1D("FHist","Histogram for centroid fitting",7,-0.5,6.5);

    gStyle->SetOptStat(1);
    //TH2F *ct_plot = new TH2F("ct_plot","GEM X vs Z",600,-120.5/2.5,479.5/2.5,300,-0.5/10.,299.5/10.);
    TH2F *ct_plot = new TH2F("ct_plot","GEM ",600,-300.5/2.5,299.5/2.5,300,-0.5/10.,299.5/10.);
    TH2F *rct_plot = new TH2F("rct_plot","GEM ",300,-0.5/10.,299.5/10.,600,-300.5/2.5,299.5/2.5);
    TAxis *xaxis = (TAxis*)ct_plot->GetXaxis();
    xaxis->SetTitle("x, mm  ");
    TAxis *yaxis = (TAxis*)ct_plot->GetYaxis();
    yaxis->SetTitle("z, mm  ");
    int w2choffset=-180;
    ct_plot->SetMaximum(4000.);
    //TH2F *wct_plot = new TH2F("wct_plot","WC X vs Z",24,-0.5*10.,23.5*10.,300,-0.5/10.,299.5/10.);
    TH2F *wct_plot = new TH2F("wct_plot","WC ",24,-12.05*10.,11.95*10.,300,-0.5/10.,299.5/10.);
    xaxis = (TAxis*)wct_plot->GetXaxis();
    xaxis->SetTitle("x, mm  ");
    yaxis = (TAxis*)wct_plot->GetYaxis();
    yaxis->SetTitle("z, mm  ");
    int wchoffset=-12;
    wct_plot->SetMaximum(4000.);
    //TH2F *cty_plot = new TH2F("cty_plot","GEM Y vs Z",600,-324.5/2.5,275.5/2.5,300,-0.5/10.,299.5/10.);
    TH2F *cty_plot = new TH2F("cty_plot","GEM ",600,-300.5/2.5,299.5/2.5,300,-0.5/10.,299.5/10.);
    TH2F *rcty_plot = new TH2F("rcty_plot","GEM ",300,-0.5/10.,299.5/10.,600,-300.5/2.5,299.5/2.5);
    xaxis = (TAxis*)cty_plot->GetXaxis();
    xaxis->SetTitle("y, mm  ");
    yaxis = (TAxis*)cty_plot->GetYaxis();
    yaxis->SetTitle("z, mm  ");
    int dchoffset=12;
    cty_plot->SetMaximum(3000.);
    //TH2F *wcty_plot = new TH2F("wcty_plot","WC Y vs Z",48,-12.5*5.,35.5*5.,300,-0.5/10.,299.5/10.);
    TH2F *wcty_plot = new TH2F("wcty_plot","WC ",48,-24.05*5.,23.95*5.,300,-0.5/10.,299.5/10.);
    xaxis = (TAxis*)wcty_plot->GetXaxis();
    xaxis->SetTitle("y, mm  ");
    yaxis = (TAxis*)wcty_plot->GetYaxis();
    yaxis->SetTitle("z, mm  ");
    int uchoffset=-12;
    wcty_plot->SetMaximum(400.);

    TH2D *PStest = new TH2D("PStest","PS time vs chmax",100,-0.5,99.5,256,0.5,255.5);
    //TRIGtest= new TH2D("TRIGtest","TRIG time test",1000,-10000000.5,10000000.-0.5,100,0.5,1000.5);
   //cok 3d  TH3D *wAvsT = new TH3D("wAvsT","Amplitude vs Time",500,-0.5,499.5,1024,-0.5,4095.5,48,-0.5,47.5);
   //cok 3d  TH3D *w2AvsT = new TH3D("w2AvsT","Amplitude vs Time",500,-0.5,499.5,1024,-0.5,4095.5,48,-0.5,47.5);
    TH1D *xeff = new TH1D("xeff","X-coordinate of the track",48,-0.5,47.5);
    TH1D *x2eff = new TH1D("x2eff","X-coordinate of the track",48,-0.5,47.5);
    //TH2D *wAvsT = new TH2D("wAvsT","Ch.1 Amplitude vs Time",120,-0.5,119.5,110,-0.5,109.5);
    //cok TH2D *wAvsT = new TH2D("wAvsT","Ch.1 Amplitude vs Time",120,-0.5,119.5,24,-0.5,23.5);
    TH2D *wAvsT = new TH2D("wAvsT","Ch.1 Amplitude vs Time",500,-0.5,499.5,24,-0.5,23.5);
    TH2D *wAvsTnorm = new TH2D("wAvsTnorm","Ch.1 Amplitude vs Time norm",500,-0.5,499.5,24,-0.5,23.5);
    //TH2D *wAvsT = new TH2D("wAvsT","Ch.1 Amplitude vs Time",120,-0.5,119.5,100,9.5,13.5);
    TH2D *wCvsT = new TH2D("wCvsT","Ch.1 Width (transverese) vs Time",500,-0.5,499.5,24,-0.5,23.5);
    TH2D *wCvsL = new TH2D("wCvsL","Ch.1 Width (longitudinal) vs Time",500,-0.5,499.5,24,-0.5,23.5);
    TH2D *wAvsB = new TH2D("wAvsB","Ch.1 Amplitude vs Time (background)",1000,-0.5,999.5,48,-0.5,47.5);
    TH2D *wCvsB = new TH2D("wCvsB","Ch.1 Counts vs Time (background)",1000,-0.5,999.5,48,-0.5,47.5);
    TH2D *wYvsB = new TH2D("wYvsB","Ch.1 XY vs Time (background)",48,-0.5,47.5,96,-48.,48.);
    TH2D *wTYvsB = new TH2D("wTYvsB","Ch.1 TY vs Time (background)",1000,-0.5,999.5,96,-48.,48.);
    TH2D *wAvsD = new TH2D("wAvsD","Ch.1 Amplitude vs Distance",1000,0.,40.,48,-0.5,47.5);
    wAvsT->Sumw2();
    wCvsT->Sumw2();
    wAvsB->Sumw2();
    wCvsB->Sumw2();
    wYvsB->Sumw2();
    wTYvsB->Sumw2();
    wAvsD->Sumw2();
    TH2D *w2AvsT = new TH2D("w2AvsT","Ch.2 Amplitude vs Time",500,-0.5,499.5,240,-0.5,239.5);
    TH2D *w2AvsTnorm = new TH2D("w2AvsTnorm","Ch.2 Amplitude vs Time norm",500,-0.5,499.5,240,-0.5,239.5);
    TH2D *w2CvsT = new TH2D("w2CvsT","Ch.2 Width (transverse) vs Time",500,-0.5,499.5,240,-0.5,239.5);
    TH2D *w2CvsL = new TH2D("w2CvsL","Ch.2 Width (longitudinal) vs Time",500,-0.5,499.5,240,-0.5,239.5);
    TH2D *w2AvsB = new TH2D("w2AvsB","Ch.2 Amplitude vs Time (background)",1000,-0.5,999.5,240,-0.5,249.5);
    TH2D *w2CvsB = new TH2D("w2CvsB","Ch.2 Counts vs Time (background)",1000,-0.5,999.5,240,-0.5,249.5);
    TH2D *w2YvsB = new TH2D("w2YvsB","Ch.2 X vs Y (background)",240,-0.5,249.5,240,-0.5,249.5);
    TH2D *w2TYvsB = new TH2D("w2TYvsB","Ch.1 TY vs Time (background)",1000,-0.5,999.5,240,-0.5,249.5);
    TH2D *w2AvsD = new TH2D("w2AvsD","Ch.2 Amplitude vs Distance",1000,0.,35.,240,-0.5,249.5);
    w2AvsTnorm->Sumw2();
    w2AvsT->Sumw2();
    w2CvsT->Sumw2();
    w2CvsL->Sumw2();
    w2AvsB->Sumw2();
    w2CvsB->Sumw2();
    w2YvsB->Sumw2();
    w2TYvsB->Sumw2();
    w2AvsD->Sumw2();
    TH2D *uAvsT = new TH2D("uAvsT","Amplitude vs Time",1000,-0.5,999.5,4096,-0.5,4095.5);
    TH2D *dAvsT = new TH2D("dAvsT","Amplitude vs Time",500,-0.5,499.5,4096,-0.5,4095.5);

    //TH2D *TTime = new TH2D("TTime","Time between peaks vs time",1000,-0.5,999.5,1000,-0.5,999.5);
    TH2D *Timeax = new TH2D("Timeax","Time between peaks vs time",1000,-0.5,999.5,48,-24.5,23.5);
    TH2D *Timebx = new TH2D("Timebx","Time between peaks vs time",1000,-0.5,999.5,48,-24.5,23.5);
    TH2D *DTimeax = new TH2D("DTimeax","Time between peaks vs time",1000,-0.5,999.5,48,-24.5,23.5);
    TH2D *DTimebx = new TH2D("DTimebx","Time between peaks vs time",1000,-0.5,999.5,48,-24.5,23.5);
    TH2D *CluCou = new TH2D("CluCou","cluster no. in ch.1 vs ch2",100,-0.5,99.5,100,-0.5,99.5);
         //TF1 *centroid=new TF1("centroid","[0]*(1-tanh([2]*(x-[1]))*tanh([2]*(x-[1])))",N1Up-0.5,N1Up+Aup-0.5);
         TF1 *centroid=new TF1("centroid","[0]*exp(-(x-[1])^2/(2.*[2]^2))",-0.5,6.5);

        for(int slot=0;slot<15;slot++){
          for(int ch=0;ch<72;ch++){
              for (int sm=0;sm<2000;sm++){
	        ADCSamples[0][slot][ch][sm] = -1000.;
              }
	    ADCPedestal[0][slot][ch] = 0.;

          }
        }
  long int evtCount=0;
  long int NEVENT=1000000;
  evioFileChannel EvOchan(fnam,"r",8000000);
  EvOchan.open();
        cout<<" here 1"<<endl;
        bool ReadBack = EvOchan.read();
        cout<<" here 2"<<endl;
        while(ReadBack && evtCount<NEVENT) {
        evtCount++;
        // begin event loop
           if(evtCount%100==0)
           //if(evtCount%1==0)
           cout<<"[][][][][][][][][][][][] new event "<<evtCount<<" [][][][][][][][][][]"<<endl;

        //if(evtCount==91)DISPLAY=1; 
        if(11==11) {
       // cout<<"[][][][][][][][][][][][] new event "<<evtCount<<" [][][][][][][][][][]"<<endl;
   try {
      
   TRIGGER_MASK_GT=0;
            evioDOMTree eventTree(EvOchan);
            analyzeEvent(eventTree);

	    if (gemclust.size()!=1) continue;

   trig=TRIGGER_MASK_GT;
  /*
  if (TRIGGER_MASK_GT>0){
      if (TRIGGER_MASK_GT & (1<<3)){
        trig=4;
      } else if (trig==3) {
        trig=100;
      }
  }
  */

         //if(evtCount<NEVENT&&evtCount>0)
         if(true){
         // slot1: cell1_w1, cell1_u1
         int slot1=0;

         int sl_c1w1=slot1;
         int ch_c1w1=0;

         int sl_c1u1=slot1;
         int ch_c1u1=24;

         int sl_c2w1=1;
         int ch_c2w1=0;
         int sl_c2w2=1;
         int ch_c2w2=24;
         int sl_c2w3=1;
         int ch_c2w3=48;

         int sl_c2w4=2;
         sl_c2w4=3;
         int ch_c2w4=0;
         int sl_c2w5=2;
         int ch_c2w5=24;
         int sl_c2w6=2;
         int ch_c2w6=48;

         int sl_c2w7=3;
         int ch_c2w7=0;
         int sl_c2w8=3;
         int ch_c2w8=24;
         int sl_c2w9=3;
         int ch_c2w9=48;

         int sl_c2w10=4;
         int ch_c2w10=0;
         int sl_c2d1=4;
         int ch_c2d1=24;
         int sl_c2d2=4;
         int ch_c2d2=48;

         float uADCmax[72][500];
         float wADCmax[72][500];
         float uADCsum[72];
         float wADCsumall[72];
         float wADCsum[72][500];

         int uSAMPmax[72][500];
         int wSAMPmax[72][500];
         int wNSAMP[72][500];
         int uNhit[72];
         int wNhit[72];
         int uSmax[72];
         int wSmax[72];
         float uAmax[72];
         float wAmax[72];
         float wAmin[72];

         float dADCmax[72][500];
         float w2ADCmax[240][500];
         float dADCsum[72];
         float w2ADCsumall[240];
         float w2ADCsum[240][500];

         int dSAMPmax[72][500];
         int w2SAMPmax[240][500];
         int w2NSAMP[240][500];
         int dNhit[72];
         int w2Nhit[240];
         int dSmax[72];
         int w2Smax[240];
         float dAmax[72];
         float w2Amax[240];
         float w2Amin[240];

         float adcval;
         float adcval1;
         float adcval2;
 
      // fixed pedestal !!!
      //  for(int slot=0;slot<15;slot++){
          for(int ch=0;ch<72;ch++){
	  //  ADCPedestal[0][uslot][ch] = 100.;
          }
      //  }
      // fixed pedestal !!!

         samples=WindowSize;
         //cout<<"samples = "<<samples<<endl;

//
// Cell1 U-strips
//
         int uwd[24];
         for (int ch=0;ch<24;ch++){
            uADCsum[ch]=0.;
            uwd[ch]=0.;
            uSmax[ch]=0;
            uAmax[ch]=0.;
            for (int hit=0;hit<500;hit++){
              uADCmax[ch][hit]=0.;
              uSAMPmax[ch][hit]=0;
            }
         }
         for (int ch=0;ch<24;ch++){
             int slot; int ch0;
                slot=sl_c1u1; ch0=ch_c1u1;
             uNhit[ch]=0;
             float adcmax=0.;
             for (int i=1;i<samples-1;i++){
                adcval=ADCSamples[0][slot][ch0+ch][i]-ADCPedestal[0][slot][ch0+ch];
                adcval1=ADCSamples[0][slot][ch0+ch][i-1]-ADCPedestal[0][slot][ch0+ch];
                adcval2=ADCSamples[0][slot][ch0+ch][i+1]-ADCPedestal[0][slot][ch0+ch];
                if(adcval>thresh/18.){
                  uADCsum[ch]+=adcval;
                  uwd[ch]++;
                  if(adcval>adcval1&&adcval>adcval2){
                    uADCmax[ch][uNhit[ch]]=adcval;
                    uSAMPmax[ch][uNhit[ch]]=i;
                    uNhit[ch]++;
                    if(adcval>adcmax){
                      adcmax=adcval;
                      uAmax[ch]=adcval;
                      uSmax[ch]=i;
                    }
                  }
                }
             }
         }


//
// Cell1 wires
//
         int wwd[24];
         for (int ch=0;ch<24;ch++){
            wADCsumall[ch]=0.;
            wwd[ch]=0.;
            wSmax[ch]=0;
            wAmax[ch]=0.;
            wAmin[ch]=5000.;
            for (int hit=0;hit<500;hit++){
              wADCmax[ch][hit]=0.;
              wSAMPmax[ch][hit]=0;
              wADCsum[ch][hit]=0.;
              wNSAMP[ch][hit]=0;
            }
         }
         for (int ch=0;ch<24;ch++){
             int slot; int ch0;
                slot=sl_c1w1; ch0=ch_c1w1;
             wNhit[ch]=0;
             float adcmax=0.;
             for (int i=4;i<samples-4;i++){
                adcval=ADCSamples[0][slot][ch+ch0][i]-ADCPedestal[0][slot][ch+ch0];
                adcval1=ADCSamples[0][slot][ch+ch0][i-1]-ADCPedestal[0][slot][ch+ch0];
                adcval2=ADCSamples[0][slot][ch+ch0][i+1]-ADCPedestal[0][slot][ch+ch0];
                if(adcval<wAmin[ch])wAmin[ch]=adcval;
                if(adcval>thresh){
                  wADCsumall[ch]+=adcval;
                  wwd[ch]++;
                  if(adcval>adcval1&&adcval>adcval2){
                    wADCmax[ch][wNhit[ch]]=adcval;
                    wSAMPmax[ch][wNhit[ch]]=i;
                    for (int is=-20;is<21;is++){
                        float adcl=ADCSamples[0][slot][ch+ch0][i+is]-ADCPedestal[0][slot][ch+ch0];
                        if(adcl>thresh/2.){
                           wADCsum[ch][wNhit[ch]]+=adcl;
                           wNSAMP[ch][wNhit[ch]]++;
                        }
                    }
                    wNhit[ch]++;
                    if(adcval>adcmax){
                      adcmax=adcval;
                      wAmax[ch]=adcval;
                      wSmax[ch]=i;
                      wped=ADCPedestal[0][slot][ch+ch0];
                    }
                  }
                }
             }
         }

//
// Cell1 2D clustering
//

         float wcht[24][500]; // amplitude vs channel and sample
         for (int ch=0;ch<24;ch++){
         for (int sm=0;sm<500;sm++){
            wcht[ch][sm]=0.;
         }
         }

         int wnclust=0;
         float wclustamp[200]; //cluster amplitide
         float wclustchn[200]; //cluster channel no
         float wclustsmp[200]; //cluster time (sample no)
         float wclusttwd[200]; //cluster transverse width (in channels)
         float wclustlwd[200]; //cluster lateral width (in samples)
         for (int i=0;i<200;i++){
            wclustamp[i]=0.;
            wclustchn[i]=0.; 
            wclustsmp[i]=0.;
            wclusttwd[i]=0.;
            wclustlwd[i]=0.;
         }

         for (int ch=0;ch<24;ch++){
            for (int ihit=0;ihit<wNhit[ch];ihit++){
                int sm=wSAMPmax[ch][ihit];
                wcht[ch][sm]=wADCsum[ch][ihit];
            } //end hit loop
         } //end channel loop

         for (int ch=2;ch<22;ch++){
            for (int ihit=0;ihit<wNhit[ch];ihit++){
                int sm=wSAMPmax[ch][ihit];
                if(sm>500)break;
                float a=wcht[ch][sm];

                if((a>wcht[ch-2][sm]&&a>wcht[ch-2][sm-1]&&a>wcht[ch-2][sm+1])
                 &&(a>wcht[ch-1][sm]&&a>wcht[ch-1][sm-1]&&a>wcht[ch-1][sm+1])
                 &&(a>wcht[ch+2][sm]&&a>wcht[ch+2][sm-1]&&a>wcht[ch+2][sm+1])
                 &&(a>wcht[ch+1][sm]&&a>wcht[ch+1][sm-1]&&a>wcht[ch+1][sm+1])) { //2d max simple condition

                   wclustsmp[wnclust]=sm; //take the time at the maximum as cluster time
                   wclustchn[wnclust]=ch; //take the time at the maximum as cluster time
                   wclustlwd[wnclust]=wNSAMP[ch][ihit]; //number of samples for the maximum (longitudinal width)

                   int ch1=ch-2; //now sum all amplitudes around if > amax/5
                   if(ch1<0)ch1=0;
                   int ch2=ch+2;
                   if(ch2>23)ch2=23;
                   int chwid=0;
                   for (int ich=ch1;ich<ch2+1;ich++){
                      bool above_thresh=false;
                      for (int is=-20;is<21;is++){
                         float amp=wcht[ich][sm+is];
                         if(amp>thresh){
                           wclustamp[wnclust]+=amp;
                           above_thresh=true;
                         }
                      }
                      if(above_thresh)wclusttwd[wnclust]++;
                   }
                   wnclust++;
                } //end 2d max condition
            } //end hit loop
         } //end channel loop

// end c1 2d clustering

         
//
// Cell2 d-strips
//
         int dwd[48];
         for (int ch=0;ch<48;ch++){
            dADCsum[ch]=0.;
            dwd[ch]=0.;
            dSmax[ch]=0;
            dAmax[ch]=0.;
            for (int hit=0;hit<500;hit++){
              dADCmax[ch][hit]=0.;
              dSAMPmax[ch][hit]=0;
            }
         }
         for (int ch=0;ch<48;ch++){
             int slot=GetYSlot(ch);
             int dch=GetYChan(ch);
             dNhit[ch]=0;
             float adcmax=0.;
             for (int i=1;i<samples-1;i++){
                adcval=ADCSamples[0][slot][dch][i]-ADCPedestal[0][slot][dch];
                adcval1=ADCSamples[0][slot][dch][i-1]-ADCPedestal[0][slot][dch];
                adcval2=ADCSamples[0][slot][dch][i+1]-ADCPedestal[0][slot][dch];
                if(adcval>thresh){
                  dADCsum[ch]+=adcval;
                  dwd[ch]++;
                  if(adcval>adcval1&&adcval>adcval2){
                    dADCmax[ch][dNhit[ch]]=adcval;
                    dSAMPmax[ch][dNhit[ch]]=i;
                    dNhit[ch]++;
                    if(adcval>adcmax){
                      adcmax=adcval;
                      dAmax[ch]=adcval;
                      dSmax[ch]=i;
                    }
                  }
                }
             }
         }


//
// Cell2 wires
//
         int w2wd[240];
         for (int ch=0;ch<240;ch++){
            w2ADCsumall[ch]=0.;
            w2wd[ch]=0.;
            w2Smax[ch]=0;
            w2Amax[ch]=0.;
            w2Amin[ch]=5000.;
            for (int hit=0;hit<500;hit++){
              w2ADCmax[ch][hit]=0.;
              w2SAMPmax[ch][hit]=0;
              w2NSAMP[ch][hit]=0;
              w2ADCsum[ch][hit]=0.;
            }
         }
         for (int ch=0;ch<240;ch++){
             int slot=GetXSlot(ch);
             int dch=GetXChan(ch);
             w2Nhit[ch]=0;
             float adcmax=0.;
             for (int i=10;i<samples-10;i++){
                adcval=ADCSamples[0][slot][dch][i]-ADCPedestal[0][slot][dch];
                adcval1=ADCSamples[0][slot][dch][i-1]-ADCPedestal[0][slot][dch];
                adcval2=ADCSamples[0][slot][dch][i+1]-ADCPedestal[0][slot][dch];
                if(adcval<w2Amin[ch])w2Amin[ch]=adcval;
                if(adcval>thresh){
                  w2ADCsumall[ch]+=adcval;
                  w2wd[ch]++;
                  if(adcval>adcval1&&adcval>adcval2){
                    w2ADCmax[ch][w2Nhit[ch]]=adcval;
                    w2SAMPmax[ch][w2Nhit[ch]]=i;
                    for (int is=-20;is<21;is++){
                        float adcl=ADCSamples[0][slot][dch][i+is]-ADCPedestal[0][slot][dch];
                        if(adcl>thresh/2.){
                           w2ADCsum[ch][w2Nhit[ch]]+=adcl;
                           w2NSAMP[ch][w2Nhit[ch]]++;
                        }
                    }
                    w2Nhit[ch]++;
                    if(adcval>adcmax){
                      adcmax=adcval;
                      w2Amax[ch]=adcval;
                      w2Smax[ch]=i;
                      w2ped=ADCPedestal[0][slot][dch];
                    }
                  }
                }
             }
         }

//
// Cell2 2D clustering
//

         float w2cht[240][500]; // amplitude vs channel and sample
         for (int ch=0;ch<240;ch++){
         for (int sm=0;sm<500;sm++){
            w2cht[ch][sm]=0.;
         }
         }

         int w2nclust=0;
         float w2clustamp[200]; //cluster amplitide
         float w2clustchn[200]; //cluster channel no
         float w2clustsmp[200]; //cluster time (sample no)
         float w2clusttwd[200]; //cluster transverse width (in channels)
         float w2clustlwd[200]; //cluster lateral width (in samples)
         for (int i=0;i<200;i++){
            w2clustamp[i]=0.;
            w2clustchn[i]=0.; 
            w2clustsmp[i]=0.;
            w2clusttwd[i]=0.;
            w2clustlwd[i]=0.;
         }

         for (int ch=0;ch<240;ch++){
            for (int ihit=0;ihit<w2Nhit[ch];ihit++){
                int sm=w2SAMPmax[ch][ihit];
                w2cht[ch][sm]=w2ADCsum[ch][ihit];
            } //end hit loop
         } //end channel loop

         for (int ch=2;ch<238;ch++){
            for (int ihit=0;ihit<w2Nhit[ch];ihit++){
                int sm=w2SAMPmax[ch][ihit];
                if(sm>500)break;
                float a=w2cht[ch][sm];

                if((a>w2cht[ch-2][sm]&&a>w2cht[ch-2][sm-1]&&a>w2cht[ch-2][sm+1])
                 &&(a>w2cht[ch-1][sm]&&a>w2cht[ch-1][sm-1]&&a>w2cht[ch-1][sm+1])
                 &&(a>w2cht[ch+2][sm]&&a>w2cht[ch+2][sm-1]&&a>w2cht[ch+2][sm+1])
                 &&(a>w2cht[ch+1][sm]&&a>w2cht[ch+1][sm-1]&&a>w2cht[ch+1][sm+1])) { //2d max simple condition

                   w2clustsmp[w2nclust]=sm; //take the time at the maximum as cluster time
                   w2clustchn[w2nclust]=ch; //take the time at the maximum as cluster time
                   w2clustlwd[w2nclust]=w2NSAMP[ch][ihit]; //number of samples for the maximum (longitudinal width)

                   int ch1=ch-20; //now sum all amplitudes around if > amax/5
                   if(ch1<0)ch1=0;
                   int ch2=ch+20;
                   if(ch2>239)ch2=239;
                   int chwid=0;
                   for (int ich=ch1;ich<ch2+1;ich++){
                      bool above_thresh=false;
                      for (int is=-20;is<21;is++){
                         float amp=w2cht[ich][sm+is];
                         if(amp>thresh){
                           w2clustamp[w2nclust]+=amp;
                           above_thresh=true;
                         }
                      }
                      if(above_thresh)w2clusttwd[w2nclust]++;
                   }
                   w2nclust++;
                } //end 2d max condition
            } //end hit loop
         } //end channel loop

// end c2 2d clustering

         umcharge=0.;
         usmax=-100;
         uchmax=-100;
         dmcharge=0.;
         dsmax=-100;
         dchmax=-100;
         wmcharge=0.;
         wsmax=-100;
         pssmax=-100;
         wchmax=-100;
         w2mcharge=0.;
         w2smax=-100;
         w2chmax=-100;
         pschmax=-100.;
         wsize=0;
         usize=0;
         dsize=0;
         w2size=0;
         wcharge=0;
         w2charge=0;
         pscharge=0;
         ucharge=0;
         dcharge=0;
         wwid=0.;
         w2wid=0.;
         uwid=0.;
         dwid=0.;


//check
         for (int ch=0;ch<24;ch++){
            if(wAmax[ch]>thresh){
               wsize++;
               wcharge+=wADCsumall[ch];
               wwid+=wwd[ch];
            }
            if(wAmax[ch]>wmcharge){
               wmcharge=wAmax[ch];
               wchmax=ch;
               wsmax=wSmax[ch];
               //cok wwid=wwd[ch];
            }
         }
         for (int ch=0;ch<24;ch++){
            if(uAmax[ch]>thresh/18.){
               usize++;
               ucharge+=uADCsum[ch];
               uwid+=uwd[ch];
            }
            if(uAmax[ch]>umcharge){
               umcharge=uAmax[ch];
               uchmax=ch;
               usmax=uSmax[ch];
               //cok uwid=uwd[ch];
            }
         }

         for (int ch=0;ch<240;ch++){
            if(w2Amax[ch]>thresh){
               w2size++;
               w2charge+=w2ADCsumall[ch];
               //w2wid+=w2wd[ch];
            }
            if(w2Amax[ch]>w2mcharge){
               w2mcharge=w2Amax[ch];
               w2chmax=ch;
               w2smax=w2Smax[ch];
               //cok w2wid=w2wd[ch];
            }
         }
         for (int ch=0;ch<48;ch++){
            if(dAmax[ch]>thresh){
               dsize++;
               dcharge+=dADCsum[ch];
               dwid+=dwd[ch];
            }
            if(dAmax[ch]>dmcharge){
               dmcharge=dAmax[ch];
               dchmax=ch;
               dsmax=dSmax[ch];
               //cok dwid=dwd[ch];
            }
         }


         if(wchmax>0&&uchmax>0){
         umax=0.;
         for (int ch=0;ch<24;ch++){
             int slot; int ch0;
                slot=sl_c1u1; ch0=ch_c1u1;
             adcval=uADCmax[ch][0];
             //adcval=uADCsum[ch];
             if(adcval>umax){
                umax=adcval;
                uchmax=ch;
                usmax=uSAMPmax[ch][0];
                uped=ADCPedestal[0][slot][ch+ch0];
                //cok uwid=uwd[ch];
             }
         }

         wmax=0.;
         for (int ch=0;ch<24;ch++){
             int slot; int ch0;
                slot=sl_c1w1; ch0=ch_c1w1;
             adcval=wADCmax[ch][0];
             //adcval=wADCsum[ch];
             if(adcval>wmax){
                wmax=adcval;
                wchmax=ch;
                wsmax=wSAMPmax[ch][0];
                wped=ADCPedestal[0][slot][ch+ch0];
                //cok wwid=wwd[ch];
             }
         }

         ucharge=0.;
         umcharge=0.;
         ucent=0.;
         ufit=0.;
         umcent=0.;
         umfit=0.;

         int ch1=uchmax-3;
         if(ch1<0)ch1=0;
         int ch2=uchmax+3;
         if(ch2>23)ch2=23;

         usig=0.;
         usize=0;
         FHist->Reset();
         for (int ich=ch1;ich<ch2+1;ich++){
            adcval=uADCmax[ich][0];
            if(adcval>thresh){
               umcharge+=adcval;
               umcent+=(float)adcval*ich;
               FHist->Fill((double)ich-ch1,(double)adcval);
               usize++;
            }
         }
         umcent/=umcharge;
         centroid->SetParameter(0,(double)uADCmax[uchmax][0]);
         centroid->SetParameter(1,(double)2.);
         centroid->SetParameter(2,(double)1.);
         if(usize>2){
         TFitResultPtr res=FHist->Fit("centroid","QNS");
         int fstatus=res;
         if(fstatus==0){
           umfit=(float)centroid->GetParameter(1)+(float)ch1;
           usig=abs((float)centroid->GetParameter(2));
        //   umcharge=centroid->GetParameter(0)*usig*sqrt(2.*PI);
         }
         }

         usize=0;
         FHist->Reset();
         for (int ich=ch1;ich<ch2+1;ich++){
            adcval=uADCsum[ich];
            if(adcval>thresh/18.){
               ucharge+=adcval;
               ucent+=(float)adcval*ich;
               FHist->Fill((double)ich-ch1,(double)adcval);
               usize++;
            }
         }
         ucent/=ucharge;
         centroid->SetParameter(0,(double)uADCsum[uchmax]);
         centroid->SetParameter(1,(double)2.);
         centroid->SetParameter(2,(double)1.);
         if(usize>2){
         TFitResultPtr res=FHist->Fit("centroid","QNS");
         int fstatus=res;
         if(fstatus==0){
           ufit=(float)centroid->GetParameter(1)+(float)ch1;
         //  usig=abs((float)centroid->GetParameter(2));
         //  ucharge=centroid->GetParameter(0)*usig*sqrt(2.*PI);
         }
         }
         

         wcharge=0.;
         wmcharge=0.;
         wcent=0.;
         wfit=0.;
         wmcent=0.;
         wmfit=0.;
         wsig=0.;
         wsize=0;

         ch1=wchmax-3;
         if(ch1<0)ch1=0;
         ch2=wchmax+3;
         if(ch2>23)ch2=23;


         wsig=0.;
         wsize=0;
         FHist->Reset();
         for (int ich=ch1;ich<ch2+1;ich++){
            adcval=wADCmax[ich][0];
            if(adcval>thresh){
               wmcharge+=adcval;
               wmcent+=(float)adcval*ich;
               FHist->Fill((double)ich-ch1,(double)adcval);
               wsize++;
            }
         }
         wmcent/=wmcharge;
         centroid->SetParameter(0,(double)wADCmax[wchmax][0]);
         centroid->SetParameter(1,(double)2.);
         centroid->SetParameter(2,(double)1.);
         if(wsize>2){
         TFitResultPtr res=FHist->Fit("centroid","QNS");
         int fstatus=res;
         if(fstatus==0){
           wmfit=(float)centroid->GetParameter(1)+(float)ch1;
           //cok wsig=abs((float)centroid->GetParameter(2));
        //   wmcharge=centroid->GetParameter(0)*wsig*sqrt(2.*PI);
         }
         }

         wsize=0;
         FHist->Reset();
         for (int ich=ch1;ich<ch2+1;ich++){
            adcval=wADCsumall[ich];
            if(adcval>sthresh){
               wcharge+=adcval;
               wcent+=(float)adcval*ich;
               FHist->Fill((double)ich-ch1,(double)adcval);
               wsize++;
            }
         }
         wcent/=wcharge;
         centroid->SetParameter(0,(double)wADCsumall[wchmax]);
         centroid->SetParameter(1,(double)2.);
         centroid->SetParameter(2,(double)1.);
         if(wsize>2){
         TFitResultPtr res=FHist->Fit("centroid","QNS");
         int fstatus=res;
         if(fstatus==0){
           wfit=centroid->GetParameter(1)+ch1;
        //   wsig=abs(centroid->GetParameter(2));
        //   wcharge=centroid->GetParameter(0)*wsig*sqrt(2.*PI);
         }
         }

         //OK for all
         }

         if(w2chmax>0&&dchmax>0){
         dmax=0.;
         for (int ch=0;ch<48;ch++){
             int slot=GetYSlot(ch);
             int dch=GetYChan(ch);
             adcval=dADCmax[ch][0];
             //adcval=dADCsum[ch];
             if(adcval>dmax){
                dmax=adcval;
                dchmax=ch;
                dsmax=uSAMPmax[ch][0];
                dped=ADCPedestal[0][slot][dch];
                //cok dwid=dwd[ch];
             }
         }

         w2max=0.;
         for (int ch=0;ch<240;ch++){
             int slot=GetXSlot(ch);
             int dch=GetXChan(ch);
             adcval=w2ADCmax[ch][0];
             //adcval=wADCsum[ch];
             if(adcval>w2max){
                w2max=adcval;
                w2chmax=ch;
                w2smax=w2SAMPmax[ch][0];
                w2ped=ADCPedestal[0][slot][dch];
                //cok w2wid=w2wd[ch];
             }
         }

         dcharge=0.;
         dmcharge=0.;
         dcent=0.;
         dfit=0.;
         dmcent=0.;
         dmfit=0.;

         int ch1=dchmax-3;
         if(ch1<0)ch1=0;
         int ch2=dchmax+3;
         if(ch2>47)ch2=47;

         dsig=0.;
         dsize=0;
         FHist->Reset();
         for (int ich=ch1;ich<ch2+1;ich++){
            adcval=dADCmax[ich][0];
            if(adcval>thresh){
               dmcharge+=adcval;
               dmcent+=(float)adcval*ich;
               FHist->Fill((double)ich-ch1,(double)adcval);
               dsize++;
            }
         }
         dmcent/=dmcharge;
         centroid->SetParameter(0,(double)dADCmax[dchmax][0]);
         centroid->SetParameter(1,(double)2.);
         centroid->SetParameter(2,(double)1.);
         if(dsize>2){
         TFitResultPtr res=FHist->Fit("centroid","QNS");
         int fstatus=res;
         if(fstatus==0){
           dmfit=(float)centroid->GetParameter(1)+(float)ch1;
           dsig=abs((float)centroid->GetParameter(2));
        //   dmcharge=centroid->GetParameter(0)*dsig*sqrt(2.*PI);
         }
         }

         dsize=0;
         FHist->Reset();
         for (int ich=ch1;ich<ch2+1;ich++){
            adcval=dADCsum[ich];
            if(adcval>thresh){
               dcharge+=adcval;
               dcent+=(float)adcval*ich;
               FHist->Fill((double)ich-ch1,(double)adcval);
               dsize++;
            }
         }
         dcent/=dcharge;
         centroid->SetParameter(0,(double)dADCsum[dchmax]);
         centroid->SetParameter(1,(double)2.);
         centroid->SetParameter(2,(double)1.);
         if(dsize>2){
         TFitResultPtr res=FHist->Fit("centroid","QNS");
         int fstatus=res;
         if(fstatus==0){
           dfit=(float)centroid->GetParameter(1)+(float)ch1;
         //  usig=abs((float)centroid->GetParameter(2));
         //  ucharge=centroid->GetParameter(0)*usig*sqrt(2.*PI);
         }
         }
         

         w2charge=0.;
         w2mcharge=0.;
         w2cent=0.;
         w2fit=0.;
         w2mcent=0.;
         w2mfit=0.;
         w2sig=0.;
         w2size=0;

         ch1=w2chmax-3;
         if(ch1<0)ch1=0;
         ch2=w2chmax+3;
         if(ch2>239)ch2=239;


         w2sig=0.;
         w2size=0;
         FHist->Reset();
         for (int ich=ch1;ich<ch2+1;ich++){
            adcval=w2ADCmax[ich][0];
            if(adcval>thresh){
               w2mcharge+=adcval;
               w2mcent+=(float)adcval*ich;
               FHist->Fill((double)ich-ch1,(double)adcval);
               w2size++;
            }
         }
         w2mcent/=w2mcharge;
         centroid->SetParameter(0,(double)w2ADCmax[w2chmax][0]);
         centroid->SetParameter(1,(double)2.);
         centroid->SetParameter(2,(double)1.);
         if(w2size>2){
         TFitResultPtr res=FHist->Fit("centroid","QNS");
         int fstatus=res;
         if(fstatus==0){
           w2mfit=(float)centroid->GetParameter(1)+(float)ch1;
           //cok w2sig=abs((float)centroid->GetParameter(2));
        //   w2mcharge=centroid->GetParameter(0)*w2sig*sqrt(2.*PI);
         }
         }

         w2size=0;
         FHist->Reset();
         for (int ich=ch1;ich<ch2+1;ich++){
            adcval=w2ADCsumall[ich];
            if(adcval>thresh){
               w2charge+=adcval;
               w2cent+=(float)adcval*ich;
               FHist->Fill((double)ich-ch1,(double)adcval);
               w2size++;
            }
         }
         w2cent/=w2charge;
         centroid->SetParameter(0,(double)w2ADCsumall[w2chmax]);
         centroid->SetParameter(1,(double)2.);
         centroid->SetParameter(2,(double)1.);
         if(w2size>2){
         TFitResultPtr res=FHist->Fit("centroid","QNS");
         int fstatus=res;
         if(fstatus==0){
           wfit=centroid->GetParameter(1)+ch1;
        //   w2sig=abs(centroid->GetParameter(2));
        //   w2charge=centroid->GetParameter(0)*w2sig*sqrt(2.*PI);
         }
         }

         //OK for all
         } //end w2chmax>0 dchmax>0

         wsig=0.;
         for (int ch=0;ch<24;ch++){
          if(abs(ch-wchmax)<2){
            wsig+=wNhit[ch];
          }
         }

         unhit=0;
         wnhit=0;
         wsize=0.;
         for (int ihit=0;ihit<200;ihit++){
           wthit[ihit]=-100.;
           wahit[ihit]=-100.;
           wmhit[ihit]=-100.;
           uthit[ihit]=-100.;
           uahit[ihit]=-100.;
         }
         float tmax12=tmax1;
         if(tmax2>tmax1)tmax12=tmax2;
         tmax12=550.;
         if(wchmax>0&&uchmax>0){
             if(wmcharge>200.){
                  unhit=uNhit[uchmax];
                  for(int ihit=0;ihit<uNhit[uchmax];ihit++){
                     uthit[ihit]=uSAMPmax[uchmax][ihit];
                     uahit[ihit]=uADCmax[uchmax][ihit];
                     uAvsT->Fill((double)uSAMPmax[uchmax][ihit],(double)uADCmax[uchmax][ihit]); 
                  }

                  /* short mode
                  for (int ch=0;ch<24;ch++){
                    int slot; int ch0;
                    slot=sl_c1w1; ch0=ch_c1w1;
                  if(abs(ch-wchmax)<2){
                  if(ch==wchmax)wsize=fADCnhit[0][slot][ch+ch0];
                  for(int ihit=0;ihit<fADCnhit[0][slot][ch+ch0];ihit++){
                     float adc_peak=fADCpeak[0][slot][ch+ch0][ihit]*8.-fADCPedestal[0][slot][ch+ch0];
                     if(adc_peak>thresh){
                     wthit[wnhit]=fADCtime[0][slot][ch+ch0][ihit];
                     wmhit[wnhit++]=adc_peak;
                     }
                  }
                  }
                  }
                  */
                  
                  for (int iclust=0;iclust<wnclust;iclust++){
                     wAvsTnorm->Fill((double)wclustsmp[iclust],(double)wclustchn[iclust],(double)wclustamp[iclust]); 
                     //wAvsT->Fill((double)wclustsmp[iclust],(double)wclustchn[iclust],(double)wclustamp[iclust]); 
                     //wCvsT->Fill((double)wclustsmp[iclust],(double)wclustchn[iclust],(double)wclusttwd[iclust]); 
                     wCvsL->Fill((double)wclustsmp[iclust],(double)wclustchn[iclust],(double)wclustlwd[iclust]); 
                  }
                  for (int ch=0;ch<24;ch++){
                  for(int ihit=0;ihit<wNhit[ch]&&ihit<200;ihit++){
                  if(abs(ch-wchmax)<2){
                     //cok raw wthit[wnhit]=wSAMPmax[ch][ihit];
                     //cok raw wahit[wnhit]=wADCsum[ch][ihit];
                     //cok raw wmhit[wnhit++]=wADCmax[ch][ihit];
                     if(wnhit<999){
                     wthit[wnhit]=wSAMPmax[ch][ihit];
                     wahit[wnhit]=wADCsum[ch][ihit];
                     wmhit[wnhit++]=wADCmax[ch][ihit];
                     }
                     wAvsD->Fill((double)(wSAMPmax[ch][ihit]-48.)*dmax1/tmax1,(double)ch,(double)wADCsum[ch][ihit]); 
                     wAvsT->Fill((double)wSAMPmax[ch][ihit],(double)ch,(double)wADCsum[ch][ihit]); 
                     wCvsT->Fill((double)wSAMPmax[ch][ihit],(double)ch); 
                     } else {
                     wAvsB->Fill((double)wSAMPmax[ch][ihit],(double)ch,(double)wADCsum[ch][ihit]); 
                     wCvsB->Fill((double)wSAMPmax[ch][ihit],(double)ch); 
                  int fillok=1;
                  for (int chu=0;chu<24&&fillok;chu++){
                  for(int ihitu=0;ihitu<uNhit[chu]&&fillok;ihitu++){
                     if(abs(uSAMPmax[chu][ihitu]-wSAMPmax[ch][ihit])<3){
                       float y=-0.5*(chu-24.)/cos(sang)+1.*ch*tan(sang);
                  if(wSAMPmax[ch][ihit]>tmax12+50.){
                       wYvsB->Fill((double)ch,(double)y,(double)wADCmax[chu][ihitu]); 
                  }
                       //cock wTYvsB->Fill((double)wSAMPmax[ch][ihit],(double)y,(double)wADCsum[ch][ihit]); 
                       wTYvsB->Fill((double)wSAMPmax[ch][ihit],(double)y,(double)uADCmax[chu][ihitu]); 
                     }
                  }
                  }
                     
                     }
                  }
                  }
                       print_flg=0;
                    


            }
         }

         w2sig=0.;
         for (int ch=0;ch<72;ch++){
          if(abs(ch-wchmax)<10){
            w2sig+=w2Nhit[ch];
          }
         }

         dnhit=0;
         w2nhit=0;
         w2size=0.;
         for (int ihit=0;ihit<200;ihit++){
           w2thit[ihit]=-100.;
           w2ahit[ihit]=-100.;
           w2mhit[ihit]=-100.;
           dthit[ihit]=-100.;
           dahit[ihit]=-100.;
         }
         tmax12=tmax1;
         if(tmax2>tmax1)tmax12=tmax2;
         tmax12=550.;
         if(w2chmax>0&&abs(wcent-w2chmax/25-4.15)<20){
             if(w2mcharge>200.){
                  dnhit=dNhit[uchmax];
                  for(int ihit=0;ihit<dNhit[dchmax];ihit++){
                     dthit[ihit]=dSAMPmax[dchmax][ihit];
                     dahit[ihit]=dADCmax[dchmax][ihit];
                     dAvsT->Fill((double)dSAMPmax[dchmax][ihit],(double)dADCmax[dchmax][ihit]); 
                  }

                  /* short mode
                  for (int ch=0;ch<24;ch++){
                    int slot; int ch0;
                    slot=sl_c1w1; ch0=ch_c1w1;
                  if(abs(ch-wchmax)<2){
                  if(ch==wchmax)wsize=fADCnhit[0][slot][ch+ch0];
                  for(int ihit=0;ihit<fADCnhit[0][slot][ch+ch0];ihit++){
                     float adc_peak=fADCpeak[0][slot][ch+ch0][ihit]*8.-fADCPedestal[0][slot][ch+ch0];
                     if(adc_peak>thresh){
                     wthit[wnhit]=fADCtime[0][slot][ch+ch0][ihit];
                     wmhit[wnhit++]=adc_peak;
                     }
                  }
                  }
                  }
                  */
                  
                  for (int iclust=0;iclust<w2nclust;iclust++){
                     w2AvsTnorm->Fill((double)w2clustsmp[iclust],(double)w2clustchn[iclust],(double)w2clustamp[iclust]); 
                     //w2AvsT->Fill((double)w2clustsmp[iclust],(double)w2clustchn[iclust],(double)w2clustamp[iclust]); 
                     //w2CvsT->Fill((double)w2clustsmp[iclust],(double)w2clustchn[iclust],(double)w2clusttwd[iclust]); 
                     w2CvsL->Fill((double)w2clustsmp[iclust],(double)w2clustchn[iclust],(double)w2clustlwd[iclust]); 
                  }
                  for (int ch=0;ch<240;ch++){
                  for(int ihit=0;ihit<w2Nhit[ch]&&ihit<200;ihit++){
                  //cold if(abs(ch-w2chmax)<10&&abs(w2charge/dcharge-0.8)<0.4&&dcharge>100000){
                  if(abs(ch-w2chmax)<10){
                     //cok raw w2thit[w2nhit]=w2SAMPmax[ch][ihit];
                     //cok raw w2ahit[w2nhit]=w2ADCsum[ch][ihit];
                     //cok raw w2mhit[w2nhit++]=w2ADCmax[ch][ihit];
                     if(w2nhit<999){
                     w2thit[w2nhit]=w2SAMPmax[ch][ihit];
                     w2ahit[w2nhit]=w2ADCsum[ch][ihit];
                     w2mhit[w2nhit++]=w2ADCmax[ch][ihit];
                     }
                     w2AvsD->Fill((double)(w2SAMPmax[ch][ihit]-48.)*dmax1/tmax1,(double)ch,(double)w2ADCsum[ch][ihit]); 
                     w2AvsT->Fill((double)w2SAMPmax[ch][ihit],(double)ch,(double)w2ADCsum[ch][ihit]); 
                     w2CvsT->Fill((double)w2SAMPmax[ch][ihit],(double)ch); 
                     } else {
                     w2AvsB->Fill((double)w2SAMPmax[ch][ihit],(double)ch,(double)w2ADCsum[ch][ihit]); 
                     w2CvsB->Fill((double)w2SAMPmax[ch][ihit],(double)ch); 
                  int fillok=1;
                  for (int chd=0;chd<48&&fillok;chd++){
                  for(int ihitd=0;ihitd<dNhit[chd]&&fillok;ihitd++){
                     if(abs(dSAMPmax[chd][ihitd]-w2SAMPmax[ch][ihit])<3){
                       float y=chd;
                  if(w2SAMPmax[ch][ihit]>tmax12+50.){
                       w2YvsB->Fill((double)ch,(double)y,(double)w2ADCmax[chd][ihitd]); 
                  }
                       //cock wTYvsB->Fill((double)wSAMPmax[ch][ihit],(double)y,(double)wADCsum[ch][ihit]); 
                       w2TYvsB->Fill((double)w2SAMPmax[ch][ihit],(double)y,(double)dADCmax[chd][ihitd]); 
                     }
                  }
                  }
                     
                     }
                  }
                  }
                       print_flg=0;
                    


            }
         }

 if(wnclust>0)wwid=wclusttwd[0];
 if(wnclust>0)wsig=wclustlwd[0];
 if(w2nclust>0)w2wid=w2clusttwd[0];
 if(w2nclust>0)w2sig=w2clustlwd[0];


/*
         int isamp0=35;
         int nsamp=120-isamp0;
         bool found=false;

         if(wchmax<12&&wnhit>0){

          to_sergey_rad<<endl<<" "<<nsamp<<endl;
         for(int isamp=isamp0;isamp<120;isamp++){ 
         found=false;
         for (int ihit=0;ihit<wnhit;ihit++){
            if(isamp==wthit[ihit]){
              to_sergey_rad<<wahit[ihit]<<" ";
              found=true;
              break;
            } 
         }
         if(!found)  to_sergey_rad<<0.<<" ";
         }
         to_sergey_rad<<endl;
  
         } else if (wchmax>12&&wnhit>0){

          to_sergey_norad<<endl<<" "<<nsamp<<endl;
         for(int isamp=isamp0;isamp<120;isamp++){
         found=false;
         for (int ihit=0;ihit<wnhit;ihit++){
            if(isamp==wthit[ihit]){
              to_sergey_norad<<wahit[ihit]<<" ";
              found=true;
              break;
            } 
         }
         if(!found)  to_sergey_norad<<0.<<" ";
         }
         to_sergey_norad<<endl;

         }
*/

// to sergey raw energy deposition
        
         //cout<<"wchmax, wAmax[wchmax],wSmax[wchmax]"<<wchmax<<" "<<wAmax[wchmax]<<" "<<wSmax[wchmax]<<endl;
             int slot; int ch0;
             slot=sl_c1w1; ch0=ch_c1w1;
         //if(wnhit>0&&wAmax[wchmax]>300.&&abs(ADCPedestal[0][slot][ch0+wchmax]-100.)<10.)
         if(wnhit>0&&wAmax[wchmax]>300.){
         int isamp0=40;
         int isamp1=110;
         int nsamp=isamp1-isamp0;
             float dedx[200];
             for(int i=0;i<samples;i++){
               dedx[i]=0.;
             }
             for (int ch=0;ch<24;ch++){
               if(abs(ch-wchmax)<2&&wAmax[ch]>thresh&&abs(ADCPedestal[0][slot][ch0+ch]-100.)<10.){
                 for (int i=0;i<samples;i++){
                    dedx[i]+=ADCSamples[0][slot][ch+ch0][i]-ADCPedestal[0][slot][ch+ch0];
                 }
               }
             }
             if (wchmax<10&&wchmax>6) {
               to_sergey_rad<<nsamp<<endl;
               for(int i=isamp0;i<isamp1;i++){
               to_sergey_rad<<dedx[i]<<" "; 
               //cout<<dedx[i]<<" ";
               }
               to_sergey_rad<<endl;
               //cout<<endl;
             } else if (wchmax>16&&wchmax<20) {
               to_sergey_norad<<nsamp<<endl;
               for(int i=isamp0;i<isamp1;i++){
               to_sergey_norad<<dedx[i]<<" "; 
               }
               to_sergey_norad<<endl;
             }
             }
             
// end  to sergey         

         wcent=wchmax+(wchmax-w2chmax/25-4.15>0)*(0.2-sqrt(0.04+98*(wsmax-53)))/98
                     +(wchmax-w2chmax/25-4.15<=0)*(0.2+sqrt(0.04+98*(wsmax-53)))/98;

         fdcFeTree->Fill();
      
        //cout<<" displaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaayyyyyyyyyyyyy="<<DISPLAY<<endl;
        //cout<<"ucharge/dcharge="<<ucharge/dcharge<<endl;
        //cout<<"ucharge="<<ucharge<<endl;
        //cout<<"ucent-uch="<<ucent-uchmax<<endl;
        //cout<<"usize,dsize="<<usize<<" "<<dsize<<endl;

	 
        if(DISPLAY){
          ct_plot->Reset();
          rct_plot->Reset();
          wct_plot->Reset();
          cty_plot->Reset();
          rcty_plot->Reset();
          wcty_plot->Reset();
          DISPLAY=1;
          //if(wmcharge>0)ratio=umcharge/wmcharge;
          int OKu=0;
          int OKd=0;
          int OKw2=0;
          int OKw=0;
         if(w2chmax!=11111){
          TMultiGraph *mg = new TMultiGraph("mg","f125 samples");
          TGraph *graf1[48];
          TGraph *graf2[240];


          float Xaxis[500];
          float Yuaxis[24][500];
          float Ydaxis[48][500];
          float Yw2axis[240][500];
          float Ywaxis[24][500];
          int smp1=0;
          int smp2=samples;
          smp1=0;
          smp2=300;
          int ns=smp2-smp1;
          for (Int_t i=smp1;i<smp2;i++){
            Xaxis[i-smp1] = i;
          }


          int okdisp=0;
          

          int ucol=6;
          int wcol=1;
          int uchlast=-1;
          int wchlast=-1;
          int uslast=-1;
          int wslast=-1;

          for (Int_t i=smp1;i<smp2;i++){
// w1
             slot=sl_c1w1; ch0=ch_c1w1;
             for (Int_t ch=0;ch<24;ch++){ 
              if(wAmax[ch]>thresh){
                Ywaxis[ch][i-smp1] = ADCSamples[0][slot][ch+ch0][i]-ADCPedestal[0][slot][ch+ch0];
		if(Ywaxis[ch][i-smp1]>thresh/3.) { 
		  wct_plot->Fill((float)(ch+wchoffset)*10.,(float)(i-smp1)/10.,Ywaxis[ch][i-smp1]);
		  //-- SF --
		  double phi = -30.1/180.*3.1415;
		  double rdist = 50.;
		  double xshift = 10.;
		  double zo = (float)(i-smp1)/10.-rdist;
		  double xo = (float)(ch+wchoffset)*10.+xshift;
		  double zn = zo*cos(phi)-xo*sin(phi);
		  double xn = zo*sin(phi)+xo*cos(phi);
		  xcom_plot->Fill(zn,xn, Ywaxis[ch][i-smp1] );
		}
                  OKw=1;
               }
             } //end ch loop
// u
             slot=sl_c1u1; ch0=ch_c1u1;
             for (Int_t ch=0;ch<24;ch++){ 
              if(uAmax[ch]>thresh/4.){
                Yuaxis[ch][i-smp1] = ADCSamples[0][slot][ch+ch0][i]-ADCPedestal[0][slot][ch+ch0];
                  if(Yuaxis[ch][i-smp1]>thresh/12.)wcty_plot->Fill((float)(ch+uchoffset)*5.,(float)(i-smp1)/10.,Yuaxis[ch][i-smp1]);
                  OKu=1;
               }
             } //end ch loop
// d
             for (Int_t ch=0;ch<48;ch++){ 
              if(dAmax[ch]>thresh){
                  int slot=GetYSlot(ch);
                  int k=GetYChan(ch);
                Ydaxis[ch][i-smp1] = ADCSamples[0][slot][k][i]-ADCPedestal[0][slot][k];
		if(Ydaxis[ch][i-smp1]>thresh/3.) { 
		  cty_plot->Fill((float)(ch+dchoffset)/2.5,(float)(i-smp1)/10.,Ydaxis[ch][i-smp1]);
		  rcty_plot->Fill((float)(i-smp1)/10.,(float)(ch+dchoffset)/2.5,Ydaxis[ch][i-smp1]);
		}
                  OKd=1;
               }
             } //end ch loop
// w2
             for (Int_t ch=0;ch<240;ch++){ 
                if(w2Amax[ch]>thresh){
                  int slot=GetXSlot(ch);
                  int k=GetXChan(ch);
                  Yw2axis[ch][i-smp1] = ADCSamples[0][slot][k][i]-ADCPedestal[0][slot][k];
                  if(Yw2axis[ch][i-smp1]>thresh/3.) { 
		    ct_plot->Fill((float)(ch+w2choffset)/2.5,(float)(i-smp1)/10.,Yw2axis[ch][i-smp1]);
		    rct_plot->Fill((float)(i-smp1)/10.,(float)(ch+w2choffset)/2.5,Yw2axis[ch][i-smp1]);
		    xcom_plot->Fill((float)(i-smp1)/10.,(float)(ch+w2choffset)/2.5,Yw2axis[ch][i-smp1]);
		  }
                  OKw2=1;
                } 
             } //end ch loop

           } //end sample loop

              if(OKu){
              int k=0;
              for (int k=0;k<48;k++){
              if(dAmax[k]>thresh){
              //cout<<" cathodes ch,dAmax,dSmax, ucol="<<k<<" "<<dAmax[k]<<" "<<dSmax[k]<<" "<<ucol<<endl;
              
              graf1[k] = new TGraph(ns,Xaxis,Yuaxis[k]);
              graf1[k]->SetMarkerStyle(1);
              graf1[k]->SetLineStyle(1);
              graf1[k]->SetMarkerColor(ucol);
              graf1[k]->SetLineColor(ucol);
              mg->Add(graf1[k]);
              ucol++;
                OKu=1;
              } 
              } 
              } 

              if(OKw2){
              int k=0;
              for (int k=0;k<240;k++){
              if(w2Amax[k]>thresh){
              
              graf2[k] = new TGraph(ns,Xaxis,Yw2axis[k]);
              graf2[k]->SetMarkerStyle(1);
              graf2[k]->SetLineStyle(1);
              graf2[k]->SetMarkerColor(wcol);
              graf2[k]->SetLineColor(wcol);
              mg->Add(graf2[k]);
              wcol++;
              } 
              } 
              } 


      //----------  SF fit ---------------------------
      /*
h1f->Fit("gaus")
TF1 * f = h1f->GetFunction("gaus")
f->GetNDF()
f->GetChisquare()    
f->GetProb()
    Int_t bin = h3->GetBin(binx,biny,binz);
    Float_t y = h3->GetBinContent(bin);
virtual Double_t TH2::GetBinContent 	( 	Int_t  	binx,		Int_t  	biny 	) 	
     */

      TF1 fx("fx","pol1",3,17);
      TF1 fy("fy","pol1",3,17);
 
      TCutG *cutgx = new TCutG("cutgx",5);
      cutgx->SetPoint(0,  5,-80);      cutgx->SetPoint(1, 15,-80);      cutgx->SetPoint(2, 15, 80);      cutgx->SetPoint(3,  5, 80);      cutgx->SetPoint(4,  5,-80);
      TCutG *cutgy = new TCutG("cutgy",5);
      cutgy->SetPoint(0,  5,-80);      cutgy->SetPoint(1, 15,-80);      cutgy->SetPoint(2, 15, 80);      cutgy->SetPoint(3,  5, 80);      cutgy->SetPoint(4,  5,-80);

      TProfile *profx = rct_plot->ProfileX("profx", 5, 500,"[cutgx]");
      profx->Fit("fx","NR");
      Double_t chi2x = fx.GetChisquare();
      Double_t Ndfx = fx.GetNDF();
      Double_t p0x = fx.GetParameter(0);
      Double_t p1x = fx.GetParameter(1);

      TProfile *profy = rcty_plot->ProfileX("profy", 5, 500,"[cutgy]");
      profy->Fit("fy","NR");
      Double_t chi2y = fy.GetChisquare();
      Double_t Ndfy = fy.GetNDF();
      Double_t p0y = fy.GetParameter(0);
      Double_t p1y = fy.GetParameter(1);


      chi2xy->Fill(chi2x/Ndfx,chi2y/Ndfy);

      double fx0 = fx.Eval(0.);
      TAxis *xaxis = wct_plot->GetXaxis();
      Int_t binx = xaxis->FindBin(fx0);
      TH1 *h1py = wct_plot->ProjectionY("h1py",binx-1,binx+1);
      
      double fx7 = fx.Eval(7.);
      xaxis = ct_plot->GetXaxis();
      binx = xaxis->FindBin(fx7);
      TH1 *h1pyg = ct_plot->ProjectionY("h1pyg");
      TH1 *h1pyyg = cty_plot->ProjectionY("h1pyyg");
      
     
      int kfit = 0;
      //if (chi2x/Ndfx<100 && chi2y/Ndfy<10 && Ndfx>10 && Ndfy>10) { 
      if (chi2x/Ndfx<100  && Ndfx>10) { 
	kfit=1;
	hp0x->Fill(p0x);
	hp1x->Fill(p1x);
	hp0y->Fill(p0y);
	hp1y->Fill(p1y);

        sct_plot->Add(ct_plot);
        scty_plot->Add(cty_plot);
        swct_plot->Add(wct_plot);
        swcty_plot->Add(wcty_plot);

	//----	CLUST WC ----
	double THR = 350.;
	int nc=0, tot=0, fc=0;
	//TAxis *xaxis = ->GetXaxis();
	int ib1 = h1py->FindBin(8.);
	int ib2 = h1py->FindBin(30.);

	for (int ib=ib1; ib<ib2; ib++) {
	  double bc = h1py->GetBinContent(ib);
	  if (bc>=THR) {
	    tot++;
	    if (fc==0) { fc=1; nc++; }	    
	  } else {
	    fc=0;
	  }
	  //printf("----> ib=%d %d %d %f nc=%d\n",ib,ib1,ib2,bc,nc);
	}


	//-----  CLUST GEM ---
	double THRg = 500., cag=0.;
	int ncg=0, totg=0, fcg=0;
	//TAxis *xaxis = ->GetXaxis();
	ib1 = h1pyg->FindBin(7.);
	ib2 = h1pyg->FindBin(17.);

	for (int ib=ib1; ib<ib2; ib++) {
	  double bc = h1pyg->GetBinContent(ib);
	  if (bc>=THRg) {
	    totg++;
	    if (fcg==0) { fcg=1; ncg++; }
	    if (fcg==1 && bc>cag) cag=bc;   
	  } else {
	    if ( fcg==1)  { 
	      if ( -35. < fx0  && fx0 < -20.) caradg->Fill(cag); 
	      if (   5. < fx0  && fx0 <  20.) canog->Fill(cag); 
	      cag=0; 
	    }
	    fcg=0; 
	  }
	  //printf("----> ib=%d %d %d %f ncg=%d\n",ib,ib1,ib2,bcg,ncg);

	}
	if (tot>45) tot=45;
	if (totg>45) totg=45;

	//nc=tot;
	printf("nc=%d tot=%d fx0=%f \n",nc,tot,fx0);
	printf("ncg=%d totg=%d fx7=%f \n",ncg,totg,fx7);

	if (nc==0) { zwct_plot->Fill(fx0);  zwct_plot2->Fill(p0x,p0y); }

	// run 525 
	if ( -35. < fx0  && fx0 < -20.  && 6 < p0y && p0y < 14.) {  nclrad->Fill(nc); nc2totrad->Fill(tot,nc); }
	if (   5. < fx0  && fx0 <  20.  && 6 < p0y && p0y < 14.) {  nclno->Fill(nc);  nc2totno->Fill(tot,nc);  }

	if (   3. < fx7  && fx7 <  13.) {  nclradg->Fill(ncg);  }
	if ( -70. < fx7  && fx7 < -50.) {  nclnog->Fill(ncg);   }

      } //-- end if chi2
	
      //---------------------------------------------------

	    
      if(OKw && OKw2 && kfit){
	//TCanvas *myc;
	//myc = new TCanvas("myc", "Event", 50,50, 1800, 900);
	//myc->Draw();
	//myc->Clear("D");
	//myc->Divide(3,4);
  //myc->SetFillColor(11);
      //TPad *p1 = (TPad*)(myc->cd(1));
      //p1->SetLogz();
      //TPad *p2 = (TPad*)(myc->cd(2));
      //p2->SetLogz();

      myc->cd(1);
      gPad->SetGrid();
      gPad->SetLeftMargin(0.08);
      gPad->SetRightMargin(0.);
      gPad->SetTopMargin(0.);
      gPad->SetBottomMargin(0.08);
      gPad->Modified();
      //mg->SetMaximum(850.);
      //mg->SetMinimum(-50.);
      //cok mg->Draw("alp");
      wct_plot->Draw("col");
      TLine  flx(fx0, 0., fx0, 25.);
      flx.Draw("same");

      myc->cd(2);
      gPad->SetGrid();
      //gPad->SetLeftMargin(0.08);
      //gPad->SetRightMargin(0.);
      gPad->SetTopMargin(0.);
      gPad->SetBottomMargin(0.08);
      gPad->Modified();
      //wcty_plot->Draw("col");
      h1py->Draw();

      //-----------------------------------sf plot --------------
      
      myc->cd(5);
      gPad->SetGrid();
      gPad->SetLeftMargin(0.08);
      gPad->SetRightMargin(0.);
      gPad->SetTopMargin(0.);
      gPad->SetBottomMargin(0.08);
      gPad->Modified();
      //rct_plot->Draw("col");
      profx->Draw("col [cutgx]");
      fx.SetRange(0,15);
      fx.Draw("same");
      rct_plot->Draw("colsame");

      myc->cd(6);
      gPad->SetGrid();
      gPad->SetLeftMargin(0.08);
      gPad->SetRightMargin(0.);
      gPad->SetTopMargin(0.);
      gPad->SetBottomMargin(0.08);
      gPad->Modified();
      profy->Draw("col [cutgy]");
      //fy.SetRange(5,30);
      fy.Draw("same");
      rcty_plot->Draw("colsame");

      myc->cd(3);
      chi2xy->Draw("box");

      myc->cd(9);      hp0x->Draw();


      //-------------------
      //   WC clust draw 
      //-------------------
      myc->cd(7);     // hp1x->Draw(); 
      
      TH1 *h1, *h0; 
      
      h0 = nclno; h1 = nclrad;  
      h0->SetLineWidth(2);
      h1->SetLineWidth(2);
      h0->SetLineColor(4);  //-- rad 
      h1->SetLineColor(2);  //-- no rad
      {
  	double mh0 = h0->GetMaximum();
	double mh1 = h1->GetMaximum();
	if (mh1>mh0) {
	  h1->Draw(); 
	  h0->Draw("sames");    
	} else {
	  h0->Draw(); 
	  h1->Draw("sames");    
	}
	gPad->Update();  
	TPaveStats *ps1 = (TPaveStats*)h1->GetListOfFunctions()->FindObject("stats");
	ps1->SetY1NDC(0.57);  ps1->SetY2NDC(0.75); ps1->SetTextColor(kRed);
	TPaveStats *ps0 = (TPaveStats*)h0->GetListOfFunctions()->FindObject("stats");
	ps0->SetTextColor(kBlue);
	gPad->Modified(); gPad->Update(); 
      }
      //---------------

      myc->cd(10);  h1pyyg->SetLineColor(2); h1pyyg->Draw();   h1pyg->Draw("same"); // hp0y->Draw();
      myc->cd(13);  sct_plot->Draw("col");
      myc->cd(14);  caradg->SetLineColor(2) ; caradg->Draw();   canog->Draw("same"); // scty_plot->Draw("col");
      myc->cd(15);  swct_plot->Draw("col");

      myc->cd(4);  zwct_plot->Draw();
      myc->cd(8);  zwct_plot2->Draw("box");

      //-------------------
      //   GEM clust draw 
      //-------------------
      myc->cd(11); 
      h0 = nclnog; h1 = nclradg;  
      h0->SetLineWidth(2);
      h1->SetLineWidth(2);
      h0->SetLineColor(4);  //-- rad 
      h1->SetLineColor(2);  //-- no rad

      {
	double mh0 = h0->GetMaximum();
	double mh1 = h1->GetMaximum();
	if (mh1>mh0) {
	  h1->Draw(); 
	  h0->Draw("sames");    
	} else {
	  h0->Draw(); 
	  h1->Draw("sames");    
	}
	gPad->Update();  
	TPaveStats *ps1 = (TPaveStats*)h1->GetListOfFunctions()->FindObject("stats");
	ps1->SetY1NDC(0.57);  ps1->SetY2NDC(0.75); ps1->SetTextColor(kRed);
	TPaveStats *ps0 = (TPaveStats*)h0->GetListOfFunctions()->FindObject("stats");
	ps0->SetTextColor(kBlue);
	gPad->Modified(); gPad->Update(); 
      }
      //---------------
      

      myc->Modified();
      myc->Update();

      //theApp.Run();
 Int_t nexti;
 //cin>>nexti;
 nexti=1;  //sleep(1);
 if(nexti==3) myc->Print("myc.pdf");
 if(nexti==3) myc->Print("myc.root");
      }
//
       	
        } // end display condition
	} // end display    
        // //end samples>0
 
        } //end NEVENT
    
          } catch (evioException e) {
            cerr << "Event: "<<evtCount<<"  exception at eventTree()"<<endl;
            cerr << endl << e.toString() << endl << endl << "Try to continue" <<endl;
          }

          ReadBack = EvOchan.read();

    } //end of if event=< loop 
    } //end of event loop 
      
    to_sergey_rad.close();
    to_sergey_norad.close();

    ROOTfile->cd();
    uAvsT->Write();
    dAvsT->Write();
    wAvsT->Write();
    ROOTfile->Write();
    ROOTfile->Close();
    delete ROOTfile;
    cout<<" closing root file "<<root_fname<<endl;
  } //OK
  
  TGraphErrors *graph;
  char t1[128];
  char t2[128];
  sprintf(t1,"graph%d_%d_%d",ROC,SLOT,CHANNEL);
  sprintf(t2,"Pedestals ROC %d SLOT %d CHANNEL %d",ROC,SLOT,CHANNEL);
  graph = new TGraphErrors(NCon,x,y,NULL,dy);
  graph->SetTitle(t2);
  graph->SetMarkerColor(4);
  graph->SetMarkerStyle(21);
  graph->GetYaxis()->SetTitle("Pedestal [ADC counts]"); 

  
  if (DISPLAY>10){
    
    TCanvas *myc1;
    char str1[128];
    sprintf(str1,"Pedestals ROC %d ADC125 Samples in SLOT %d CHANNEL %d",ROC,SLOT,CHANNEL);
    myc1 = new TCanvas("myc1", str1, 800, 400);
    myc1->SetFillColor(42);
    gPad->SetGrid();
    graph->Draw("AP");
    myc1->Update();

    cout<<"Continue: ";
    char inp[128];
    cin>>inp;

  }

/*
  TFile *fout;
  sprintf(fnam,"%s/PedestalsROC%dSLOT%dCHANNEL%d.root",DataDir,ROC,SLOT,CHANNEL);
  fout = new TFile(fnam,"RECREATE");
  graph->Write();

  fout->Close();
*/
  myc->Print("myc.pdf");
  //theApp.Run();
  return 0;
  
} //end main



void analyzeEvent(evioDOMTree &eventTree) {

  gemclust.clear();

  //cout << endl << endl << endl << "analyzing event " << evtCount << endl << endl;
  
  // get list of all banks in event

  evioDOMNodeListP bankList2 = eventTree.getNodeList(typeIs<uint64_t>());
  evioDOMNodeList::iterator iter = bankList2->begin();

 const uint64_t *run_number_and_type = NULL;

  for(; iter!=bankList2->end(); iter++){
    evioDOMNodeP bankPtr = *iter;
    evioDOMNodeP physics_event_built_trigger_bank = bankPtr->getParent();
    if(physics_event_built_trigger_bank == NULL) continue;
    uint32_t tag = physics_event_built_trigger_bank->tag;
    cout << "++> tag= 0x" << std::hex << tag << std::dec << endl;
    const vector<uint64_t> *vec;
    switch(tag){
    case 0xFF22:
    case 0xFF23:
    case 0xFF26:
    case 0xFF27:
      vec = bankPtr->getVector<uint64_t>();
      if(!vec) continue;
      if(vec->size()<1) continue;
      run_number_and_type = &((*vec)[vec->size()-1]);
      break;
    }
    if(run_number_and_type != NULL){
      TheRunNumber =  ((*vec)[vec->size()-1])>>32;
      EVENTNUMBER = ((*vec)[0]);
      int MEVENTS = vec->size()-2;
      NEventsInBlock = MEVENTS ;
      TheEventNum = EVENTNUMBER;
      EVTCounts += MEVENTS;
   }
  }



  evioDOMNodeListP bankList = eventTree.getNodeList(typeIs<uint32_t>());
  evioDOMNodeList::iterator iterX = bankList->begin();

  for(; iterX!=bankList->end(); iterX++){
    evioDOMNodeP bankPtr = *iterX;
    evioDOMNodeP physics_event_built_trigger_bank = bankPtr->getParent();
    if(physics_event_built_trigger_bank == NULL) continue;

    if (DEBUG>0) cout << "==> tag= 0x" << std::hex << bankPtr->tag << std::dec << endl;

    if (bankPtr->tag==1){
      const vector<uint32_t> *vec;
      vec = bankPtr->getVector<uint32_t>();
      int NEV = vec->size()/4;
      if (NEventsInBlock != NEV){
        cout<<"Error Event Number Miss Match In Block "<<endl;
        cout<<"Expected: "<<NEventsInBlock<<"    found: "<<NEV<<endl;
      }
      for (int k=0; k<NEV; k++){
        uint64_t hi_t = (*vec)[k*4];
        uint64_t lo_t = (*vec)[k*4+1];
        TRIGGERTIMES = hi_t + (lo_t<<32);
        TRIGGER_MASK_GT = (*vec)[k*4+2];
        TRIGGER_MASK_FP = (*vec)[k*4+3];

        //cout<<" count triggers "<<endl;
        CountTriggers();

      }
    }
  }

    
  
  // analyze each bank
  for_each(bankList->begin(),bankList->end(),analyzeBank);
}

void CountTriggers(){



  int bit1[100];
  int NBits1 = 0;

  int bit1_S[100];
  int NBits1_S = 0;
  if (TRIGGER_MASK_GT>0){
    unsigned int val = TRIGGER_MASK_GT;
    for (int nn=0; nn<32; nn++){
      bit1[nn] = -1;
      if (val & (1<<nn)){
        //cout<<" trigger bit = "<<nn<<endl;
        bit1[NBits1] = nn;
        NBits1++;
      }
      bit1_S[nn] = -1;
      if (val == (uint32_t) (1<<nn) ){
        bit1_S[NBits1_S] = nn;
        NBits1_S++;
      }
    }
  }
  int bit2[100];
  int NBits2 = 0;
  if (TRIGGER_MASK_FP>0){
    unsigned int val = TRIGGER_MASK_FP;
    for (int nn=0; nn<32; nn++){
      bit2[nn] = -1;
      if (val & (1<<nn)){
        bit2[NBits2] = nn;
        NBits2++;
      }
    }
  }

  if (NBits1){ // GTP triggers
  }
  if (NBits1_S){ // SINGLE GTP Trigger
  }

  if (NBits2){ // FP TRIGGER
  }




}


void analyzeBank(evioDOMNodeP bankPtr) {
  
  
  // dump bank info for all banks
  //   cout << hex << left << "bank content type:  0x" << setw(6) << bankPtr->getContentType() 
  //        << "   tag:  0x" << setw(6) << bankPtr->tag 
  //        << "   num:  0x" << setw(6) << (int)bankPtr->num << dec << endl;


  evioDOMNodeP data_bank = bankPtr->getParent();

  cout << "--> tag= 0x" << std::hex << bankPtr->tag << endl;

  if( data_bank==NULL ) {
    if(VERBOSE>9) 
      cout << "     bank has no parent. skipping ... " << endl;
    return;
  }
  evioDOMNodeP physics_event_bank = data_bank->getParent();
  if( physics_event_bank==NULL ){
    if(VERBOSE>9) cout << "     bank has no grandparent. skipping ... " << endl;
    return;
  }
  if( physics_event_bank->getParent() != NULL ){
    if(VERBOSE>9) cout << "     bank DOES have great-grandparent. skipping ... " << endl;
    return; // physics event bank should have no parent!
  }
  if(VERBOSE>9){
    cout << "     Physics Event Bank: tag="  << physics_event_bank->tag << " num=" << (int)physics_event_bank->num << dec << endl;
    cout << "     Data Bank:          tag="  << data_bank->tag << " num=" << (int)data_bank->num << dec << endl;
  }
  if((data_bank->tag & 0xFF00) == 0xFF00){
    if(VERBOSE>9) cout << "     Data Bank tag is in reserved CODA range. This bank is not ROC data. Skipping ..." << endl;
    return;
  }
  
  if(VERBOSE>9) cout << "     bank lineage check OK. Continuing with parsing ... " << endl;

  // Get data from bank in the form of a vector of uint32_t
  const vector<uint32_t> *vec = bankPtr->getVector<uint32_t>();
  //const uint32_t *iptr = &(*vec)[0];
  //const uint32_t *iend = &(*vec)[vec->size()];
  //cok if(VERBOSE>6) cout << "     uint32_t bank has " << vec->size() << " words" << endl;
  //check cout << "     uint32_t bank has " << vec->size() << " words"<<" bank_tag="<<data_bank->tag << endl;
  
  int HitCnt = 0;

  cout << "**> getParent= " << data_bank->tag << std::dec << endl;

  if (data_bank->tag == 65313) {  //--- 65313= 0xFF21 

    cout<<"65313 New event"<<endl;


    //} else if ( data_bank->tag == 76 ) { // rocTRD1  == 0x4C 
  } else if ( data_bank->tag == 76 && bankPtr->tag==16 ) { // rocTRD1  == 0x4C 
    //} else if ( data_bank->tag == 53) { // rocfdc

    int Sz;
    Sz = vec->size();

    cout << "...> data size = " << Sz <<endl;

    if (Sz>0){

      int OLDSLOT;
      int SLOTNUM, CHANNEL,WSize,DATAReady;
      int slotidx,idx;
      char Crate[128];
      int C;
      float ped,adc1,adc2;
      int pedcnt;
      int MaxSlot;
      char Detector[128];
      int DetectorID = 0;
      int ROCID=0;

      float TIME;
      int NPEAK;
      float CHARGE;
      float PEDESTAL;
      float PEAK;
      ROCSlots[ROCID] = 0;


      DATAReady = 0;
      int NPK_count = 0;
      int NPK = 0;
      OLDSLOT = 0;
      SLOTNUM = 0;
      slotidx = 0;
      WSize=0;

        for(int slot=0;slot<15;slot++){
          for(int ch=0;ch<72;ch++){
            fADCnhit[ROCID][slot][ch]=0;
            for(int hit=0;hit<200;hit++){
               fADCpeak[ROCID][slot][ch][hit]=-1000.;
               fADCcharge[ROCID][slot][ch][hit]=-1000.;
               fADCtime[ROCID][slot][ch][hit]=-1000.;
            }
              for (int sm=0;sm<2000;sm++){
	        ADCSamples[ROCID][slot][ch][sm] = -1000.;
              }
	    ADCPedestal[ROCID][slot][ch] = 0.;

	    fADCPedestal[ROCID][slot][ch] = 0.;
          }
        }


	for (int k=0; k<Sz; k++){
	  
	  unsigned int data = (*vec)[k];
	  
	  //cout<<" data="<<hex<<data<<dec<<endl;
          //cout<<" mode="<<((data & 0xf8000000) >>27)<<endl;
	  
	  if(data_bank->tag==58){
	    // cout<<" mode="<<((data & 0xf8000000) >>27)<<endl;
	    //check  printf("mode=0x%x \n",data );
	  }
	  if (((data & 0xf8000000) >>27) == 0x10) { // Block Header
	    SLOTNUM = ((data& 0x07C00000)>>22);
	    //cout<<"slot="<<SLOTNUM<<endl;
	    int evntnost = (data& 0xf);
	    //cout<<" slot, number of events in block="<<evntnost<<endl;
	    
	    if (SLOTNUM!=OLDSLOT){
	      //cout<<SLOTNUM<<"   "<<OLDSLOT<<endl;
	      OLDSLOT = SLOTNUM;  
	      ROCSlots[ROCID]++;
	      
	    }
	    
	    slotidx = SLOTNUM-3;
	    if (SLOTNUM>10){
	      slotidx -= 2; 
	    }
	    MaxSlot = slotidx;
	  } else if (((data & 0xf8000000)>>27) == 0x12) {
	    //cout<<" ST event no="<<SLOTNUM<<" "<<(data & 0x3FFFFF)<<endl; 
	    evntno_trd = (data & 0x3FFFFF);
	  } else if (((data & 0xf8000000)>>27) == 0x13) {
	    long int ta = (long int)(data>>16)&0xff;
	    long int tb = (long int)(data>>8)&0xff;
	    long int tc = (long int)data&0xff;
	    unsigned int next_data = (*vec)[k+1];
	    long int td = (long int)(next_data>>16)&0xff;
	    long int te = (long int)(next_data>>8)&0xff;
	    long int tf = (long int)next_data&0xff;
	    //cout<<" a,b,c,d,e,f="<<hex<<ta<<" "<<tb<<" "<<tc<<" "<<td<<" "<<te<<" "<<tf<<dec<<endl; 
	    long int trigtrd=(tf<<0) + (te<<8) + (td<<16) + (tc<<24) + (tb<<32) + (ta<<40);
	    //cout<<" trigtrd="<<trigtrd<<endl;
	    trig_trd[SLOTNUM-10]=(tf<<0) + (te<<8) + (td<<16) + (tc<<24) + (tb<<32) + (ta<<40);
	    trig_trd[SLOTNUM-10]=8*trig_trd[SLOTNUM-10];
	    //trig_trd[SLOTNUM-10]=8*trig_trd[SLOTNUM-10];
	    //cout<<" -----TRD ----- slot,trig_trd,delta="<<SLOTNUM<<" "<<trig_trd[SLOTNUM-10][9]<<" "<<trig_st[3][9]-trig_trd[SLOTNUM-10][9]<<endl; 
	  } else if (((data & 0xf8000000)>>27 == 0x14)) {
	    CHANNEL = ((data & 0x7F00000)>>20) ; // flash is couning channels from 1 to 72 need 0 to 71
	    WSize =  (data & 0xFFF);
	    //cout<<" channel,samples="<<CHANNEL<<" "<<WSize<<endl;
	    DATAReady = WSize/2;
	    idx = 0;
	    ped = 0.;
	    pedcnt = 0;
	    ADCPedestal[ROCID][slotidx][CHANNEL] = 0.;
	    fADCPedestal[ROCID][slotidx][CHANNEL] = 0.;
	    //check if(data_bank->tag==58)cout<<endl<<endl<<" slot,ch,wsize="<<SLOTNUM<<" "<<CHANNEL<<" "<<WSize<<endl;;
	  } else if (DATAReady>0) { // Window Raw Data values
	    DATAReady--; 
            if(data_bank->tag==58){
              //check cout<<" "<<adc1<<" "<<adc2<<" ";
            }
	  if (pedcnt<16){
	  //if (pedcnt<4)
	    adc1 =  (float)((data & 0x1FFF0000) >> 16);
	    adc2 =  (float)(data & 0x1FFF);
	    ped += adc1;
	    ped += adc2;
	     ADCSamples[ROCID][slotidx][CHANNEL][idx++] = adc1;
	     ADCSamples[ROCID][slotidx][CHANNEL][idx++] = adc2;
             //if(NPK>10)cout<<" rocid,slot,ch,idx,ADCSamples="<<ROCID<<" "<<slotidx<<" "<<CHANNEL<<" "<<idx<<" "<< ADCSamples[ROCID][slotidx][CHANNEL][idx-1]<<" "<<ADCSamples[ROCID][slotidx][CHANNEL][idx-2]<<endl;
	    pedcnt+=2;
	    if (pedcnt>15){
	    ped /= 16.;
	      ADCPedestal[ROCID][slotidx][CHANNEL] = ped;
	    }
	  } else {
	    adc1 =  (float)((data & 0x1FFF0000) >> 16);
	    adc2 =  (float)(data & 0x1FFF);
            if(adc1>4095)adc1=4095;
            if(adc2>4095)adc2=4095;
	    ADCSamples[ROCID][slotidx][CHANNEL][idx++] = adc1;
	    ADCSamples[ROCID][slotidx][CHANNEL][idx++] = adc2;
             if(NPK>10){
               print_flg=1;
               //cout<<" rocid,slot,ch,idx,ADCSamples="<<ROCID<<" "<<slotidx<<" "<<CHANNEL<<" "<<idx-2<<" "<< ADCSamples[ROCID][slotidx][CHANNEL][idx-2]<<" "<<ADCSamples[ROCID][slotidx][CHANNEL][idx-1]<<" "<<ADCPedestal[ROCID][slotidx][CHANNEL]<<endl;
            }
	    if( (adc1-ADCPedestal[ROCID][slotidx][CHANNEL])>200. || 
		(adc2-ADCPedestal[ROCID][slotidx][CHANNEL])>200.) {
                slot_special=slotidx;
                ch_special=CHANNEL;
                samp_special=idx-1;
             //cout<<" ------------------------------ rocid,slot,ch,idx,ADCSamples="<<ROCID<<" "<<slotidx<<" "<<CHANNEL<<" "<<idx<<" "<< ADCSamples[9][ROCID][slotidx][CHANNEL][idx-1]<<endl;
             //cout<<" ------------------------------ rocid,slot,ch,idx,ADCPedestal="<<ROCID<<" "<<slotidx<<" "<<CHANNEL<<" "<<idx<<" "<< ADCPedestal[9][ROCID][slotidx][CHANNEL]<<endl;
	    }
	    
	  }
	} else if (((data & 0xf8000000)>>27) == 0x19) { //pulse integral
	  CHANNEL = ((data & 0x7F00000)>>20) ; 
	  NPK = ((data & 0xF8000)>>15) ; 
          NPK_count=NPK;
       } else if (NPK_count>0) { //Peak loop
          NPK_count--;
          float peak = (float) ((data & 0x7FF80000)>>19) ;
          float time = (float) ((data & 0x7F800)>>11) ;
          float ped = (float) (data & 0x7FF) ;
          fADCpeak[ROCID][slotidx][CHANNEL][fADCnhit[ROCID][slotidx][CHANNEL]]=peak-ped;
          fADCtime[ROCID][slotidx][CHANNEL][fADCnhit[ROCID][slotidx][CHANNEL]]=time;
          //if(NPK>10) cout<<" -------------------------------- "<<fADCnhit[ROCID][slotidx][CHANNEL]<<" "<<CHANNEL<<" "<<time<<" "<<peak<<" "<<ped<<endl;
          fADCnhit[ROCID][slotidx][CHANNEL]++;
	} else if (((data & 0xf8000000)>>27) == 0x18) { // pulse time
	  CHANNEL = ((data & 0x7F00000)>>20) ; 
	  NPEAK = ((data & 0xC0000)>>18) ; 
          TIME = (float) (data & 0xFFFF) ;
          //if((ROCID==3&&slotidx==7&&CHANNEL==9))
          //check if((NPEAK>1))
           //    cout<<" roc,slot,ch="<<ROCID<<" "<<slotidx<<" "<<CHANNEL<<endl;
           //    cout<<" time, peak number="<<TIME/64<<" "<<NPEAK<<endl;
           //    cout<<" samples0="<<endl;
           //    for(int sm=0;sm<160;sm++)
           //    cout<<" "<<ADCSamples[ROCID][slotidx][CHANNEL][sm];
           //    
          //check
          fADCtime[ROCID][slotidx][CHANNEL][NPEAK]=TIME/64.;
	} else if (((data & 0xf8000000)>>27) == 0x10) { // pulse pedestal and max amplitude
	  //run<375(Beni's numbering)  CHANNEL = ((data & 0x7F00000)>>20) ; 
	  //CHANNEL = ((data & 0x7800000)>>23) ; 
	    
	  NPEAK = ((data & 0x600000)>>21) ; 

          PEAK = (float) (data & 0xFFF) ;

	  //PEDESTAL = (float) ((data & 0xFF000)>>12) ; 
	  PEDESTAL = (float) ((data & 0x1FF000)>>12) ; 
	  
	  //fADCPedestal[ROCID][slotidx][CHANNEL] =PEDESTAL;
          //fADCpeak[ROCID][slotidx][CHANNEL][0]=PEAK-PEDESTAL;
          //fADCnhit[ROCID][slotidx][CHANNEL]=1;
          //if((ROCID==3&&slotidx==7&&CHANNEL==3))
          //check if((NPEAK>1))
          //     cout<<" roc,slot,ch="<<ROCID<<" "<<slotidx<<" "<<CHANNEL<<endl;
          //     cout<<" ped, peak, npeak="<<PEDESTAL<<" "<<PEAK<<" "<<NPEAK<<endl;
          //check 
        }
      }
      WindowSize = WSize;


    } //Sz>0
 } else if ( data_bank->tag == 83) { //end rocst, begin rocps1 

    //cout<<" -------------------- rocPS111111"<<endl;
    int Sz;
    Sz = vec->size();
    if (Sz>0){

      int OLDSLOT;
      int SLOTNUM, CHANNEL,WSize,DATAReady;
      int slotidx,idx;
      char Crate[128];
      int C;
      float ped,adc1,adc2;
      int pedcnt;
      int MaxSlot;
      char Detector[128];
      int DetectorID = 0;
      int ROCID=0;

      float TIME;
      int NPEAK;
      float CHARGE;
      float PEDESTAL;
      float PEAK;
      ROCSlots[ROCID] = 0;


      DATAReady = 0;
      OLDSLOT = 0;
      SLOTNUM = 0;
      slotidx = 0;
      WSize=0;

        for(int slot=0;slot<15;slot++){
          for(int ch=0;ch<16;ch++){
            for (int sm=0;sm<2000;sm++){
	      PSADCSamples[slot][ch][sm] = -1000.;
            }
	    PSADCPedestal[slot][ch] = 0.;
          }
        }

      for (int k=0; k<Sz; k++){
	
	unsigned int data = (*vec)[k];

         //cout<<" data="<<hex<<data<<dec<<endl;
         // cout<<" mode="<<((data & 0xf8000000) >>27)<<endl;

	if (((data & 0xf8000000) >>27) == 0x10) { // Block Header
	  SLOTNUM = ((data& 0x07C00000)>>22);
          //cout<<"slot="<<SLOTNUM<<endl;
	  int evntnblock = (data& 0xf);
          //cout<<" number of events in block="<<evntblock<<endl;

	  slotidx = SLOTNUM-3;
	  if (SLOTNUM>10){
	    slotidx -= 2; 
	  }
	  MaxSlot = slotidx;
	} else if (((data & 0xf8000000)>>27) == 0x12) {
          evntno_ps=(data & 0x3FFFFF);
          //cout<<" PS event no="<<SLOTNUM<<" "<<(data & 0x3FFFFF)<<endl; 
	} else if (((data & 0xf8000000)>>27) == 0x13) {
          long int pstd = (long int)(data>>16)&0xff;
          long int pste = (long int)(data>>8)&0xff;
          long int pstf = (long int)data&0xff;
	  unsigned int next_data = (*vec)[k+1];
          long int psta = (long int)(next_data>>16)&0xff;
          long int pstb = (long int)(next_data>>8)&0xff;
          long int pstc = (long int)next_data&0xff;
          if(SLOTNUM<10){
          trig_ps[SLOTNUM]=(pstf<<0) + (pste<<8) + (pstd<<16) + (pstc<<24) + (pstb<<32) + (psta<<40);
          long int trig_delta=trig_ps[SLOTNUM]-trig_trd[3];
          //cout<<" ><><><PS><><>< slot,trig_ps,trig_delta="<<SLOTNUM<<" "<<trig_ps[SLOTNUM][9]<<" "<<trig_delta<<endl; 
          }
          //if(SLOTNUM==3){
          //TRIGtest->Fill((double)trig_delta,(double)evntno_ps);
          //TRIGtest->SetPoint(tgpoint,(double)evntno_ps,(double)trig_delta);
          //TGtrig->SetPoint(tgpoint,(double)evntno_ps,(double)trig_delta);
          //TGtrig->SetPoint(tgpoint,(double)evntno_ps,(double)trig_delta);
          //TGtrig->SetPoint(tgpoint,(double)evntno_ps,(double)evntno_ps-evntno_trd);
          //tgpoint++;
          //}
          //if(abs(trig_delta+8000)>20)cout<<" ><><><PS><><>< slot,trig_delta="<<SLOTNUM<<" "<<trig_delta<<endl; 
          //cout<<" PS slotnum="<<SLOTNUM<<endl;
          /*
          if(SLOTNUM==13){
          for (int itrd=0;itrd<10;itrd++){
          int jtrd=3;
            cout<<" "<<trig_trd[jtrd][itrd];
          }
          cout<<endl;
          for (int ips=0;ips<10;ips++){
          int jps=3;
            cout<<" "<<trig_ps[jps][ips];
          }
          cout<<endl;
          for (int ips=0;ips<10;ips++){
          int jps=3;
          cout<<endl;
          for (int itrd=0;itrd<10;itrd++){
          int jtrd=3;
            long int delta=trig_ps[jps][ips]-trig_trd[jtrd][itrd];
            cout<<" "<<delta;
          }
          }
          cout<<endl;
          }
          */
	} else if (((data & 0xf8000000)>>27) == 0x14) {
	  CHANNEL = ((data & 0x7800000)>>23) ; // flash is couning channels from 1 to 72 need 0 to 71
	  WSize =  (data & 0xFFF);
          //cout<<" channel,samples="<<CHANNEL<<" "<<WSize<<endl;
	  DATAReady = WSize/2;
	  idx = 0;
	  ped = 0.;
	  pedcnt = 0;
	  PSADCPedestal[slotidx][CHANNEL] = 0.;
             //check if(data_bank->tag==58)cout<<endl<<endl<<" slot,ch,wsize="<<SLOTNUM<<" "<<CHANNEL<<" "<<WSize<<endl;;
	} else if (DATAReady>0) { // Window Raw Data values
	  DATAReady--; 
	  if (pedcnt<16){
	  //if (pedcnt<4)
	    adc1 =  (float)((data & 0x1FFF0000) >> 16);
	    adc2 =  (float)(data & 0x1FFF);
	    ped += adc1;
	    ped += adc2;
	     PSADCSamples[slotidx][CHANNEL][idx++] = adc1;
	     PSADCSamples[slotidx][CHANNEL][idx++] = adc2;
             //cout<<" rocid,slot,ch,idx,ADCSamples="<<ROCID<<" "<<slotidx<<" "<<CHANNEL<<" "<<idx<<" "<< ADCSamples[ROCID][slotidx][CHANNEL][idx-1]<<endl;
	    pedcnt+=2;
	    if (pedcnt>15){
	    ped /= 16.;
	      PSADCPedestal[slotidx][CHANNEL] = ped;
	    }
	  } else {
	    adc1 =  (float)((data & 0x1FFF0000) >> 16);
	    adc2 =  (float)(data & 0x1FFF);
            if(adc1>4095)adc1=4095;
            if(adc2>4095)adc2=4095;
	    PSADCSamples[slotidx][CHANNEL][idx++] = adc1;
	    PSADCSamples[slotidx][CHANNEL][idx++] = adc2;
          }	    
      }//end data ready
      PSWindowSize = WSize;
    } // data loop
   } //Sz>0
 } else if ( data_bank->tag == 84) { //end rocps1, begin rocps2 

    //cout<<" -------------------- rocPS222222"<<endl;
    int Sz;
    Sz = vec->size();
    if (Sz>0){

      int SLOTNUM, CHANNEL,WSize,DATAReady;
      int idx;
      float ped,adc1,adc2;
      int pedcnt;


      DATAReady = 0;
      SLOTNUM = 0;
      WSize=0;

          for(int ch=0;ch<16;ch++){
            for (int sm=0;sm<2000;sm++){
	      PSCADCSamples[ch][sm] = -1000.;
            }
	    PSCADCPedestal[ch] = 0.;
          }

      for (int k=0; k<Sz; k++){
	
	unsigned int data = (*vec)[k];
         //cout<<" data="<<hex<<data<<dec<<endl;
         // cout<<" mode="<<((data & 0xf8000000) >>27)<<endl;

	if (((data & 0xf8000000) >>27) == 0x10) { // Block Header
	  SLOTNUM = ((data& 0x07C00000)>>22);
          //cout<<" SLOTNUM="<<SLOTNUM<<endl;
	unsigned int data1 = (*vec)[k+1];
         //cout<<" data type="<<((data1& 0xf8000000)>>27)<<endl;
	} else if (((data & 0xf8000000)>>27) == 0x18&&SLOTNUM==6) {
           CHANNEL = ((data & 0x7800000)>>23) ;
           //cout<<"channel="<<CHANNEL<<endl;
           PSCADCSamples[CHANNEL][10]=200.;
           PSCADCPedestal[CHANNEL]=100.;
	} else if (((data & 0xf8000000)>>27) == 0x14&&SLOTNUM==6) {
	  CHANNEL = ((data & 0x7800000)>>23) ; // flash is couning channels from 1 to 72 need 0 to 71
	  WSize =  (data & 0xFFF);
          //cout<<" channel,samples="<<CHANNEL<<" "<<WSize<<endl;
	  DATAReady = WSize/2;
	  idx = 0;
	  ped = 0.;
	  pedcnt = 0;
	  PSCADCPedestal[CHANNEL] = 0.;
	  //check if(data_bank->tag==58)cout<<endl<<endl<<" slot,ch,wsize="<<SLOTNUM<<" "<<CHANNEL<<" "<<WSize<<endl;;
	} else if (DATAReady>0&&SLOTNUM==6) { // Window Raw Data values
	  DATAReady--; 
	  if (pedcnt<16){
	    //if (pedcnt<4)
	    adc1 =  (float)((data & 0x1FFF0000) >> 16);
	    adc2 =  (float)(data & 0x1FFF);
	    ped += adc1;
	    ped += adc2;
	    PSCADCSamples[CHANNEL][idx++] = adc1;
	    PSCADCSamples[CHANNEL][idx++] = adc2;
	    //cout<<" slot,ch,idx,ADCSamples="<<SLOTNUM<<" "<<CHANNEL<<" "<<idx<<" "<< PSCADCSamples[CHANNEL][idx-1]<<endl;
	    pedcnt+=2;
	    if (pedcnt>15){
	      ped /= 16.;
	      PSCADCPedestal[CHANNEL] = ped;
	    }
	  } else {
	    adc1 =  (float)((data & 0x1FFF0000) >> 16);
	    adc2 =  (float)(data & 0x1FFF);
            if(adc1>4095)adc1=4095;
            if(adc2>4095)adc2=4095;
	    PSCADCSamples[CHANNEL][idx++] = adc1;
	    PSCADCSamples[CHANNEL][idx++] = adc2;
          }	    
	}//end data ready
	//cok PSCWindowSize = WSize;
	PSCWindowSize = 100;
      } // data loop
    } //Sz>0
    //end rocps2
  } else if ( data_bank->tag == 76 && bankPtr->tag==17 ) { // rocTRD1  == 0x4C SRS bank = 17

    printf("Found SRS data bank 17 \n");

    
    gemView->ProcessSingleEvent(bankPtr,gemclust);
    printf("***>>  SRS clusters = %d \n",gemclust.size());
    for (int ic=0; ic<gemclust.size(); ic++) printf(" SRS clusters = %d x=%f y=%f \n",ic, gemclust[ic].x, gemclust[ic].y);

    sleep(1);

  }
}

