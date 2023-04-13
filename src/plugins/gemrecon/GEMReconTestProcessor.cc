#include "GEMReconTestProcessor.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "rawdataparser/Df125WindowRawData.h"
#include "rawdataparser/Df125FDCPulse.h"
#include "GEMOnlineHitDecoder.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include <services/root_output/RootFile_service.h>
#include <filesystem>

inline static void FillTrdHistogram(uint64_t event_number,
                      TDirectory *hists_dir,
                      std::vector<const Df125WindowRawData *> f125_records,
                      int slot_shift,
                      int zero_fill,
                      int max_x = 250,
                      int max_y = 300
                      ) {

    // Crate histogram
    std::string histo_name = fmt::format("trd_event_{}", event_number);
    std::string histo_title = fmt::format("TRD event #{} by F125WindowRawData", event_number);
    auto histo= new TH2F(histo_name.c_str(), histo_title.c_str(), max_x, -0.5, max_x - 0.5, max_y, -0.5, max_y - 0.5);
    histo->SetDirectory(hists_dir);

    // f125_records has data for only non-zero channels
    // But we want to fill all channels possible channels with fill_value
    // So we create an event_table that we fill first, and then fill histogram by its values
    float event_table[max_x][max_y];
    // Fill histogram
    for(size_t x_i=0; x_i<max_x; x_i++) {
        for(size_t y_i=0; y_i<max_y; y_i++) {
            event_table[x_i][y_i] = zero_fill;
        }
    }

    // Fill data into event_table
    for (auto record: f125_records) {
        int x = 72 * (record->slot - slot_shift) + record->channel;
        for (int sample_i = 0; sample_i < record->samples.size(); sample_i++) {
            if(x < max_x && sample_i < max_y) {
                float sample = record->samples[sample_i];
                if(sample < zero_fill) sample = zero_fill;  // Fill 0 values with zero_fill
                event_table[x][sample_i] = sample;
            }
        }
    }

    // Fill histogram
    for(size_t x_i=0; x_i<max_x; x_i++) {
        for(size_t y_i=0; y_i<max_y; y_i++) {
            spdlog::info("    event_table[{}][{}]={}", x_i, y_i, event_table[x_i][y_i]);
            histo->Fill(x_i, y_i, event_table[x_i][y_i]);
        }
    }

    // Save histogram
    histo->Write();
}


//------------------
// OccupancyAnalysis (Constructor)
//------------------
GEMReconTestProcessor::GEMReconTestProcessor(JApplication *app) :
        JEventProcessor(app) {
}

//------------------
// Init
//------------------
void GEMReconTestProcessor::Init() {
    std::string plugin_name = GetPluginName();

    // Get JANA application
    auto app = GetApplication();

    // Ask service locator a file to write histograms to
    auto root_file_service = app->GetService<RootFile_service>();

    // Get TDirectory for histograms root file
    auto globalRootLock = app->GetService<JGlobalRootLock>();
    globalRootLock->acquire_write_lock();
    auto file = root_file_service->GetHistFile();
    globalRootLock->release_lock();

    // Create a directory for this plugin. And subdirectories for series of histograms
    m_dir_main = file->mkdir(plugin_name.c_str());
    m_dir_event_hists = m_dir_main->mkdir("trd_events", "TRD events visualization");
    m_dir_main->cd();

    // Get Log level from user parameter or default
    InitLogger(plugin_name);

    m_histo_1d = new TH1F("test_histo", "Test histogram", 100, -10, 10);
    m_trd_integral_h2d = new TH2F("trd_integral_events", "TRD events from Df125WindowRawData integral", 250, -0.5, 249.5, 300, -0.5, 299.5);

    std::string gem_config = "Config.cfg";
    app->SetDefaultParameter(plugin_name + ":config", gem_config, "Full path to gem config");
    logger()->info("Configuration file: {}", gem_config);

    try {
        fConfig.Load(gem_config);
    }
    catch(std::iostream::failure &ex) {
        logger()->error("std::iostream::failure error opening '{}': {}", gem_config, ex.what());
        throw;
    }
    catch (std::exception& ex) {
        logger()->error("Error opening '{}': {}", gem_config, ex.what());
        throw;
    }

    std::filesystem::path pedestals_file_name = fConfig.GetPedFileName();

    std::filesystem::path relative_file_path(gem_config);
    // Get the parent directory
    std::filesystem::path parent_directory = relative_file_path.parent_path();
    std::filesystem::path pedestals_abs_path = std::filesystem::absolute(parent_directory / pedestals_file_name);
    logger()->info("Pedestals absolute path: {}", pedestals_abs_path.string());


    logger()->info("Creating pedestals: {}", fConfig.GetPedFileName());
    fPedestal = new GEMPedestal(pedestals_abs_path.c_str(), fConfig.GetNbOfTimeSamples());
    fPedestal->BookHistos();

    //fHandler->InitPedestal(fPedestal);
    //printf(" = GemView::InitConfig() ==> Initialisation for a %s Run \n", fRunType.Data());

    logger()->info("This plugin name is: " + GetPluginName());
    logger()->info("GEMReconTestProcessor initialization is done");

    std::string mapping_file = "Config.cfg";
    app->SetDefaultParameter(plugin_name + ":mapping", mapping_file, "Full path to gem config");
    logger()->info("Mapping file file: {}", mapping_file);

    fMapping = GemMapping::GetInstance();
    fMapping->LoadMapping(mapping_file.c_str());
    fMapping->PrintMapping();

    InitHistForZeroSup();
}


