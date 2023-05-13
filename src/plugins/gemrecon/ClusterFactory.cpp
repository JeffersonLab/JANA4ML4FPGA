#include "ClusterFactory.h"

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
#include "Pedestal.h"

namespace ml4fpga::gem {

//-------------------------------------
// ClusterFactory (Constructor)
//-------------------------------------
ClusterFactory::ClusterFactory(JApplication *app)
        {
    m_app = app;
}

//-------------------------------------
// Init
//-------------------------------------
void ClusterFactory::Init() {

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

    auto dir_main_tobj = file->FindObjectAny(plugin_name.c_str());
    m_dir_main = dir_main_tobj? (TDirectory*) dir_main_tobj : file->mkdir(plugin_name.c_str());
    // m_dir_event_hists = m_dir_main->mkdir("gem_events", "TRD events visualization");
    m_dir_main->cd();

    // Get Log level from user parameter or default
    InitLogger(plugin_name + ":ClusterF");

    m_histo_1d = new TH1F("test_histo", "Test histogram", 100, -10, 10);
    m_trd_integral_h2d = new TH2F("trd_integral_events", "TRD events from Df125WindowRawData integral", 250, -0.5, 249.5, 300, -0.5, 299.5);

    // P A R A M E T E R S
    // Number of SRS time samples:
    app->SetDefaultParameter("daq:srs_window_raw:ntsamples", m_srs_ntsamples, "Number of SRS time samples");
    app->SetDefaultParameter( plugin_name + ":min_adc", m_min_adc, "Min ADC value (For histos?)");
    app->SetDefaultParameter( plugin_name + ":max_adc", m_max_adc, "Max ADC value (For histos?)");


    // I N I T   M A P P I N G
    std::string mapping_file = "mapping.cfg";
    app->SetDefaultParameter(plugin_name + ":mapping", mapping_file, "Full path to gem config");
    logger()->info("Mapping file file: {}", mapping_file);

    fMapping = GemMapping::GetInstance();
    fMapping->LoadMapping(mapping_file.c_str());
    fMapping->PrintMapping();

        auto plane_map = fMapping->GetAPVIDListFromPlaneMap();
        logger()->info("MAPPING EXPLAINED:");
        for(auto pair: plane_map) {
            auto name = pair.first;
            auto det_name = fMapping->GetDetectorFromPlane(name);
            auto apv_list = pair.second;
            m_log->info("  Plane: {:<10} from detector {:<10} has {} APVs:", name, det_name, apv_list.size());
            for(auto apv_id: apv_list) {
                m_log->info("    {}", apv_id);
            }
        }

    // I N I T   H I S T O G R A M S
    InitHistForZeroSup();

    //  D O N E
    logger()->info("This plugin name is: " + GetPluginName());
    logger()->info("ClusterFactory initialization is done");
}


//=====================================================
void ClusterFactory::InitHistForZeroSup() {

    std::string fRunType("SINGLEEVENT");

    Int_t nbOfPlaneHists = 2 * fMapping->GetNbOfPlane();
    Int_t driftTime = 25 * m_srs_ntsamples;
    int fNbADCBins = 50;

    // PLANES
    if ((fRunType == "SINGLEEVENT") || (fRunType == "SEARCHEVENTS")) {
        f1DSingleEventHist = new TH1F *[nbOfPlaneHists];
        f2DSingleEventHist = new TH2F *[nbOfPlaneHists];

        std::map<std::string, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
        std::map<std::string, Int_t>::const_iterator plane_itr;
        for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
            std::string plane = (*plane_itr).first;
            Int_t planeId = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane(plane)) + (*plane_itr).second;
            Int_t nbOfStrips = 128 * fMapping->GetNbOfAPVsOnPlane((*plane_itr).first);
            Float_t size = fMapping->GetPlaneSize((*plane_itr).first);
            Int_t id = fMapping->GetNbOfPlane() + planeId;

            Int_t nbBins = nbOfStrips * m_srs_ntsamples;

            TString zeroSup = (*plane_itr).first + "_zeroSup";
            f1DSingleEventHist[planeId] = new TH1F(zeroSup, zeroSup, nbBins, 0, (nbBins - 1));
            f1DSingleEventHist[planeId]->SetLabelSize(0.04, "X");
            f1DSingleEventHist[planeId]->SetLabelSize(0.04, "Y");

            TString presZeroSup = (*plane_itr).first + "_pedSub";
            f1DSingleEventHist[id] = new TH1F(presZeroSup, presZeroSup, nbBins, 0, (nbBins - 1));
            f1DSingleEventHist[id]->SetLabelSize(0.04, "X");
            f1DSingleEventHist[id]->SetLabelSize(0.04, "Y");

            zeroSup = (*plane_itr).first + "_SingleHit_vs_timebin_2D";
            f2DSingleEventHist[planeId] = new TH2F(zeroSup, zeroSup, m_srs_ntsamples, 0, driftTime, nbOfStrips, 0,
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
                   plane.c_str(), nbOfStrips, m_srs_ntsamples, size, planeId);

            presZeroSup = (*plane_itr).first + "Accu_Hits_vs_timebin_2D";
            f2DSingleEventHist[id] = new TH2F(presZeroSup, presZeroSup, m_srs_ntsamples, 0, driftTime, nbOfStrips, 0,
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
                   plane.c_str(), nbOfStrips, m_srs_ntsamples, size, id);
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

        std::map<std::string, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
        std::map<std::string, Int_t>::const_iterator plane_itr;
        for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
            auto plane = (*plane_itr).first;
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
            fADCHist[planeId] = new TH1F(adcDist, adcDist, fNbADCBins, m_min_adc, m_max_adc);
            fADCHist[planeId]->GetXaxis()->SetTitle("ADC");
            fADCHist[planeId]->GetYaxis()->SetTitle("Counts");
            fADCHist[planeId]->SetLabelSize(0.04, "X");
            fADCHist[planeId]->SetLabelSize(0.04, "Y");

            Int_t tsbin = (Int_t) (nbOfStrips / 8);

            TString alltimeBin = (*plane_itr).first + "adc_vs_pos_allTimeBin";
            fADCTimeBinPosHist[planeId] = new TH2F(alltimeBin, alltimeBin, m_srs_ntsamples, 0, driftTime, tsbin, 0,
                                                   (nbOfStrips - 1));
            fADCTimeBinPosHist[planeId]->GetXaxis()->SetTitle("drift time");
            fADCTimeBinPosHist[planeId]->GetYaxis()->SetTitle("Position (mm)");
            fADCTimeBinPosHist[planeId]->SetLabelSize(0.04, "X");
            fADCTimeBinPosHist[planeId]->SetLabelSize(0.04, "Y");

            TString timeBinPeak = (*plane_itr).first + "adc_vs_pos_timeBinPeak";
            fADCTimeBinPosHist[id] = new TH2F(timeBinPeak, timeBinPeak, m_srs_ntsamples, 0, driftTime, tsbin, 0,
                                              (nbOfStrips - 1));
            fADCTimeBinPosHist[id]->GetXaxis()->SetTitle("drift time");
            fADCTimeBinPosHist[id]->GetYaxis()->SetTitle("Position (mm)");
            fADCTimeBinPosHist[id]->SetLabelSize(0.04, "X");
            fADCTimeBinPosHist[id]->SetLabelSize(0.04, "Y");

            alltimeBin = (*plane_itr).first + "_pos_vs_allTimeBin";
            fTimeBinPosHist[planeId] = new TH2F(alltimeBin, alltimeBin, m_srs_ntsamples, 0, driftTime, tsbin, 0,
                                                (nbOfStrips - 1));
            fTimeBinPosHist[planeId]->GetXaxis()->SetTitle("drift time");
            fTimeBinPosHist[planeId]->GetYaxis()->SetTitle("Position (mm)");
            fTimeBinPosHist[planeId]->SetLabelSize(0.04, "X");
            fTimeBinPosHist[planeId]->SetLabelSize(0.04, "Y");

            timeBinPeak = (*plane_itr).first + "_pos_vs_timeBinPeak";
            fTimeBinPosHist[id] = new TH2F(timeBinPeak, timeBinPeak, m_srs_ntsamples, 0, driftTime, tsbin, 0,
                                           (nbOfStrips - 1));
            fTimeBinPosHist[id]->GetXaxis()->SetTitle("drift time");
            fTimeBinPosHist[id]->GetYaxis()->SetTitle("Position (mm)");
            fTimeBinPosHist[id]->SetLabelSize(0.04, "X");
            fTimeBinPosHist[id]->SetLabelSize(0.04, "Y");
            printf(" = GemView::InitHistForZeroSup() ==> plane[%s] to histId[%d]  \n", plane.c_str(), planeId);
        }

