#include "GemView.h"
#include "stdio.h"
#include "TApplication.h"
#include "TAxis.h"
#include <vector>
#include <map>
#include <ctime>
#include <cstdlib>
#include "TMath.h"

using namespace std;
#define SEC *1e3
//=====================================================
//static variables to be used in the global scale
//PRadClient* GemView::TheClient = new PRadClient();
static int TheCurrentLine = 0;

//==================================================
GemView::GemView(int runnum) {

    printf(" => GemView::GemView() ==>  Create Canvas \n");

    //c3 = new TCanvas("c3", "SRS", 2000,50, 1000, 900);
    //c3->Divide(3,4);
    c3 = NULL;

    printf(" => GemView::GemView() ==> Init Config file \n");
    fConfig.Init("./db/Config_GEM_TRD.cfg");
    //  fConfig.Init("./db/Config_crGEM_Sr90.cfg");
    printf(" = GemView::GemView() ==> Init Config file done \n");
    fRunType = (TString) fConfig.GetRunType();
    fNbOfTimeSamples = fConfig.GetNbOfTimeSamples();
    printf(" = GemView::GemView() ==> Start the run of type = %s and nbTimeS = %d\n", fRunType.Data(),
           fNbOfTimeSamples);
    //add reading map here
    fMapping = GemMapping::GetInstance();
    fMapping->LoadMapping(fConfig.GetInputMapName());
    fMapping->PrintMapping();
    fNbADCBins = 50;
    printf(" = GemView::GemView() ==> Mapping is loaded \n");
    InitConfig(runnum);
    InitGUI();
    InitToolBox();
    //InitRootGraphics();
    fCurrentEntry = 1; //start counting event from 1
    SetMonitor(fConfig.GetCycleWait() SEC);//second
    printf(" = GemView::InitConfig() ==> All initialisation done \n\n");

}

//==================================================
GemView::~GemView() {
}

//==================================================
void GemView::InitConfig(int runnum) {
    printf(" = GemView::InitConfig() ==> Enter \n");

    if (fConfig.GetOnlineMode()) {
        /*
       fHandler = new GemInputHandler(fConfig.GetETIPAddress(), fConfig.GetTCPPort(), fConfig.GetInputFileName());
        //assert(fConfig.GetETStationName() != NULL);
        fConfig.GetETStationName();
        std::string astr(fConfig.GetETStationName());
        fHandler->CreateStation(astr, fConfig.GetETMode());
        fHandler->AttachStation();
        */
        printf(" = GemView::InitConfig() ==> ONLINE fHandler does not supprted \n");
        exit(1);
    } else {
        fHandler = new GemInputHandler(c3);
        printf(" = GemView::InitConfig() ==> OFFLINE fHandler is initialised for run = %d \n", runnum);
    }

    // Run config Parameters
    fHandler->SetZeroSupCut(fConfig.GetZeroSupCut());;
    fHandler->SetCommonModeCut(fConfig.GetComModeCut());
    fHandler->SetHitPeakOrSumADCs(fConfig.GetHitPeakOrSumADCs());
    fHandler->SetCentralOrAllStripsADCs(fConfig.GetCentralOrAllStripsADCs());
    fHandler->SetMinADCvalue(fConfig.GetMinADCvalue());
    fHandler->SetMinClusterSize(fConfig.GetMinClusterSize());
    fHandler->SetMaxClusterSize(fConfig.GetMaxClusterSize());
    fHandler->SetMaxClusterMult(fConfig.GetMaxClusterMult());
    fHandler->SetNbOfTimeSamples(fConfig.GetNbOfTimeSamples());
    fHandler->SetStopTimeSample(fConfig.GetStopTimeSamples());
    fHandler->SetStartTimeSample(fConfig.GetStartTimeSamples());
    fRootFileName = fConfig.GetOutputFileName();

    // Zero suppression mode
    if ((fRunType == "SINGLEEVENT") || (fRunType == "MULTIEVENTS") || (fRunType == "MULTIFILES") ||
        (fRunType == "SEARCHEVENTS")) {
        InitHistForZeroSup();

        char ped_name[256];
        //fPedestal = new GEMPedestal(fConfig.GetPedFileName(), fConfig.GetNbOfTimeSamples());
        sprintf(ped_name, "./pedestalDir/pedestal_%06d.root", runnum);
        fPedestal = new GEMPedestal(ped_name, fConfig.GetNbOfTimeSamples());
        fHandler->InitPedestal(fPedestal);
        printf(" = GemView::InitConfig() ==> Initialisation for a %s Run \n", fRunType.Data());
    }

        // Pedestal mode
    else if (fRunType == "PEDESTAL") {
        InitHistForRawData();
        fPedestal = new GEMPedestal(fConfig.GetPedFileName(), fConfig.GetNbOfTimeSamples());
        fPedestal->BookHistos();
        fHandler->InitPedestal(fPedestal);
        printf(" = GemView::InitConfig() ==> Initialisation for a %s Run \n", fRunType.Data());
    }

        // Raw data mode
    else if (fRunType == "RAWDATA") {
        InitHistForRawData();
        printf(" = GemView::InitConfig() ==> Initialisation for a %s Run \n", fRunType.Data());
    } else printf(" = GemView::InitConfig() ==> No Run specified \n");
    printf(" = GemView::InitConfig() ==> Done \n");
}

//=====================================================
void GemView::InitGUI() {
#if ROOT_VERSION_CODE >= ROOT_VERSION(5, 16, 0)
// Make sure the ROOT graphical layer is initialised.
//static struct needgraph {   needgraph () {  TApplication::NeedGraphicsLibs() ;  gApplication->InitializeGraphics();} }  needgraph;
#endif

    printf(" = GemView::InitGUI() ==> Enter Init GUI for a %s run\n", fRunType.Data());
    if ((fRunType == "SINGLEEVENT") || (fRunType == "SEARCHEVENTS")) {
        // printf(" = GemView::InitGUI() ==> Init GUI for a %s run\n",  fRunType.Data()) ;
        map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
        map<TString, Int_t>::const_iterator plane_itr;
        for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
            Int_t planeId = (*plane_itr).second;
            //     if (planeId == 1) continue ;
            Int_t detId = fMapping->GetDetectorID(fMapping->GetDetectorFromPlane((*plane_itr).first));
            Int_t i = 2 * detId + planeId;
            Int_t j = fMapping->GetNbOfPlane() + i;
            printf(" = GemView::InitGUI() ==> Widget initialized for %s \n", fRunType.Data());
        }
    } else printf(" = GemView::InitGUI() ==> No Run specified \n");
    printf(" = GemView::InitGUI() ==> Exit Init GUI for a %s run\n", fRunType.Data());
}

//=====================================================
void GemView::InitToolBox() {
    printf(" = GemView::InitToolBox() ==> Enter Init Tool Box for a %s run\n", fRunType.Data());

    printf(" = GemView::InitToolBox() ==> Exit Init Tool Box for a %s run\n", fRunType.Data());
}

//==================================================
void GemView::InitRootGraphics() {
    if (fRunType == "MULTIFILES") return;

    printf(" = GemView::InitRootGraphics() ==> Enter Init ROOT Graphics for a %s run\n", fRunType.Data());
#if ROOT_VERSION_CODE >= ROOT_VERSION(5, 16, 0)
    // Make sure the ROOT graphical layer is initialised.
    static struct needgraph {
        needgraph() {
            TApplication::NeedGraphicsLibs();
            gApplication->InitializeGraphics();
        }
    } needgraph;
#endif
    printf(" = GemView::InitRootGraphics() ==> Exit Init ROOT Graphics for a %s run\n", fRunType.Data());
}