//=====================================================
void GEMReconTestProcessor::InitHistForZeroSup() {

    std::string fRunType("SINGLEEVENT");
    int fNbOfTimeSamples = fConfig.GetNbOfTimeSamples();

    Int_t nbOfPlaneHists = 2 * fMapping->GetNbOfPlane();
    Int_t driftTime = 25 * fNbOfTimeSamples;
    int fNbADCBins = 50;

    // PLANES
    if ((fRunType == "SINGLEEVENT") || (fRunType == "SEARCHEVENTS")) {
        f1DSingleEventHist = new TH1F *[nbOfPlaneHists];
        f2DSingleEventHist = new TH2F *[nbOfPlaneHists];

        std::map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
        std::map<TString, Int_t>::const_iterator plane_itr;
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

        std::map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
        std::map<TString, Int_t>::const_iterator plane_itr;
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

        std::map<Int_t, TString> listOfDetectors = fMapping->GetDetectorFromIDMap();
        std::map<Int_t, TString>::const_iterator det_itr;
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
    logger()->info(" = GemView::InitHistForZeroSup() ==> Exit Init for zero sup analysis %s run\n", fRunType);
}


//------------------
// Process
//------------------
// This function is called every event
void GEMReconTestProcessor::Process(const std::shared_ptr<const JEvent> &event) {
    m_log->debug("new event");
    try {
        auto srs_data = event->Get<DGEMSRSWindowRawData>();
        m_log->trace("DGEMSRSWindowRawData data items: {} ", srs_data.size());

        logger()->trace("  [rocid] [slot]  [channel] [apv_id] [channel_apv] [samples size:val1,val2,...]");
        for(auto srs_item: srs_data) {
            std::string row = fmt::format("{:<7} {:<7} {:<9} {:<8} {:<13} {:<2}: ",
                                          srs_item->rocid, srs_item->slot, srs_item->channel,
                                          srs_item->apv_id, srs_item->channel_apv, srs_item->samples.size());

            for(auto sample: srs_item->samples) {
                row+=fmt::format(" {}", sample);
            }

            logger()->info("   {}", row);
        }

        std::map<int, std::map<int, std::vector<int> > > fCurrentEvent;

        for(auto srs_item: srs_data) {

            // Dumb copy of samples because one is int and the other is uint16_t
            std::vector<int> samples;
            for(size_t i=0; i<srs_data.size(); i++) {
                samples.push_back(srs_item->samples[i]);
            }

            fCurrentEvent[(int)srs_item->apv_id][(int)srs_item->channel_apv] = samples;
        }

        unsigned int fSrsStart, fSrsEnd;
        //  float   fOfflineProgress;
        TString fIsHitPeakOrSumADCs, fIsCentralOrAllStripsADCs;
        Float_t fMinADCvalue;
        int fEventNumber, fLastEvent, fZeroSupCut, fComModeCut, fNbOfTimeSamples, fStopTimeSample, fStartTimeSample;
        int fMinClusterSize, fMaxClusterSize, fMaxClusterMult;



        fSrsStart = 0xda000022;
        fSrsEnd = 0xda0000ff;
        auto fMapping = GemMapping::GetInstance();
        fEventNumber = -1;
        fLastEvent = 0;
        fMinADCvalue = 50;
        fZeroSupCut = 10;
        fComModeCut = 20;
        fNbOfTimeSamples = 9;
        fStartTimeSample = 2;
        fStopTimeSample = 7;
        fMinClusterSize = 2;
        fMaxClusterSize = 20;
        fMaxClusterMult = 5;
        fIsHitPeakOrSumADCs = "peakADCs";
        fIsCentralOrAllStripsADCs = "centralStripADCs";


        GEMOnlineHitDecoder hit_decoder(fEventNumber, fNbOfTimeSamples, fStartTimeSample, fStopTimeSample,
                                        fZeroSupCut, fComModeCut, fIsHitPeakOrSumADCs, fIsCentralOrAllStripsADCs,
                                        fMinADCvalue, fMinClusterSize, fMaxClusterSize, fMaxClusterMult);
        hit_decoder.ProcessEvent(fCurrentEvent, fPedestal);

        TH1F **hist = f1DSingleEventHist;
        TH2F **hist2d = f2DSingleEventHist;

        hit_decoder.ProcessEvent(fCurrentEvent, fPedestal);
        std::map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
        std::map<TString, Int_t>::const_iterator plane_itr;
        int pad = 0;
        int ncx = 0, ncy = 0;
        std::vector<SFclust> clustX, clustY;
        for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
            TString plane = (*plane_itr).first;
            Int_t i = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane((*plane_itr).first)) + (*plane_itr).second;
            Int_t j = fMapping->GetNbOfPlane() + i;
            // all channels hit
            hist[j]->Reset();
            hit_decoder.GetHit((*plane_itr).first, hist[j]);
            // Zero sup hit
            hist[i]->Reset();
            hit_decoder.GetClusterHit((*plane_itr).first, hist[i]);
            hist2d[i]->Reset();
            hit_decoder.GetTimeBinClusterHit((*plane_itr).first, hist2d[i]);

            if (plane == "GEM1X") {
                ncx = hit_decoder.GetClusters((*plane_itr).first, clustX);
            }
            if (plane == "GEM1Y") {
                ncy = hit_decoder.GetClusters((*plane_itr).first, clustY);
            }
        }
    }
    catch (std::exception &exp) {
        m_log->trace("Got exception when doing event->Get<Df125WindowRawData>()");
        m_log->trace("Exception what()='{}', type='{}'", exp.what(), typeid(exp).name());
    }
}


//------------------
// Finish
//------------------
void GEMReconTestProcessor::Finish() {
//    m_log->trace("GEMReconTestProcessor finished\n");

}