        // DETECTORS
        f2DPlotsHist = new TH2F *[3 * fMapping->GetNbOfDetectors()];
        fChargeSharingHist = new TH2F *[fMapping->GetNbOfDetectors()];
        fChargeRatioHist = new TH1F *[fMapping->GetNbOfDetectors()];

        std::map<Int_t, std::string> listOfDetectors = fMapping->GetDetectorFromIDMap();
        std::map<Int_t, std::string>::const_iterator det_itr;
        for (det_itr = listOfDetectors.begin(); det_itr != listOfDetectors.end(); ++det_itr) {
            Int_t detId = (*det_itr).first;
            std::string detector = (*det_itr).second;

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
            fChargeSharingHist[detId] = new TH2F(chSharing, chSharing, 50, m_min_adc, m_max_adc, 50, m_min_adc, m_max_adc);
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
            printf(" = GemView::InitHistForZeroSup() ==> detector[%s] to histId[%d]  \n", detector.c_str(), detId);
        }
    }
    logger()->info(" = GemView::InitHistForZeroSup() ==> Exit Init for zero sup analysis %s run\n", fRunType);
}


//------------------
// Process
//------------------
// This function is called every event
void ClusterFactory::Process(const std::shared_ptr<const JEvent> &event) {
    m_log->debug("new event");
    try {
        auto srs_data = event->Get<DGEMSRSWindowRawData>();
        auto pedestal = const_cast<Pedestal*>(event->GetSingle<ml4fpga::gem::Pedestal>());  // we are not going to modify it, just remove hassle with const map
        m_log->trace("DGEMSRSWindowRawData data items: {} ", srs_data.size());

        //TraceDumpSrsData(srs_data, /*num rows*/ 256);
        //TraceDumpMapping(srs_data, /*num rows*/ 256);

        std::map<int, std::map<int, std::vector<int> > > apvid_chan_sampls;

        for(auto srs_item: srs_data) {

            // Dumb copy of samples because one is int and the other is uint16_t
            std::vector<int> samples;
            for(unsigned short sample : srs_item->samples) {
                samples.push_back(sample);
            }

            apvid_chan_sampls[(int)srs_item->apv_id][(int)srs_item->channel_apv] = samples;
            fMapping->APVchannelCorrection(srs_item->channel_apv);
        }

        // Now lets go over this
        // fec, apv, raw_data
        std::map<int, std::map<int, std::vector<int> > > fec_apv_raw;
        for(auto apv_pair: apvid_chan_sampls) {
            auto apv_id = apv_pair.first;
            auto apv_channels = apv_pair.second;

            // Crate histogram
            size_t samples_size = apv_channels[0].size();
            size_t raw_data_len = samples_size*128;
            std::vector<int> raw_data(raw_data_len, 0);

            // go over channels
            for(auto channel_pair: apv_channels) {
                auto apv_channel = channel_pair.first;
                auto samples = channel_pair.second;
                // go over samples
                for(size_t sample_i=0; sample_i < samples.size(); sample_i++){
                    int data_index = sample_i*128 + apv_channel;  // + sample_i to make gaps in between samples
                    raw_data[data_index] = samples[sample_i];
                }
            }

            // Fec is always 0,
            fec_apv_raw[0][apv_id] = raw_data;
        }

        // this->FillTrdHistogram(event->GetEventNumber(), m_dir_main, srs_data);

        GEMOnlineHitDecoder hit_decoder(event->GetEventNumber(), m_srs_ntsamples, fStartTimeSample, m_srs_ntsamples-1,
                                        fZeroSupCut, fComModeCut, fIsHitPeakOrSumADCs, fIsCentralOrAllStripsADCs,
                                        m_min_adc, fMinClusterSize, fMaxClusterSize, fMaxClusterMult);
//        hit_decoder.ProcessEvent(fec_apv_raw, fPedestal);
//
        TH1F **hist = f1DSingleEventHist;
        TH2F **hist2d = f2DSingleEventHist;


        hit_decoder.ProcessEvent(fec_apv_raw, pedestal->offsets, pedestal->noises);
        std::map<std::string, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
        std::map<std::string, Int_t>::const_iterator plane_itr;
        int pad = 0;
        int ncx = 0, ncy = 0;
        std::vector<SFclust> clustX, clustY;
        for (auto plane_itr: listOfPlanes) {

            std::string plane_name(plane_itr.first);
            int plane_id = plane_itr.second;
            int i = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane(plane_name)) + plane_id;
            int j = fMapping->GetNbOfPlane() + i;
            // all channels hit
            hist[j]->Reset();
            hit_decoder.GetHit(plane_name, hist[j]);
            // Zero sup hit
            hist[i]->Reset();
            hit_decoder.GetClusterHit(plane_name, hist[i]);
            hist2d[i]->Reset();
            hit_decoder.GetTimeBinClusterHit(plane_name, hist2d[i]);

            if (plane_name == "URWELLX") {
                ncx = hit_decoder.GetClusters(plane_name, clustX);
            }
            if (plane_name == "URWELLY") {
                ncy = hit_decoder.GetClusters(plane_name, clustY);
            }
        }
        std::vector<SFclust*> result_clusters;
        if (ncx == ncy) {
            m_log->debug(" OK nx={} ny={} at event {}\n", clustX.size(), clustY.size(), event->GetEventNumber());
            for (int ic = 0; ic < ncx; ic++) {
                m_log->debug("  cl={}  x={}  E={} A={} N={}", ic, clustX[ic].x, clustX[ic].E, clustX[ic].A, clustX[ic].N);
                m_log->debug("  cl={}  y={}  E=}| A={} N={}", ic, clustY[ic].x, clustY[ic].E, clustY[ic].A, clustY[ic].N);

                auto cl = new SFclust();
                cl->x = clustX[ic].x;
                cl->y = clustY[ic].x;
                cl->E = clustX[ic].E + clustY[ic].E;
                cl->A = clustX[ic].A + clustY[ic].A;
                result_clusters.push_back(cl);
                //TODO gemclust.push_back(cl);
            }
        }
        Set(result_clusters);
    }
    catch (std::exception &exp) {
        m_log->error("Error during process");
        m_log->error("Exception what()='{}', type='{}'", exp.what(), typeid(exp).name());
    }
}