//=====================================================
void GemView::Clear() {
    for (int i = 0; i < fMapping->GetNbOfFECs(); i++) fFEC[i]->ClearHist();
}

//=====================================================
void GemView::DeleteHistForMultipleFiles() {
    // printf(" = GemView::DeleteHistos() ==> Enter Init for zero sup analysis %s run\n",  fRunType.Data()) ;

    // PLANES
    map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
    map<TString, Int_t>::const_iterator plane_itr;
    for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
        TString plane = (*plane_itr).first;
        Int_t planeId = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane(plane)) + (*plane_itr).second;
        Int_t id = fMapping->GetNbOfPlane() + planeId;
        delete fHitHist[planeId];
        delete fHitHist[id];
        delete fClusterHist[planeId];
        delete fClusterHist[id];
        delete fClusterInfoHist[planeId];
        delete fClusterInfoHist[id];
        delete fADCHist[planeId];
        delete fADCTimeBinPosHist[planeId];
        delete fADCTimeBinPosHist[id];
        delete fTimeBinPosHist[planeId];
        delete fTimeBinPosHist[id];
    }

    delete[] fADCHist;
    delete[] fHitHist;
    delete[] fClusterHist;
    delete[] fClusterInfoHist;
    delete[] fTimeBinPosHist;
    delete[] fADCTimeBinPosHist;

    // DETECTORS
    map<Int_t, TString> listOfDetectors = fMapping->GetDetectorFromIDMap();
    map<Int_t, TString>::const_iterator det_itr;
    for (det_itr = listOfDetectors.begin(); det_itr != listOfDetectors.end(); ++det_itr) {
        Int_t detId = (*det_itr).first;
        TString detector = (*det_itr).second;
        delete f2DPlotsHist[3 * detId];
        delete f2DPlotsHist[3 * detId + 1];
        delete f2DPlotsHist[3 * detId + 2];
        delete fChargeSharingHist[detId];
        delete fChargeRatioHist[detId];
    }

    delete[] f2DPlotsHist;
    delete[] fChargeSharingHist;
    delete[] fChargeRatioHist;

    //  printf(" = GemView::DeleteHistos ==> Exit Init for zero sup analysis %s run\n",  fRunType.Data()) ;
}

