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

        TH1F **hist;
        TH2F **hist2d;
        Int_t nbOfPlaneHists = 2 * fMapping->GetNbOfPlane();
        hist    = new TH1F * [nbOfPlaneHists] ;
        hist2d    = new TH2F * [nbOfPlaneHists] ;

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