//------------------
// Finish
//------------------
void ClusterFactory::Finish() {
//    m_log->trace("ClusterFactory finished\n");

}

void ClusterFactory::TraceDumpSrsData(std::vector<const DGEMSRSWindowRawData *> srs_data, size_t print_rows) {
    logger()->trace("  [rocid] [slot]  [channel] [apv_id] [channel_apv] [samples size:val1,val2,...]");

    auto rows_to_display = std::min(srs_data.size(), print_rows);


    for(size_t i=0; i < rows_to_display; i++) {
        auto &srs_item = srs_data[i];
        std::string row = fmt::format("{:<7} {:<7} {:<9} {:<8} {:<13} {:<2}: ",
                                      srs_item->rocid, srs_item->slot, srs_item->channel,
                                      srs_item->apv_id, srs_item->channel_apv, srs_item->samples.size());

        for(auto sample: srs_item->samples) {
            row+=fmt::format(" {}", sample);
        }

        logger()->trace("   {}", row);
    }
}

void ClusterFactory::FillTrdHistogram(uint64_t event_number, TDirectory *hists_dir, std::vector<const DGEMSRSWindowRawData *> srs_data, int max_x, int max_y) {

    // check srs data is valid. At least assertions of what we have
    assert(srs_data.size() > 0);
    assert(srs_data[0]->samples.size() > 0);
    assert(srs_data[0]->samples.size() == srs_data[srs_data.size()-1]->samples.size());  // At least some test of samples size consistency

    int samples_size = srs_data[0]->samples.size();
    int nbins = samples_size * 128 + (samples_size-1);

    // This is going to be map<apvId, map<channelNum, vector<samples>>>
    std::map<int, std::map<int, std::vector<int> > > event_map;

    for(auto srs_item: srs_data) {

        // Dumb copy of samples because one is int and the other is uint16_t
        std::vector<int> samples;
        for(size_t i=0; i< srs_item->samples.size(); i++) {
            samples.push_back(srs_item->samples[i]);
        }

        event_map[(int)srs_item->apv_id][(int)srs_item->channel_apv] = samples;
        fMapping->APVchannelCorrection(srs_item->channel_apv);
    }
    auto storeDir = gDirectory;

    m_dir_event_hists->cd();

    // Now lets go over this
    for(auto apvPair: event_map) {
        auto apv_id = apvPair.first;
        auto apv_channels = apvPair.second;

        // Crate histogram
        std::string histo_name = fmt::format("evt_{}_{}", event_number, fMapping->GetAPVFromID(apv_id));
        std::string histo_title = fmt::format("SRS Raw Window data for Event# {} APV {}", event_number, apv_id);

        auto histo= new TH1F(histo_name.c_str(), histo_title.c_str(), nbins, -0.5, nbins - 0.5);
        histo->SetDirectory(hists_dir);

        // go over channels
        for(auto channel_pair: apv_channels) {
            auto apv_channel = channel_pair.first;
            auto samples = channel_pair.second;
            // go over samples
            for(size_t sample_i=0; sample_i < samples.size(); sample_i++){
                int bin_index = sample_i*128 + sample_i + fMapping->APVchannelCorrection(apv_channel);  // + sample_i to make gaps in between samples
                histo->SetBinContent(bin_index, samples[sample_i]);
            }
        }

        histo->Write();
    }
}

