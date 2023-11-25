#pragma once

#include <JANA/JMultifactory.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JFactoryT.h>
#include <TH2F.h>
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "GemReconDqmProcessor.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "SFclust.h"
#include "ApvDecodingResult.h"
#include "Constants.h"
#include "extmath/PeakFinder.h"
#include "GemMapping.h"
#include "PlanePeak.h"

namespace ml4fpga::gem {

    class PeakFactory: public JMultifactory {
    public:
        explicit PeakFactory() {
            DeclareOutput<PlanePeakFindingResult>("");
            DeclareOutput<PlanePeak>("");
        }
        void Init() override {
            // We need to declare upfront what our Multifactory produces.

            //DeclarePodioOutput<ExampleHit>("hits_filtered", false);
            //DeclarePodioOutput<ExampleCluster>("clusters_from_hits_filtered");
            //DeclarePodioOutput<ExampleCluster>("clusters_filtered", false);
            // Logger. Get plugin level sub-Log
            auto app = japp;     // TODO change asap as globals are terrible
            m_log = app->GetService<Log_service>()->logger("GemPeaksFactory");

            std::string default_level = "";

            // Get Log level from user parameter or default
            std::string log_level_str = default_level.empty() ?         // did user provide default level?
                                        spdlog::extensions::LogLevelToString(m_log->level()) :   //
                                        default_level;
            app->SetDefaultParameter("GemPeaksFactory:LogLevel", log_level_str, "LogLevel: trace, debug, info, warn, err, critical, off");
            m_log->set_level(spdlog::extensions::ParseLogLevel(log_level_str));
            m_log->info("Initialized");
            m_gem_mapping = GemMapping::GetInstance();
        }
        void Process(const std::shared_ptr<const JEvent>& event) override {
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

                // Peak finding saves result in 2 different forms
                // #1 - a single object with map of plane names and peaks for each plane
                auto pf_result = new PlanePeakFindingResult;
                // #2 - All peaks as pointer to save to JANA and to space
                std::vector<PlanePeak*> all_peaks;

                // Now iterate over planes
                for(auto &plane_data: srs_data->plane_data) {

                    // Get plane information
                    auto plane_name = plane_data.first;
                    auto plane_size = m_gem_mapping->GetPlaneSize(plane_name);
                    auto plane_apv_num = m_gem_mapping->GetAPVIDListFromPlane(plane_name).size();
                    auto plane_id = m_gem_mapping->GetPlaneID(plane_name);

                    // Get data
                    auto data = plane_data.second;

                    // >>> FIND PEAKS HAPPENS HERE <<<
                    auto extmath_peaks = ml4fpga::extmath::find_common_peaks(data.data, data.PedestalNoises, n_sigmas, min_width, min_distance, peak_time_tolerance);

                    // Process peaks
                    std::vector<PlanePeak> plane_peaks;
                    for(auto peak: extmath_peaks) {
                        auto plane_peak = new PlanePeak();
                        plane_peak->plane_id = plane_id;
                        plane_peak->plane_name = plane_name;
                        plane_peak->index = peak.index;
                        plane_peak->width = peak.width;
                        plane_peak->area = peak.area;
                        plane_peak->height = peak.height;
                        plane_peak->apv_id = peak.index >= Constants::ChannelsCount && plane_apv_num > 1 ? data.apv_ids[1] : data.apv_ids[0];
                        plane_peak->real_pos = Constants::CalculateStripPosition(peak.index, plane_size, plane_apv_num);
                        all_peaks.push_back(plane_peak);
                        plane_peaks.push_back(*plane_peak);
                    }
                    pf_result->peaks_by_plane[plane_name]=plane_peaks;
                }

                // Save peaks to JANA
                SetData("", all_peaks);

                // Use it here as JMultifactory can't save single record now
                std::vector<PlanePeakFindingResult*> pf_result_save;
                pf_result_save.emplace_back(pf_result);
                SetData("", pf_result_save);

//                const auto& plane_data_x = srs_data->plane_data.at(m_plane_name_x);
//                const auto& plane_data_y = srs_data->plane_data.at(m_plane_name_y);
//
//                // Find peaks separately in X and Y planes over all time samples
//                auto peaks_x = ml4fpga::extmath::find_common_peaks(plane_data_x.data, plane_data_x.PedestalNoises, n_sigmas, min_width, min_distance, peak_time_tolerance);
//                auto peaks_y = ml4fpga::extmath::find_common_peaks(plane_data_y.data, plane_data_y.PedestalNoises, n_sigmas, min_width, min_distance, peak_time_tolerance);
//
//                // Match peaks between X and Y
//                auto matched_peaks = ml4fpga::extmath::match_peaks(peaks_x, peaks_y, extmath::PeakFindingMode::AUTO);
//
//                std::vector<SFclust *> result_clusters;
//
//                m_log->debug("X: i, x, amp, width, int-l, Y: i, x, amp, width, int-l");
//                for(auto &peak: matched_peaks) {
//                    auto clust = new SFclust();
//                    clust->x_index = peak.x_data.index;
//                    clust->y_index = peak.y_data.index;
//                    clust->x = Constants::CalculateStripPosition(peak.x_data.index, m_plane_size_x, m_plane_apv_num_x);
//                    clust->y = Constants::CalculateStripPosition(peak.y_data.index, m_plane_size_y, m_plane_apv_num_y);
//                    m_log->debug("X: {:<3}, {:>7.2f}, {:>7.2f}, {:>5}, {:>7.2f} Y: {:>3}, {:>7.2f}, {:>7.2f}, {:>5}, {:>7.2f}",
//                                 clust->x_index, clust->x, peak.x_data.height, peak.x_data.width, peak.x_data.area,
//                                 clust->y_index, clust->y, peak.y_data.height, peak.y_data.width, peak.y_data.area
//                    );
//
//                    clust->A = peak.x_data.height;
//                    clust->A = peak.y_data.height;
//
//                    // Get peak plane
//                    result_clusters.push_back(clust);
//                }
//                Set(result_clusters);
            }
            catch (std::exception &exp) {
                m_log->error("Error during process");
                m_log->error("Exception what()='{}', type='{}'", exp.what(), typeid(exp).name());
            }
        }

