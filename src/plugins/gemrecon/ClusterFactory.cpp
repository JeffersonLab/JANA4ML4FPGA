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
    ClusterFactory::ClusterFactory() {
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
        m_trd_integral_h2d = new TH2F("trd_integral_events", "TRD events from Df125WindowRawData integral",
                                      250, -0.5, 249.5, 300, -0.5, 299.5);

        // P A R A M E T E R S
        // Number of SRS time samples:
        app->SetDefaultParameter("daq:srs_window_raw:ntsamples", m_srs_ntsamples, "Number of SRS time samples");
        app->SetDefaultParameter(plugin_name + ":min_adc", m_min_adc, "Min ADC value (For hists?)");
        app->SetDefaultParameter(plugin_name + ":max_adc", m_max_adc, "Max ADC value (For hists?)");


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

        app->SetDefaultParameter(plugin_name + ":plane_name_x", m_plane_name_x, "X Plane name (like URWELLX)");
        app->SetDefaultParameter(plugin_name + ":plane_name_y", m_plane_name_y, "Y Plane name (like URWELLY)");

        m_plane_size_x = fMapping->GetPlaneSize(m_plane_name_x);
        m_plane_size_y = fMapping->GetPlaneSize(m_plane_name_y);
        m_plane_apv_num_x = fMapping->GetAPVIDListFromPlane(m_plane_name_x).size();
        m_plane_apv_num_y = fMapping->GetAPVIDListFromPlane(m_plane_name_y).size();

        //  D O N E
        logger()->info("This plugin name is: " + GetPluginName());
        logger()->info("ClusterFactory initialization is done");
    }

//------------------
// Process
//------------------
// This function is called every event
    void ClusterFactory::Process(const std::shared_ptr<const JEvent> &event) {
        using namespace ml4fpga::gem;

        m_log->debug("Event # {}", event->GetEventNumber());
        try {
            auto srs_data = event->GetSingle<PlaneDecodedData>();
            if(srs_data == nullptr) {
                m_log->debug("PlaneDecodedData is null. No decoded data? Skipping event");
                return;
            }


            double n_sigmas = 3.0;
            int min_width = 2;
            int min_distance = 2;
            int peak_time_tolerance = 2;

            const auto& plane_data_x = srs_data->plane_data.at(m_plane_name_x);
            const auto& plane_data_y = srs_data->plane_data.at(m_plane_name_y);

            // Find peaks separately in X and Y planes over all time samples
            auto peaks_x = ml4fpga::extmath::find_common_peaks(plane_data_x.data, plane_data_x.PedestalNoises, n_sigmas, min_width, min_distance, peak_time_tolerance);
            auto peaks_y = ml4fpga::extmath::find_common_peaks(plane_data_y.data, plane_data_y.PedestalNoises, n_sigmas, min_width, min_distance, peak_time_tolerance);

            // Match peaks between X and Y
            auto matched_peaks = ml4fpga::extmath::match_peaks(peaks_x, peaks_y, extmath::PeakFindingMode::AUTO);

            std::vector<SFclust *> result_clusters;

            m_log->debug("X: i, x, amp, width, int-l, Y: i, x, amp, width, int-l");
            for(auto &peak: matched_peaks) {
                auto clust = new SFclust();
                clust->x_index = peak.x_data.index;
                clust->y_index = peak.y_data.index;
                clust->x = Constants::CalculateStripPosition(peak.x_data.index, m_plane_size_x, m_plane_apv_num_x);
                clust->y = Constants::CalculateStripPosition(peak.y_data.index, m_plane_size_y, m_plane_apv_num_y);
                m_log->debug("X: {:<3}, {:>7.2f}, {:>7.2f}, {:>5}, {:>7.2f} Y: {:>3}, {:>7.2f}, {:>7.2f}, {:>5}, {:>7.2f}",
                             clust->x_index, clust->x, peak.x_data.height, peak.x_data.width, peak.x_data.area,
                             clust->y_index, clust->y, peak.y_data.height, peak.y_data.width, peak.y_data.area
                             );

                clust->A = peak.x_data.height;
                clust->A = peak.y_data.height;

                // Get peak plane
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