//=====================================================
void GemView::InitHistForMultipleFiles() {
    //  printf(" = GemView::InitHistForMultipleFiles() ==> Enter Init for zero sup analysis %s run\n",  fRunType.Data()) ;

    Int_t nbOfPlaneHists = 2 * fMapping->GetNbOfPlane();
    Int_t driftTime = 25 * fNbOfTimeSamples;

    // PLANES
    fADCHist = new TH1F *[fMapping->GetNbOfPlane()];
    fHitHist = new TH1F *[nbOfPlaneHists];
    fClusterHist = new TH1F *[nbOfPlaneHists];
    fClusterInfoHist = new TH1F *[nbOfPlaneHists];
    fTimeBinPosHist = new TH2F *[nbOfPlaneHists];
    fADCTimeBinPosHist = new TH2F *[nbOfPlaneHists];

    map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
    map<TString, Int_t>::const_iterator plane_itr;
    for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
        TString plane = (*plane_itr).first;
        Int_t planeId = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane(plane)) + (*plane_itr).second;
        Int_t id = fMapping->GetNbOfPlane() + planeId;
        Int_t nbOfStrips = 128 * fMapping->GetNbOfAPVsOnPlane((*plane_itr).first);
        Float_t size = fMapping->GetPlaneSize((*plane_itr).first);

        TString hitCount = (*plane_itr).first + "_stripHitCount";
        fHitHist[planeId] = new TH1F(hitCount, hitCount, nbOfStrips, -1 * size / 2., size / 2.);
        fHitHist[planeId]->GetXaxis()->SetTitle("Strip hit position");
        fHitHist[planeId]->GetYaxis()->SetTitle("Strip hit counts");
        fHitHist[planeId]->SetLabelSize(0.04, "X");
        fHitHist[planeId]->SetLabelSize(0.04, "Y");

        TString hitUniformity = (*plane_itr).first + "_HitUniformity";
        fHitHist[id] = new TH1F(hitUniformity, hitUniformity, nbOfStrips, -1 * size / 2., size / 2.);
        fHitHist[id]->GetXaxis()->SetTitle("Strip hit position");
        fHitHist[id]->GetYaxis()->SetTitle("Average strip hit ADC");
        fHitHist[id]->SetLabelSize(0.04, "X");
        fHitHist[id]->SetLabelSize(0.04, "Y");

        TString clusterCount = (*plane_itr).first + "_clusterHitCount";
        fClusterHist[planeId] = new TH1F(clusterCount, clusterCount, nbOfStrips, -1 * size / 2., size / 2.);
        fClusterHist[planeId]->GetXaxis()->SetTitle("Cluster position");
        fClusterHist[planeId]->GetYaxis()->SetTitle("Counts");
        fClusterHist[planeId]->SetLabelSize(0.04, "X");
        fClusterHist[planeId]->SetLabelSize(0.04, "Y");

        TString clusterUniformity = (*plane_itr).first + "_clusterUniformity";
        fClusterHist[id] = new TH1F(clusterUniformity, clusterUniformity, nbOfStrips, -1 * size / 2., size / 2.);
        fClusterHist[id]->GetXaxis()->SetTitle("Cluster's position");
        fClusterHist[id]->GetYaxis()->SetTitle("Average ADC");
        fClusterHist[id]->SetLabelSize(0.04, "X");
        fClusterHist[id]->SetLabelSize(0.04, "Y");

        TString clusterMult = (*plane_itr).first + "_clusterMultiplicity";
        fClusterInfoHist[planeId] = new TH1F(clusterMult, clusterMult, 21, 0, 20.);
        fClusterInfoHist[planeId]->GetXaxis()->SetTitle("Nb of clusters");
        fClusterInfoHist[planeId]->GetYaxis()->SetTitle("Counts");
        fClusterInfoHist[planeId]->SetLabelSize(0.04, "X");
        fClusterInfoHist[planeId]->SetLabelSize(0.04, "Y");

        TString clusterSize = (*plane_itr).first + "_clusterSize";
        fClusterInfoHist[id] = new TH1F(clusterSize, clusterSize, 21, 0, 20.);
        fClusterInfoHist[id]->GetXaxis()->SetTitle("Nb of hit per cluster");
        fClusterInfoHist[id]->GetYaxis()->SetTitle("Counts");
        fClusterInfoHist[id]->SetLabelSize(0.04, "X");
        fClusterInfoHist[id]->SetLabelSize(0.04, "Y");

        TString adcDist = (*plane_itr).first + "_adcDist";
        fADCHist[planeId] = new TH1F(adcDist, adcDist, fNbADCBins, fConfig.GetMinADCvalue(), fConfig.GetMaxADCvalue());
        fADCHist[planeId]->GetXaxis()->SetTitle("ADC");
        fADCHist[planeId]->GetYaxis()->SetTitle("Counts");
        fADCHist[planeId]->SetLabelSize(0.04, "X");
        fADCHist[planeId]->SetLabelSize(0.04, "Y");

        Int_t tsbin = (Int_t) (nbOfStrips / 8);

        TString alltimeBin = (*plane_itr).first + "adc_vs_pos_allTimeBin";
        fADCTimeBinPosHist[planeId] = new TH2F(alltimeBin, alltimeBin, fNbOfTimeSamples, 0, driftTime, tsbin, 0,
                                               (nbOfStrips - 1));
        fADCTimeBinPosHist[planeId]->GetXaxis()->SetTitle("drift time");
        fADCTimeBinPosHist[planeId]->GetYaxis()->SetTitle("Position (mm)");
        fADCTimeBinPosHist[planeId]->SetLabelSize(0.04, "X");
        fADCTimeBinPosHist[planeId]->SetLabelSize(0.04, "Y");

        TString timeBinPeak = (*plane_itr).first + "adc_vs_pos_timeBinPeak";
        fADCTimeBinPosHist[id] = new TH2F(timeBinPeak, timeBinPeak, fNbOfTimeSamples, 0, driftTime, tsbin, 0,
                                          (nbOfStrips - 1));
        fADCTimeBinPosHist[id]->GetXaxis()->SetTitle("drift time");
        fADCTimeBinPosHist[id]->GetYaxis()->SetTitle("Position (mm)");
        fADCTimeBinPosHist[id]->SetLabelSize(0.04, "X");
        fADCTimeBinPosHist[id]->SetLabelSize(0.04, "Y");

        alltimeBin = (*plane_itr).first + "_pos_vs_allTimeBin";
        fTimeBinPosHist[planeId] = new TH2F(alltimeBin, alltimeBin, fNbOfTimeSamples, 0, driftTime, tsbin, 0,
                                            (nbOfStrips - 1));
        fTimeBinPosHist[planeId]->GetXaxis()->SetTitle("drift time");
        fTimeBinPosHist[planeId]->GetYaxis()->SetTitle("Position (mm)");
        fTimeBinPosHist[planeId]->SetLabelSize(0.04, "X");
        fTimeBinPosHist[planeId]->SetLabelSize(0.04, "Y");

        timeBinPeak = (*plane_itr).first + "_pos_vs_timeBinPeak";
        fTimeBinPosHist[id] = new TH2F(timeBinPeak, timeBinPeak, fNbOfTimeSamples, 0, driftTime, tsbin, 0,
                                       (nbOfStrips - 1));
        fTimeBinPosHist[id]->GetXaxis()->SetTitle("drift time");
        fTimeBinPosHist[id]->GetYaxis()->SetTitle("Position (mm)");
        fTimeBinPosHist[id]->SetLabelSize(0.04, "X");
        fTimeBinPosHist[id]->SetLabelSize(0.04, "Y");
        //    printf(" = GemView::InitHistForMultipleFiles() ==> plane[%s] to histId[%d]  \n", plane.Data(), planeId);
    }
    // DETECTORS
    f2DPlotsHist = new TH2F *[3 * fMapping->GetNbOfDetectors()];
    fChargeSharingHist = new TH2F *[fMapping->GetNbOfDetectors()];
    fChargeRatioHist = new TH1F *[fMapping->GetNbOfDetectors()];

    map<Int_t, TString> listOfDetectors = fMapping->GetDetectorFromIDMap();
    map<Int_t, TString>::const_iterator det_itr;
    for (det_itr = listOfDetectors.begin(); det_itr != listOfDetectors.end(); ++det_itr) {
        Int_t detId = (*det_itr).first;
        TString detector = (*det_itr).second;

        Float_t size1 = 0.5 * fMapping->GetPlaneSize(fMapping->GetPlaneListFromDetector(detector).front());
        Float_t size2 = 0.5 * fMapping->GetPlaneSize(fMapping->GetPlaneListFromDetector(detector).back());
        //    Int_t nbin1 = 256;
        //    Int_t nbin2 = 256;
        Int_t nbin1 = 64;
        Int_t nbin2 = 64;

        /**
        if (fMapping->GetReadoutBoardFromDetector(detector) == "UV_ANGLE") {
          size1 = 0.5 * fMapping->GetPlaneSize(fMapping->GetPlaneListFromDetector(detector).front());
          size2 = 0.5 * fMapping->GetPlaneSize(fMapping->GetPlaneListFromDetector(detector).back());
          nbin1 = 25 ;
          nbin2 = 25 ;
        }
        */

        TString hitMap = detector + "_ClusterHitMap";
        f2DPlotsHist[3 * detId] = new TH2F(hitMap, hitMap, nbin1, -1 * size1, size1, nbin2, -1 * size2, size2);
        f2DPlotsHist[3 * detId]->GetXaxis()->SetTitle("cluster x-position (mm)");
        f2DPlotsHist[3 * detId]->GetYaxis()->SetTitle("cluster y-position (mm)");
        f2DPlotsHist[3 * detId]->SetLabelSize(0.04, "X");
        f2DPlotsHist[3 * detId]->SetLabelSize(0.04, "Y");

        TString adcMap = detector + "_gain_Uniformity";
        f2DPlotsHist[3 * detId + 1] = new TH2F(adcMap, adcMap, nbin1, -1 * size1, size1, nbin2, -1 * size2, size2);
        f2DPlotsHist[3 * detId + 1]->GetXaxis()->SetTitle("cluster x-position (mm)");
        f2DPlotsHist[3 * detId + 1]->GetYaxis()->SetTitle("cluster y-position (mm)");
        f2DPlotsHist[3 * detId + 1]->SetLabelSize(0.04, "X");
        f2DPlotsHist[3 * detId + 1]->SetLabelSize(0.04, "Y");

        TString timingMap = detector + "_timing_Uniformity";
        f2DPlotsHist[3 * detId + 2] = new TH2F(timingMap, timingMap, nbin1, -1 * size1, size1, nbin2, -1 * size2,
                                               size2);
        f2DPlotsHist[3 * detId + 2]->GetXaxis()->SetTitle("cluster x-position (mm)");
        f2DPlotsHist[3 * detId + 2]->GetYaxis()->SetTitle("cluster y-position (mm)");
        f2DPlotsHist[3 * detId + 2]->SetLabelSize(0.04, "X");
        f2DPlotsHist[3 * detId + 2]->SetLabelSize(0.04, "Y");

        TString chSharing = detector + "_charge_Sharing";
        fChargeSharingHist[detId] = new TH2F(chSharing, chSharing, 50, fConfig.GetMinADCvalue(),
                                             fConfig.GetMaxADCvalue(), 50, fConfig.GetMinADCvalue(),
                                             fConfig.GetMaxADCvalue());
        fChargeSharingHist[detId]->GetXaxis()->SetTitle("ADC in X-strips");
        fChargeSharingHist[detId]->GetYaxis()->SetTitle("ADC in Y-strips");
        fChargeSharingHist[detId]->SetLabelSize(0.04, "X");
        fChargeSharingHist[detId]->SetLabelSize(0.04, "Y");

        TString chRatio = detector + "_charge_Ratio";
        fChargeRatioHist[detId] = new TH1F(chRatio, chRatio, 31, 0.0, 3.0);
        fChargeRatioHist[detId]->GetXaxis()->SetTitle("ADC charge ratio x / Y");
        fChargeRatioHist[detId]->GetYaxis()->SetTitle("ratio");
        fChargeRatioHist[detId]->SetLabelSize(0.04, "X");
        fChargeRatioHist[detId]->SetLabelSize(0.04, "Y");
        //    printf(" = GemView::InitHistForMultipleFiles() ==> detector[%s] to histId[%d]  \n", detector.Data(), detId) ;
    }
    //  printf(" = GemView::InitHistForMultipleFiles() ==> Exit Init for zero sup analysis %s run\n",  fRunType.Data()) ;
}