        void Finish() override {
        }
    private:

        std::shared_ptr<spdlog::logger> m_log;
        GemMapping *m_gem_mapping;
    };


    class ClusterFactory: public JFactoryT<SFclust>, public spdlog::extensions::SpdlogMixin<ClusterFactory>  {
    public:
        explicit ClusterFactory();
        void Init() override;
        void Process(const std::shared_ptr<const JEvent>&) override;
        void Finish() override;
    private:


        /// Directory to store histograms to


        GemMapping *fMapping;               // GEM mapping


        void InitHistForZeroSup();

        void TraceDumpSrsData(std::vector<const DGEMSRSWindowRawData *> srs_data, size_t print_rows=10);
        void TraceDumpMapping(std::vector<const DGEMSRSWindowRawData *> srs_data, size_t print_rows=10);
        void FillTrdHistogram(uint64_t event_number, TDirectory *hists_dir, std::vector<const DGEMSRSWindowRawData *> srs_data, int max_x = 250, int max_y = 300);
        GemApvDecodingResult DecodeApv(int apv_id, std::vector<std::vector<int>> raw_data);

        unsigned int m_srs_ntsamples = 9;
        int m_min_adc = 50;         /// Min ADC value for histogram plotting
        int m_max_adc = 5000;       /// Max ADC value for histogram plotting
        std::string fIsHitPeakOrSumADCs = "peakADCs";
        std::string fIsCentralOrAllStripsADCs = "centralStripADCs";
        std::string m_plane_name_x = "URWELLX";
        std::string m_plane_name_y = "URWELLY";
        double m_plane_size_x;
        double m_plane_size_y;
        int m_plane_apv_num_x;
        int m_plane_apv_num_y;
    };
} // namespace ml4fpga::gem