void ClusterFactory::TraceDumpMapping(std::vector<const DGEMSRSWindowRawData *> srs_data, size_t print_rows) {

    logger()->trace("  [rocid] [slot]  [channel] [apv_id] [channel_apv] [apv map]");

    auto rows_to_display = std::min(srs_data.size(), print_rows);

    for(size_t i=0; i < rows_to_display; i++) {
        auto &srs_item = srs_data[i];
        std::string row = fmt::format("{:<7} {:<7} {:<9} {:<8} {:<13} {:<2} ",
                                      srs_item->rocid, srs_item->slot, srs_item->channel,
                                      srs_item->apv_id, srs_item->channel_apv, fMapping->APVchannelCorrection(srs_item->channel));





        logger()->trace("   {}", row);
    }

}

GemApvDecodingResult ClusterFactory::DecodeApv(int apv_id, std::vector<std::vector<int>> raw_data) {
//    //===========================================================================================================
//    // BLOCK WHERE COMMON MODE CORRECTION FOR ALL APV25 TIME BINS IS COMPUTED
//    //===========================================================================================================
//    static const int NCH = 128;
//
//    auto fPedestalOffsets = fPedestal->GetAPVOffsets(apv_id);
//    auto fPedestalNoises = fPedestal->GetAPVNoises(apv_id);
//    int fComModeCut = 20;       // TODO parameter
//    double fZeroSupCut = 10;    // TODO parameter
//    int fAPVBaseline = 2500;
//    int time_bins_size = raw_data.size();
//    assert(time_bins_size>0);
//    assert(raw_data[0].size() == NCH);
//    assert(fPedestalOffsets.size() == NCH);
//
//    std::vector<double> commonModeOffsets(time_bins_size, 0);
//    std::vector<double> rawDataZS(time_bins_size, 0);
//    std::map<int, GEMHit*> fListOfHits;
//    std::map<int, GEMHit*> fListOfHitsClean;
//
//    for (Int_t timebin = 0; timebin < time_bins_size; timebin++) {
//        std::vector<int> raw_channel_values = raw_data[timebin];
//
//        // PERFORM APV25 PEDESTAL OFFSET CORRECTION FOR A GIVEN TIME BIN
//        std::vector<double> channel_values;
//        for(size_t i; i < raw_channel_values.size(); i++) {
//            channel_values[i] = raw_channel_values[i] - fPedestalOffsets[i];
//        }
//
//        std::map<double, int> values_index_map;
//        for (int i = 0; i < NCH; i++) {
//            values_index_map[channel_values[i]] = i;
//        }
//
//        // Select only N channels with lowest adc
//        std::vector<double> dataTest = channel_values;
//        std::sort(dataTest.begin(), dataTest.end());
//        assert(fComModeCut < 28);
//        for (int i = 0; i < fComModeCut; i++) {
//            //     if(fAPVID == 0) printf("\n Enter  GEMHitDecoder::APVEventDecoder()=>BF: data[%d]=%f \n",timebin,dataTest[i]) ;
//            dataTest[i] = -fPedestalOffsets[values_index_map[dataTest[i]]] + fAPVBaseline;
//            //     if(fAPVID == 0) printf(" Enter  GEMHitDecoder::APVEventDecoder()=>AF: data[%d]=%f \n",  timebin,dataTest[i]) ;
//        }
//
//        // COMPUTE COMMON MODE FOR A GIVEN APV AND TIME BIN
//        double commonMode = std::accumulate(dataTest.begin(), dataTest.end(), 0.0) / NCH;
//        commonModeOffsets.push_back(commonMode);
//        //    if(fAPVID == 0) printf(" Enter  GEMHitDecoder::APVEventDecoder(), timebin = %d, commonMode = %d \n",  timebin, commonMode) ;
//
//        // PERFORM COMMON MODE CORRECTION FOR A GIVEN TIME BIN
//        std::transform(channel_values.begin(), channel_values.end(), channel_values.begin(), std::bind2nd(std::minus<Float_t>(), commonMode));
//
//        //  ADC SUM OVER ALL TIME BINS USE AS THE TEST CRITERIA FOR ZERO SUPPRESSION
//        for(int i=0; i < rawDataZS.size(); i++) {
//            rawDataZS[i] += channel_values[i];
//        }
//    }
//
//    // ADC AVERAGE OVER ALL TIME BINS USE AS THE TEST CRITERIA FOR ZERO SUPPRESSION
//    for(int i=0; i < rawDataZS.size(); i++) {
//        rawDataZS[i] = rawDataZS[i]/time_bins_size;
//    }
//
//    //if (isCommonModeTooLarge) return;
//
//    for (size_t timebin = 0; timebin < raw_data.size(); timebin++) {
//        // EXTRACT APV25 DATA FOR A GIVEN TIME BIN
//        std::vector<int> rawDataTS = raw_data[timebin];
//        for (int chNo = 0; chNo < NCH; chNo++) {
//            int hitID = (apv_id << 8) | chNo;
//            Float_t data = -(rawDataTS[chNo] - fPedestalOffsets[chNo] - commonModeOffsets[timebin]);
//            Float_t avgdata = -rawDataZS[chNo];
//
//            //   if (fAPVID % 2 == 1) data = 1.2 * data ;
//
//            // NO ZERO SUPPRESSION
//            if (!fListOfHits[hitID]) {
//                GEMHit *hit = new GEMHit(hitID, apv_id, chNo, 0, "peakADCs", time_bins_size, /**StopTimeSamples**/time_bins_size, /**StartTimeSamples**/0);
//                fListOfHits[hitID] = hit;
//            }
//            fListOfHits[hitID]->AddTimeBinADCs(timebin, data,  fPedestalNoises[chNo]);
//            TString planename = fListOfHits[hitID]->GetPlane();
//
////            // ZERO SUPPRESSION
////            if (avgdata > (fZeroSupCut * fPedestalNoises[chNo])) {
////                if (!fListOfHitsClean[hitID]) {
////                    GEMHit *hit = new GEMHit(hitID, apv_id, chNo, fZeroSupCut, fIsHitMaxOrTotalADCs, fNbOfTimeSamples, fStopTimeSamples, fStartTimeSamples);
////                    fListOfHitsClean[hitID] = hit;
////                }
////                fListOfHitsClean[hitID]->AddTimeBinADCs(timebin, data, fPedestalNoises[chNo]);
////            }
//        }
//    }
//    GemApvDecodingResult result;
//    result.RawData = raw_data;
//    result.PedestalOffsets = fPedestalOffsets;
//    result.PedestalNoises = fPedestalNoises;
//    result.CommonModeOffsets = commonModeOffsets;
//    result.RawDataAverage = rawDataZS;
//    return result;
}
}