//=====================================================
void GemView::InitHistForZeroSup() {
    printf(" = GemView::InitHistForZeroSup() ==> Enter Init for zero sup analysis %s run\n", fRunType.Data());

    Int_t nbOfPlaneHists = 2 * fMapping->GetNbOfPlane();
    Int_t driftTime = 25 * fNbOfTimeSamples;

    // PLANES
    if ((fRunType == "SINGLEEVENT") || (fRunType == "SEARCHEVENTS")) {
        f1DSingleEventHist = new TH1F *[nbOfPlaneHists];
        f2DSingleEventHist = new TH2F *[nbOfPlaneHists];

        map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
        map<TString, Int_t>::const_iterator plane_itr;
        for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
            TString plane = (*plane_itr).first;
            Int_t planeId = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane(plane)) + (*plane_itr).second;
            Int_t nbOfStrips = 128 * fMapping->GetNbOfAPVsOnPlane((*plane_itr).first);
            Float_t size = fMapping->GetPlaneSize((*plane_itr).first);
            Int_t id = fMapping->GetNbOfPlane() + planeId;

            Int_t nbBins = nbOfStrips * fNbOfTimeSamples;

            TString zeroSup = (*plane_itr).first + "_zeroSup";
            f1DSingleEventHist[planeId] = new TH1F(zeroSup, zeroSup, nbBins, 0, (nbBins - 1));
            f1DSingleEventHist[planeId]->SetLabelSize(0.04, "X");
            f1DSingleEventHist[planeId]->SetLabelSize(0.04, "Y");

            TString presZeroSup = (*plane_itr).first + "_pedSub";
            f1DSingleEventHist[id] = new TH1F(presZeroSup, presZeroSup, nbBins, 0, (nbBins - 1));
            f1DSingleEventHist[id]->SetLabelSize(0.04, "X");
            f1DSingleEventHist[id]->SetLabelSize(0.04, "Y");

            zeroSup = (*plane_itr).first + "_SingleHit_vs_timebin_2D";
            f2DSingleEventHist[planeId] = new TH2F(zeroSup, zeroSup, fNbOfTimeSamples, 0, driftTime, nbOfStrips, 0,
                                                   (nbOfStrips - 1));
            f2DSingleEventHist[planeId]->GetXaxis()->SetTitle("drift time");
            f2DSingleEventHist[planeId]->GetYaxis()->SetTitle("Strip Nb");
            f2DSingleEventHist[planeId]->GetYaxis()->SetTitleSize(0.05);
            f2DSingleEventHist[planeId]->GetXaxis()->SetTitleSize(0.05);
            f2DSingleEventHist[planeId]->GetXaxis()->SetTitleOffset(1.1);
            f2DSingleEventHist[planeId]->GetYaxis()->SetTitleOffset(1.1);

            f2DSingleEventHist[planeId]->SetLabelSize(0.04, "X");
            f2DSingleEventHist[planeId]->SetLabelSize(0.04, "Y");
            printf(" = GemView::InitHistForZeroSup() => Initialize single events histos: plane[%s], nbStrips=%d, nbTimeS=%d, size=%f, to histId[%d]\n",
                   plane.Data(), nbOfStrips, fNbOfTimeSamples, size, planeId);

            presZeroSup = (*plane_itr).first + "Accu_Hits_vs_timebin_2D";
            f2DSingleEventHist[id] = new TH2F(presZeroSup, presZeroSup, fNbOfTimeSamples, 0, driftTime, nbOfStrips, 0,
                                              (nbOfStrips - 1));
            f2DSingleEventHist[id]->GetXaxis()->SetTitle("drift time");
            f2DSingleEventHist[id]->GetYaxis()->SetTitle("Strip Nb");
            f2DSingleEventHist[id]->GetYaxis()->SetTitleSize(0.05);
            f2DSingleEventHist[id]->GetXaxis()->SetTitleSize(0.05);
            f2DSingleEventHist[id]->GetYaxis()->SetTitleOffset(1.1);
            f2DSingleEventHist[id]->GetXaxis()->SetTitleOffset(1.1);

            f2DSingleEventHist[id]->SetLabelSize(0.04, "X");
            f2DSingleEventHist[id]->SetLabelSize(0.04, "Y");
            printf(" = GemView::InitHistForZeroSup() => Initialize histos: plane[%s], nbOfStrips=%d, nbOfTimeSamples=%d, size=%f, to histId[%d]\n",
                   plane.Data(), nbOfStrips, fNbOfTimeSamples, size, id);
        }
    }

    if (fRunType == "MULTIEVENTS") {
        // PLANES
        fADCHist = new TH1F *[fMapping->GetNbOfPlane()];
        fHitHist = new TH1F *[nbOfPlaneHists];
        fClusterHist = new TH1F *[nbOfPlaneHists];
        fClusterInfoHist = new TH1F *[nbOfPlaneHists];
        fTimeBinPosHist = new TH2F *[nbOfPlaneHists];
        fADCTimeBinPosHist = new TH2F *[nbOfPlaneHists];

        map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
        map<TString, Int_t>::const_iterator plane_itr;
        for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
            TString plane = (*plane_itr).first;
            Int_t planeId = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane(plane)) + (*plane_itr).second;
            Int_t id = fMapping->GetNbOfPlane() + planeId;
            Int_t nbOfStrips = 128 * fMapping->GetNbOfAPVsOnPlane((*plane_itr).first);
            Float_t size = fMapping->GetPlaneSize((*plane_itr).first);

            TString hitCount = (*plane_itr).first + "_stripHitCount";
            fHitHist[planeId] = new TH1F(hitCount, hitCount, nbOfStrips, -1 * size / 2., size / 2.);
            fHitHist[planeId]->GetXaxis()->SetTitle("Strip hit position");
            fHitHist[planeId]->GetYaxis()->SetTitle("Strip hit counts");
            fHitHist[planeId]->SetLabelSize(0.04, "X");
            fHitHist[planeId]->SetLabelSize(0.04, "Y");

            TString hitUniformity = (*plane_itr).first + "_HitUniformity";
            fHitHist[id] = new TH1F(hitUniformity, hitUniformity, nbOfStrips, -1 * size / 2., size / 2.);
            fHitHist[id]->GetXaxis()->SetTitle("Strip hit position");
            fHitHist[id]->GetYaxis()->SetTitle("Average strip hit ADC");
            fHitHist[id]->SetLabelSize(0.04, "X");
            fHitHist[id]->SetLabelSize(0.04, "Y");

            TString clusterCount = (*plane_itr).first + "_clusterHitCount";
            fClusterHist[planeId] = new TH1F(clusterCount, clusterCount, nbOfStrips, -1 * size / 2., size / 2.);
            fClusterHist[planeId]->GetXaxis()->SetTitle("Cluster position");
            fClusterHist[planeId]->GetYaxis()->SetTitle("Counts");
            fClusterHist[planeId]->SetLabelSize(0.04, "X");
            fClusterHist[planeId]->SetLabelSize(0.04, "Y");

            TString clusterUniformity = (*plane_itr).first + "_clusterUniformity";
            fClusterHist[id] = new TH1F(clusterUniformity, clusterUniformity, nbOfStrips, -1 * size / 2., size / 2.);
            fClusterHist[id]->GetXaxis()->SetTitle("Cluster's position");
            fClusterHist[id]->GetYaxis()->SetTitle("Average ADC");
            fClusterHist[id]->SetLabelSize(0.04, "X");
            fClusterHist[id]->SetLabelSize(0.04, "Y");

            TString clusterMult = (*plane_itr).first + "_clusterMultiplicity";
            fClusterInfoHist[planeId] = new TH1F(clusterMult, clusterMult, 21, 0, 20.);
            fClusterInfoHist[planeId]->GetXaxis()->SetTitle("Nb of clusters");
            fClusterInfoHist[planeId]->GetYaxis()->SetTitle("Counts");
            fClusterInfoHist[planeId]->SetLabelSize(0.04, "X");
            fClusterInfoHist[planeId]->SetLabelSize(0.04, "Y");

            TString clusterSize = (*plane_itr).first + "_clusterSize";
            fClusterInfoHist[id] = new TH1F(clusterSize, clusterSize, 21, 0, 20.);
            fClusterInfoHist[id]->GetXaxis()->SetTitle("Nb of hit per cluster");
            fClusterInfoHist[id]->GetYaxis()->SetTitle("Counts");
            fClusterInfoHist[id]->SetLabelSize(0.04, "X");
            fClusterInfoHist[id]->SetLabelSize(0.04, "Y");

            //      Int_t nbADCBins = 500 ;
            TString adcDist = (*plane_itr).first + "_adcDist";
            fADCHist[planeId] = new TH1F(adcDist, adcDist, fNbADCBins, fConfig.GetMinADCvalue(),
                                         fConfig.GetMaxADCvalue());
            fADCHist[planeId]->GetXaxis()->SetTitle("ADC");
            fADCHist[planeId]->GetYaxis()->SetTitle("Counts");
            fADCHist[planeId]->SetLabelSize(0.04, "X");
            fADCHist[planeId]->SetLabelSize(0.04, "Y");

            Int_t tsbin = (Int_t) (nbOfStrips / 8);

            TString alltimeBin = (*plane_itr).first + "adc_vs_pos_allTimeBin";
            fADCTimeBinPosHist[planeId] = new TH2F(alltimeBin, alltimeBin, fNbOfTimeSamples, 0, driftTime, tsbin, 0,
                                                   (nbOfStrips - 1));
            fADCTimeBinPosHist[planeId]->GetXaxis()->SetTitle("drift time");
            fADCTimeBinPosHist[planeId]->GetYaxis()->SetTitle("Position (mm)");
            fADCTimeBinPosHist[planeId]->SetLabelSize(0.04, "X");
            fADCTimeBinPosHist[planeId]->SetLabelSize(0.04, "Y");

            TString timeBinPeak = (*plane_itr).first + "adc_vs_pos_timeBinPeak";
            fADCTimeBinPosHist[id] = new TH2F(timeBinPeak, timeBinPeak, fNbOfTimeSamples, 0, driftTime, tsbin, 0,
                                              (nbOfStrips - 1));
            fADCTimeBinPosHist[id]->GetXaxis()->SetTitle("drift time");
            fADCTimeBinPosHist[id]->GetYaxis()->SetTitle("Position (mm)");
            fADCTimeBinPosHist[id]->SetLabelSize(0.04, "X");
            fADCTimeBinPosHist[id]->SetLabelSize(0.04, "Y");

            alltimeBin = (*plane_itr).first + "_pos_vs_allTimeBin";
            fTimeBinPosHist[planeId] = new TH2F(alltimeBin, alltimeBin, fNbOfTimeSamples, 0, driftTime, tsbin, 0,
                                                (nbOfStrips - 1));
            fTimeBinPosHist[planeId]->GetXaxis()->SetTitle("drift time");
            fTimeBinPosHist[planeId]->GetYaxis()->SetTitle("Position (mm)");
            fTimeBinPosHist[planeId]->SetLabelSize(0.04, "X");
            fTimeBinPosHist[planeId]->SetLabelSize(0.04, "Y");

            timeBinPeak = (*plane_itr).first + "_pos_vs_timeBinPeak";
            fTimeBinPosHist[id] = new TH2F(timeBinPeak, timeBinPeak, fNbOfTimeSamples, 0, driftTime, tsbin, 0,
                                           (nbOfStrips - 1));
            fTimeBinPosHist[id]->GetXaxis()->SetTitle("drift time");
            fTimeBinPosHist[id]->GetYaxis()->SetTitle("Position (mm)");
            fTimeBinPosHist[id]->SetLabelSize(0.04, "X");
            fTimeBinPosHist[id]->SetLabelSize(0.04, "Y");
            printf(" = GemView::InitHistForZeroSup() ==> plane[%s] to histId[%d]  \n", plane.Data(), planeId);
        }

        // DETECTORS
        f2DPlotsHist = new TH2F *[3 * fMapping->GetNbOfDetectors()];
        fChargeSharingHist = new TH2F *[fMapping->GetNbOfDetectors()];
        fChargeRatioHist = new TH1F *[fMapping->GetNbOfDetectors()];

        map<Int_t, TString> listOfDetectors = fMapping->GetDetectorFromIDMap();
        map<Int_t, TString>::const_iterator det_itr;
        for (det_itr = listOfDetectors.begin(); det_itr != listOfDetectors.end(); ++det_itr) {
            Int_t detId = (*det_itr).first;
            TString detector = (*det_itr).second;

            Float_t size1 = 0.5 * fMapping->GetPlaneSize(fMapping->GetPlaneListFromDetector(detector).front());
            Float_t size2 = 0.5 * fMapping->GetPlaneSize(fMapping->GetPlaneListFromDetector(detector).back());
            //    Int_t nbin1 = 256;
            //      Int_t nbin2 = 256;
            Int_t nbin1 = 64;
            Int_t nbin2 = 64;

            /**
            if (fMapping->GetReadoutBoardFromDetector(detector) == "UV_ANGLE") {
          size1 = 0.5 * fMapping->GetPlaneSize(fMapping->GetPlaneListFromDetector(detector).front());
          size2 = 0.5 * fMapping->GetPlaneSize(fMapping->GetPlaneListFromDetector(detector).back());
          nbin1 = 25 ;
          nbin2 = 25 ;
            }
            */

            TString hitMap = detector + "_ClusterHitMap";
            f2DPlotsHist[3 * detId] = new TH2F(hitMap, hitMap, nbin1, -1 * size1, size1, nbin2, -1 * size2, size2);
            f2DPlotsHist[3 * detId]->GetXaxis()->SetTitle("cluster x-position (mm)");
            f2DPlotsHist[3 * detId]->GetYaxis()->SetTitle("cluster y-position (mm)");
            f2DPlotsHist[3 * detId]->SetLabelSize(0.04, "X");
            f2DPlotsHist[3 * detId]->SetLabelSize(0.04, "Y");

            TString adcMap = detector + "_gain_Uniformity";
            f2DPlotsHist[3 * detId + 1] = new TH2F(adcMap, adcMap, nbin1, -1 * size1, size1, nbin2, -1 * size2, size2);
            f2DPlotsHist[3 * detId + 1]->GetXaxis()->SetTitle("cluster x-position (mm)");
            f2DPlotsHist[3 * detId + 1]->GetYaxis()->SetTitle("cluster y-position (mm)");
            f2DPlotsHist[3 * detId + 1]->SetLabelSize(0.04, "X");
            f2DPlotsHist[3 * detId + 1]->SetLabelSize(0.04, "Y");

            TString timingMap = detector + "_timing_Uniformity";
            f2DPlotsHist[3 * detId + 2] = new TH2F(timingMap, timingMap, nbin1, -1 * size1, size1, nbin2, -1 * size2,
                                                   size2);
            f2DPlotsHist[3 * detId + 2]->GetXaxis()->SetTitle("cluster x-position (mm)");
            f2DPlotsHist[3 * detId + 2]->GetYaxis()->SetTitle("cluster y-position (mm)");
            f2DPlotsHist[3 * detId + 2]->SetLabelSize(0.04, "X");
            f2DPlotsHist[3 * detId + 2]->SetLabelSize(0.04, "Y");

            TString chSharing = detector + "_charge_Sharing";
            fChargeSharingHist[detId] = new TH2F(chSharing, chSharing, 50, fConfig.GetMinADCvalue(),
                                                 fConfig.GetMaxADCvalue(), 50, fConfig.GetMinADCvalue(),
                                                 fConfig.GetMaxADCvalue());
            fChargeSharingHist[detId]->GetXaxis()->SetTitle("ADC in X-strips");
            fChargeSharingHist[detId]->GetYaxis()->SetTitle("ADC in Y-strips");
            fChargeSharingHist[detId]->SetLabelSize(0.04, "X");
            fChargeSharingHist[detId]->SetLabelSize(0.04, "Y");

            TString chRatio = detector + "_charge_Ratio";
            fChargeRatioHist[detId] = new TH1F(chRatio, chRatio, 31, 0.0, 3.0);
            fChargeRatioHist[detId]->GetXaxis()->SetTitle("ADC charge ratio x / Y");
            fChargeRatioHist[detId]->GetYaxis()->SetTitle("ratio");
            fChargeRatioHist[detId]->SetLabelSize(0.04, "X");
            fChargeRatioHist[detId]->SetLabelSize(0.04, "Y");
            printf(" = GemView::InitHistForZeroSup() ==> detector[%s] to histId[%d]  \n", detector.Data(), detId);
        }
    }
    printf(" = GemView::InitHistForZeroSup() ==> Exit Init for zero sup analysis %s run\n", fRunType.Data());
}

