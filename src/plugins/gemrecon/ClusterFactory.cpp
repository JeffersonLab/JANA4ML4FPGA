#include "ClusterFactory.h"

#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "rawdataparser/Df125WindowRawData.h"
#include "rawdataparser/Df125FDCPulse.h"
#include "plugins/gemrecon/old_code/GEMOnlineHitDecoder.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include "services/root_output/RootFile_service.h"
#include <filesystem>
#include "Pedestal.h"
#include <extmath/PeakFinder.h>

namespace ml4fpga::gem {

//-------------------------------------
// ClusterFactory (Constructor)
//-------------------------------------
    ClusterFactory::ClusterFactory(JApplication *app) {
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
        m_dir_main = dir_main_tobj ? (TDirectory *) dir_main_tobj : file->mkdir(plugin_name.c_str());
        // m_dir_event_hists = m_dir_main->mkdir("gem_events", "TRD events visualization");
        m_dir_main->cd();

        // Get Log level from user parameter or default
        InitLogger(plugin_name + ":ClusterF");

        m_histo_1d = new TH1F("test_histo", "Test histogram", 100, -10, 10);
        m_trd_integral_h2d = new TH2F("trd_integral_events", "TRD events from Df125WindowRawData integral", 250, -0.5,
                                      249.5, 300, -0.5, 299.5);

        // P A R A M E T E R S
        // Number of SRS time samples:
        app->SetDefaultParameter("daq:srs_window_raw:ntsamples", m_srs_ntsamples, "Number of SRS time samples");
        app->SetDefaultParameter(plugin_name + ":min_adc", m_min_adc, "Min ADC value (For histos?)");
        app->SetDefaultParameter(plugin_name + ":max_adc", m_max_adc, "Max ADC value (For histos?)");


        // I N I T   M A P P I N G
        std::string mapping_file = "mapping.cfg";
        app->SetDefaultParameter(plugin_name + ":mapping", mapping_file, "Full path to gem config");
        logger()->info("Mapping file file: {}", mapping_file);

        fMapping = GemMapping::GetInstance();
        fMapping->LoadMapping(mapping_file.c_str());
        fMapping->PrintMapping();

        auto plane_map = fMapping->GetAPVIDListFromPlaneMap();
        logger()->info("MAPPING EXPLAINED:");
        for (auto pair: plane_map) {
            auto name = pair.first;
            auto det_name = fMapping->GetDetectorFromPlane(name);
            auto apv_list = pair.second;
            m_log->info("  Plane: {:<10} from detector {:<10} has {} APVs:", name, det_name, apv_list.size());
            for (auto apv_id: apv_list) {
                m_log->info("    {}", apv_id);
            }
        }

        //  D O N E
        logger()->info("This plugin name is: " + GetPluginName());
        logger()->info("ClusterFactory initialization is done");
    }

//------------------
// Process
//------------------
// This function is called every event
    void ClusterFactory::Process(const std::shared_ptr<const JEvent> &event) {
        m_log->debug("new event");
        try {
            auto srs_data = event->GetSingle<PlaneDecodedData>();
            m_log->trace("DGEMSRSWindowRawData data items: {} ", srs_data->plane_data.size());

            double n_sigmas = 3.0;
            int min_width = 2;
            int min_distance = 2;
            int peak_time_tolerance = 2;

            const auto& plane_data = srs_data->plane_data.at("URWELLX");

            auto peaksx = ml4fpga::extmath::find_common_peaks(plane_data.data, plane_data.PedestalNoises, n_sigmas, min_width, min_distance, peak_time_tolerance);
            auto peaksy = ml4fpga::extmath::find_common_peaks(plane_data.data, plane_data.PedestalNoises, n_sigmas, min_width, min_distance, peak_time_tolerance);

            auto matched_peaks = ml4fpga::extmath::match_peaks(peaksx, peaksy, extmath::PeakFindingMode::AUTO);

            std::vector<SFclust *> result_clusters;

            for(auto &peak: matched_peaks) {
                auto clust = new SFclust();
                clust->x = peak.x_data.index;
                clust->y = peak.y_data.index;
                result_clusters.push_back(clust);

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

}