//=====================================================
void GemView::ResetMultipleFilesHistos() {
    //  printf(" = GemView::ResetHistos() ==> Enter Init for zero sup analysis %s run\n",  fRunType.Data()) ;

    // PLANES
    map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
    map<TString, Int_t>::const_iterator plane_itr;
    for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
        TString plane = (*plane_itr).first;
        Int_t planeId = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane(plane)) + (*plane_itr).second;
        Int_t id = fMapping->GetNbOfPlane() + planeId;
        fHitHist[planeId]->Reset();
        fHitHist[id]->Reset();
        fClusterHist[planeId]->Reset();
        fClusterHist[id]->Reset();
        fClusterInfoHist[planeId]->Reset();
        fClusterInfoHist[id]->Reset();
        fADCHist[planeId]->Reset();
        fADCTimeBinPosHist[planeId]->Reset();
        fADCTimeBinPosHist[id]->Reset();
        fTimeBinPosHist[planeId]->Reset();
        fTimeBinPosHist[id]->Reset();
    }

    // DETECTORS
    map<Int_t, TString> listOfDetectors = fMapping->GetDetectorFromIDMap();
    map<Int_t, TString>::const_iterator det_itr;
    for (det_itr = listOfDetectors.begin(); det_itr != listOfDetectors.end(); ++det_itr) {
        Int_t detId = (*det_itr).first;
        TString detector = (*det_itr).second;
        f2DPlotsHist[3 * detId]->Reset();
        f2DPlotsHist[3 * detId + 1]->Reset();
        f2DPlotsHist[3 * detId + 2]->Reset();
        fChargeSharingHist[detId]->Reset();
        fChargeRatioHist[detId]->Reset();
    }
    //  printf(" = GemView::ResetHistos ==> Exit Init for zero sup analysis %s run\n",  fRunType.Data()) ;
}

//=====================================================
void GemView::InitHistForRawData() {
    printf(" = GemView::InitHistForRawData() ==> Enter Init for raw APV data analysis %s run\n", fRunType.Data());

    fRootWidget = new GemHistContainer *[fMapping->GetNbOfAPVs()];
    printf(" = GemView::InitHistForRawData() ==> fRootWidget is called\n");
    fFEC = new GemFEC *[fMapping->GetNbOfFECs()];
    printf(" = GemView::InitHistForRawData() ==> fFEC is called \n");

    int globalIndex = 0;
    for (int i = 0; i < fMapping->GetNbOfFECs(); i++) {
        list<int> apv_list = fMapping->GetAPVIDListFromFECID(i);
        fFEC[i] = new GemFEC(this, i, fMapping->GetNChannelEachFEC(i), &(apv_list));
        fFEC[i]->SetIPAddress(fMapping->GetFECIPFromFECID(i));
    }
    printf(" = GemView::InitHistForRawData() ==> fFEC is initialised \n");

    for (int i = 0; i < fMapping->GetNbOfAPVs(); i++) fRootWidget[i] = new GemHistContainer(this, i);

    for (int i = 0; i < fMapping->GetNbOfFECs(); i++) {
        for (int j = 0; j < fFEC[i]->GetNActiveChannel(); j++) {
            int currentAPVID = fFEC[i]->GetAPVIDFromThisFEC(j);
            TString currentPlane = (fMapping->GetPlaneFromAPVID(currentAPVID)).Data();
            fRootWidget[globalIndex]->Activate();
            Int_t adc_no = fMapping->GetAPVNoFromID(currentAPVID);
            fRootWidget[globalIndex]->RegisterHist(fFEC[i]->GetHist(adc_no));
            fRootWidget[globalIndex]->SetBasicInfo(currentPlane, i, globalIndex, adc_no);
            globalIndex++;
        }
    }
    printf(" = GemView::InitHistForRawData() ==> fRootWidget is initialised \n");
    printf(" = GemView::InitHistForRawData() ==> Exit Init for raw APV data analysis %s run\n", fRunType.Data());
}

//=====================================================
void GemView::fillRawDataHisto() {

    //fill raw data
    Clear();
    int fec_id = 0;
    int adc_ch = 0;
    map<int, map<int, vector<int> > >::iterator it;
    for (it = fHandler->fCurrentEvent.begin(); it != fHandler->fCurrentEvent.end(); ++it) {
        fec_id = it->first;
        map<int, vector<int> > temp = it->second;
        map<int, vector<int> >::iterator itt;
        for (itt = temp.begin(); itt != temp.end(); ++itt) {
            adc_ch = itt->first;
            vector<int> adc_temp = itt->second;
            Int_t apv_id = (fec_id << 4) | adc_ch;
            Int_t adc_no = fMapping->GetAPVNoFromID(apv_id);
            if (fFEC[fec_id]->IsChannelActive(adc_no) == 0) {
                continue;
            } else if (adc_temp.size() > 0) {
                fFEC[fec_id]->GetHist(adc_no)->SetBins(adc_temp.size(), 0, adc_temp.size());
            }
            int histMax = 0;
            int histMin = 1e6;
            for (unsigned int i = 0; i < adc_temp.size(); i++) {
                fFEC[fec_id]->GetHist(adc_no)->SetBinContent(i, adc_temp.at(i));
                if (adc_temp.at(i) < histMin) histMin = adc_temp.at(i);
                if (adc_temp.at(i) > histMax) histMax = adc_temp.at(i);
            }
            fFEC[fec_id]->GetHist(adc_no)->SetAxisRange(0.8 * histMin, 1.05 * histMax, "Y");
        }
    }
    for (int i = 0; i < fMapping->GetNbOfAPVs(); i++) fRootWidget[i]->drawHist();
    for (int i = 0; i < fMapping->GetNbOfFECs(); i++) fFEC[i]->DrawHist();
}


//===================================================== 
void GemView::DrawSingleEventDataHisto() {
    //  printf(" = GemView::fillSingleEventDataHisto() ==> Start \n") ;
    map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
    map<TString, Int_t>::const_iterator plane_itr;
    for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
        Int_t planeId =
                2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane((*plane_itr).first)) + (*plane_itr).second;
        Int_t id = fMapping->GetNbOfPlane() + planeId;
        Int_t nbOfStrips = 128 * fMapping->GetNbOfAPVsOnPlane((*plane_itr).first);
        DrawSingleEvent1DHistos(planeId, f1DSingleEventHist[planeId], nbOfStrips);
        DrawSingleEvent1DHistos(id, f1DSingleEventHist[id], nbOfStrips);
        DrawSingleEvent2DHistos(planeId, f2DSingleEventHist[planeId], "lego2", nbOfStrips);
        DrawSingleEvent2DHistos(id, f2DSingleEventHist[planeId], "colz", nbOfStrips);
    }
    //  printf(" = GemView::fillSingleEventDataHisto() ==> End \n") ;
}

//===============================================================================================
void GemView::DrawMultipleEventDataHisto() {
    printf(" = GemView::fillMultipleEventDataHisto() ==> Start \n");
    map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
    map<TString, Int_t>::const_iterator plane_itr;
    for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
        Int_t planeId =
                2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane((*plane_itr).first)) + (*plane_itr).second;
        Int_t id = fMapping->GetNbOfPlane() + planeId;
        DrawMultiEventsADCHistos(planeId, fADCHist[planeId]);
        DrawMultiEvents1DHistos(planeId, fHitHist[planeId], fClusterHist[planeId], fClusterInfoHist[planeId],
                                fADCTimeBinPosHist[planeId]);
        DrawMultiEvents1DHistos(id, fHitHist[id], fClusterHist[id], fClusterInfoHist[id], fADCTimeBinPosHist[id]);
    }
    map<Int_t, TString> listOfDetectors = fMapping->GetDetectorFromIDMap();
    map<Int_t, TString>::const_iterator det_itr;
    for (det_itr = listOfDetectors.begin(); det_itr != listOfDetectors.end(); ++det_itr) {
        Int_t detId = (*det_itr).first;
        DrawMultiEvents2DHistos(3 * detId, f2DPlotsHist[3 * detId]);
        DrawMultiEvents2DHistos(3 * detId + 1, f2DPlotsHist[3 * detId + 1]);
        DrawMultiEvents2DHistos(3 * detId + 2, f2DPlotsHist[3 * detId + 2]);
        DrawMultiEvents2DHistos(detId, fChargeSharingHist[detId], fChargeRatioHist[detId]);
    }
    printf(" = GemView::fillMultipleEventDataHisto() ==> End \n");
}

// TODO
////=====================================================
//int GemView::ProcessSingleEvent(evioDOMNodeP bankPtr, vector <SFclust> &gemclust ) {
//  int status = 0;
//  map <TString, Int_t>  listOfPlanes  = fMapping-> GetPlaneIDFromPlaneMap() ;
//
//  if( fRunType == "SINGLEEVENT")   status = fHandler->ProcessSingleEventFromBank(f1DSingleEventHist, f2DSingleEventHist, bankPtr, gemclust);
//  if (status == 1) {
//    if( fRunType == "SINGLEEVENT")   DrawSingleEventDataHisto() ;
//    else if ( fRunType == "RAWDATA") fillRawDataHisto();
//    return 1;
//  }
//  else{
//    printf("Error reading event %d from file %d \n", fCurrentEntry);
//    return 0;
//  }
//}
//=====================================================
int GemView::GemMonitorLoop() {
    int status = 0;
    map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
    if (fConfig.GetOnlineMode())
        status = fHandler->ProcessETEvent(f1DSingleEventHist, f2DSingleEventHist);
    else if (fRunType == "SINGLEEVENT")
        status = fHandler->ProcessSingleEventFromFile(fConfig.GetInputFileName(), fCurrentEntry, f1DSingleEventHist,
                                                      f2DSingleEventHist);
    else if (fRunType == "RAWDATA")
        status = fHandler->ProcessRawDataFromFile(fConfig.GetInputFileName(), fCurrentEntry);
    //cout << "GemView::GemMonitorLoop() fCurrentEntry=" << fCurrentEntry << endl;
    if (status == 1) {
        if (fRunType == "SINGLEEVENT") DrawSingleEventDataHisto();
        else if (fRunType == "RAWDATA") fillRawDataHisto();
        return 1;
    } else {
        printf("Error reading event %d from file %d \n", fCurrentEntry);
        return 0;
    }
}

//=====================================================
int GemView::AutoGemMonitorLoop() {
    int status = 0;
    if (fConfig.GetOnlineMode() && fHandler->IsEventFound()) {
        status = fHandler->ProcessETEvent(f1DSingleEventHist, f2DSingleEventHist);
    }
    if (status == 1) {
        if (fRunType == "SINGLEEVENT") DrawSingleEventDataHisto();
        else fillRawDataHisto();
        fHandler->Restart();
        return 1;
    }
    if (!fConfig.GetOnlineMode()) {
        status = fHandler->ProcessSingleEventFromFile(fConfig.GetInputFileName(), fCurrentEntry, f1DSingleEventHist,
                                                      f2DSingleEventHist);
        fCurrentEntry++;
    }
    if (status == 1) return 1;
    return 0;
}


//=====================================================
void GemView::SetMonitor(int sec) {
    assert(sec > 0);
}

//=====================================================_
void GemView::SpinCurrentEvent(int i) {
    fCurrentEntry = i;
    //cout << "GemView::SpinCurrentEvent() i=" << i << endl;
    GemMonitorLoop();
}


//=====================================================
void GemView::WriteRootFile(TString outputRootFile) {
    printf(" = GemView::WriteRootFile() ==> Write Analysis Histograms in root files \n");

    printf("Writing all current histograms to root file %s", outputRootFile.Data());
    TFile *rootFile = new TFile(outputRootFile.Data(), "RECREATE");
    rootFile->cd();

    map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
    map<TString, Int_t>::const_iterator plane_itr;
    for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
        Int_t planeId =
                2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane((*plane_itr).first)) + (*plane_itr).second;
        Int_t id = fMapping->GetNbOfPlane() + planeId;

        fADCTimeBinPosHist[planeId]->Divide(fTimeBinPosHist[planeId]);
        fADCHist[planeId]->Write();
        fHitHist[planeId]->Write();
        fClusterHist[planeId]->Write();
        fClusterInfoHist[planeId]->Write();
        fTimeBinPosHist[planeId]->Write();
        fADCTimeBinPosHist[planeId]->Write();

        fADCTimeBinPosHist[id]->Divide(fTimeBinPosHist[planeId]);
        fHitHist[id]->Write();
        fClusterHist[id]->Write();
        fClusterInfoHist[id]->Write();
        fTimeBinPosHist[id]->Write();
        fADCTimeBinPosHist[id]->Write();
    }

    map<Int_t, TString> listOfDetectors = fMapping->GetDetectorFromIDMap();
    map<Int_t, TString>::const_iterator det_itr;
    for (det_itr = listOfDetectors.begin(); det_itr != listOfDetectors.end(); ++det_itr) {
        Int_t detId = (*det_itr).first;

        Int_t nbOfBinsX = f2DPlotsHist[3 * detId + 1]->GetNbinsX();
        Int_t nbOfBinsY = f2DPlotsHist[3 * detId + 1]->GetNbinsY();
        for (Int_t ii = 0; ii < nbOfBinsX; ii++) {
            for (Int_t jj = 0; jj < nbOfBinsY; jj++) {
                Int_t binx = ii + 1;
                Int_t biny = jj + 1;
                Int_t binContent = ((Int_t) f2DPlotsHist[3 * detId]->GetBinContent(binx, biny));
                if (binContent < 1) {
                    f2DPlotsHist[3 * detId]->SetBinContent(binx, biny, 0);
                    f2DPlotsHist[3 * detId + 1]->SetBinContent(binx, biny, 0);
                    f2DPlotsHist[3 * detId + 2]->SetBinContent(binx, biny, 0);
                }
            }
        }

        f2DPlotsHist[3 * detId + 1]->Divide(f2DPlotsHist[3 * detId]);
        f2DPlotsHist[3 * detId + 2]->Divide(f2DPlotsHist[3 * detId]);

        Float_t norm = (f2DPlotsHist[3 * detId + 1]->GetNbinsX() * f2DPlotsHist[3 * detId + 1]->GetNbinsY()) /
                       f2DPlotsHist[3 * detId + 1]->Integral();
        TString detector = (*det_itr).second;
        if (fMapping->GetReadoutBoardFromDetector(detector) == "UV_ANGLE") {
            norm = 0.75 * norm;
        }

        /**
        f2DPlotsHist[3*detId+1]->Scale(norm) ;
        f2DPlotsHist[3*detId+1]->SetMaximum(1.5);
        f2DPlotsHist[3*detId+1]->SetMinimum(0.5);
        */

        f2DPlotsHist[3 * detId]->Write();
        f2DPlotsHist[3 * detId + 1]->Write();
        f2DPlotsHist[3 * detId + 2]->Write();
        fChargeSharingHist[detId]->Write();
        fChargeRatioHist[detId]->Write();
    }
    rootFile->Close();
    delete rootFile;
}

//==================================================
void GemView::AnalyzeMultiEvents() {
    /*
    bool ok;
    unsigned int minEntry = 0, maxEntry = 5000;
    unsigned int minEntry = (unsigned int) (fConfig.GetFirstEvent()) ;
    unsigned int maxEntry = (unsigned int) (fConfig.GetLastEvent());
    QString eventRange;
    fProgressBar->reset();
    fProgressBar->setVisible(true);
    fProgressBar->show();
    eventRange = QInputDialog::getText(fMainWidget, tr("Event range (minEntry~maxEntry)"), tr("Event range: "), QLineEdit::Normal,"", &ok);   if (ok && !eventRange.isEmpty()) {
      QStringList list = eventRange.split("~");
      if (list.size() != 2) {
       printf(" = GemView::AnalyzeFile() => WARNING !!! Cannot understand the input formal, has to be minEntry~maxEntry. i.e 0~1000 \n") ;
       return;
      }
      if ((list.at(0)).toInt(&ok, 10) < 0 || (list.at(1)).toInt(&ok, 10) < 0){
        cout<<"both entry has to be larger than 0"<<endl;
        return;
      }
      minEntry = (unsigned int)(list.at(0)).toInt(&ok, 10);
      maxEntry = (unsigned int)(list.at(1)).toInt(&ok, 10);
        if (!ok) return;
      if (maxEntry<=minEntry){
       printf(" = GemView::AnalyzeFile() => WARNING !!! max entry has to be larger than min entry \n") ;
        return;
      }
    }
    */

    fHandler->ProcessMultiEventsFromFile(fConfig.GetInputFileName(), fConfig.GetFirstEvent(), fConfig.GetLastEvent(),
                                         fADCHist, fHitHist, fClusterHist, fClusterInfoHist, f2DPlotsHist,
                                         fChargeSharingHist, fChargeRatioHist, fTimeBinPosHist, fADCTimeBinPosHist);
    DrawMultipleEventDataHisto();

    //  TString rootFileName = fConfig.GetOutputFileName() ;
    //  WriteRootFile(rootFileName.Data()) ;
    WriteRootFile(fRootFileName.Data());
}

//==================================================
void GemView::AnalyzeMultipleFiles(TString inputDataFile, TString outputRootFile) {
    fHandler->ProcessMultiEventsFromFile(inputDataFile.Data(), fConfig.GetFirstEvent(), fConfig.GetLastEvent(),
                                         fADCHist, fHitHist, fClusterHist, fClusterInfoHist, f2DPlotsHist,
                                         fChargeSharingHist, fChargeRatioHist, fTimeBinPosHist, fADCTimeBinPosHist);
    WriteRootFile(outputRootFile.Data());
    ResetMultipleFilesHistos();
}

//==================================================
void GemView::SearchEvents() {
    //  printf(" = GemView::SearchEvents() \n") ;
    fHandler->ProcessSearchEventFromFile(fConfig.GetInputFileName(), f1DSingleEventHist, f2DSingleEventHist);
    DrawSingleEventDataHisto();
}

//=================================
void GemView::ProducePedestals() {
    if (fRunType != "PEDESTAL") {
        printf(" = GemView::ProducePedestals() => WARNING !!! Not a pedestal run \n");
        return;
    }
    fHandler->ProcessPedestals(fConfig.GetInputFileName(), fConfig.GetFirstEvent(), fConfig.GetLastEvent());
}

//======================================================================

//===============================================================================================================
void GemView::DrawMultiEventsADCHistos(int id, TH1F *adcHist) {
    //  printf(" = GemView::DrawMultiEvents1DHistos() ==> Enter \n") ;
    gStyle->SetOptStat(1111);
    adcHist->Draw();
}

//===============================================================================================================
void GemView::DrawMultiEvents1DHistos(int id, TH1F *hitHist, TH1F *clusterHist, TH1F *clusterInfoHist,
                                      TH2F *adcTimeBinPosHist) {
    //  printf(" = GemView::DrawMultiEvents1DHistos() ==> Enter \n") ;
    gStyle->SetOptStat(1111);

    hitHist->Draw();

    clusterHist->Draw();

    clusterInfoHist->Draw();

    gStyle->SetOptStat(0);
    adcTimeBinPosHist->Draw("colz");
}

//================================================================================================================
void GemView::DrawSingleEvent1DHistos(int id, TH1F *hist, Int_t nbOfStrips, Float_t size) {
    //  printf(" = GemView::DrawSingleEvent1DHistos() ==> Enter \n") ;
    hist->SetBins(nbOfStrips, -1 * size / 2.0, size / 2.0);
    hist->Draw();
}

//================================================================================================================
void GemView::DrawSingleEvent1DHistos(int id, TH1F *hist, Int_t nbOfStrips) {
    //  printf(" = GemView::DrawSingleEvent1DHistos() ==> Enter \n") ;
    Int_t nbBins = nbOfStrips;
    //  Int_t nbBins = nbOfStrips*fNbOfTimeSamples ;
    hist->SetBins(nbBins, 0, (nbBins - 1));
    hist->Draw();
}

//================================================================================================================
void GemView::DrawSingleEvent2DHistos(int id, TH2F *hist2d, TString plotStyle, Float_t size, Int_t nbOfStrips) {
    Int_t driftTime = (fNbOfTimeSamples - 1) * 25;
    hist2d->SetBins(fNbOfTimeSamples, 0, driftTime, nbOfStrips, -1 * size / 2.0, size / 2.0);
    hist2d->Draw(plotStyle);
}

//================================================================================================================
void GemView::DrawSingleEvent2DHistos(int id, TH2F *hist2d, TString plotStyle, Int_t nbOfStrips) {
    Int_t driftTime = (fNbOfTimeSamples - 1) * 25;
    hist2d->SetBins(fNbOfTimeSamples, 0, driftTime, nbOfStrips, 0, (nbOfStrips - 1));
    hist2d->Draw(plotStyle);
}

//================================================================================================================
void GemView::DrawMultiEvents2DHistos(int id, TH2F *pos2DHist) {
    pos2DHist->Draw("colz");
}

//================================================================================================================
void GemView::DrawMultiEvents2DHistos(int id, TH2F *chargeSharingHist, TH1F *chRatioHist) {
    chargeSharingHist->Draw("colz");

    chRatioHist->Draw();
}

//====================================================================
