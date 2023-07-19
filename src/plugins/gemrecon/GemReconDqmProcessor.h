#pragma once

#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>

#include <TDirectory.h>
#include <TH1F.h>
#include <TH2F.h>

#include <plugins/gemrecon/old_code/PreReconData.h>
#include <plugins/gemrecon/old_code/GemConfiguration.h>
#include <plugins/gemrecon/old_code/GEMPedestal.h>
#include <plugins/gemrecon/old_code/GEMHit.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <rawdataparser/DGEMSRSWindowRawData.h>
#include <services/dqm/DataQualityMonitor_service.h>

#include "DecodedData.h"
#include "SFclust.h"
#include "PlanePeak.h"

class JEvent;
class JApplication;


namespace ml4fpga::gem {

    class GemReconDqmProcessor :
            public JEventProcessor,
            public spdlog::extensions::SpdlogMixin<GemReconDqmProcessor>   // this automates proper Log initialization
    {
    public:
        explicit GemReconDqmProcessor(JApplication *);

        ~GemReconDqmProcessor() override = default;

        //----------------------------
        // Init
        //
        // This is called once before the first call to the Process method
        // below. You may, for example, want to open an output file here.
        // Only one thread will call this.
        void Init() override;



        //----------------------------
        // Process
        //
        // This is called for every event. Multiple threads may call this
        // simultaneously. If you write something to an output file here
        // then make sure to protect it with a mutex or similar mechanism.
        // Minimize what is done while locked since that directly affects
        // the multi-threaded performance.
        void Process(const std::shared_ptr<const JEvent> &event) override;

        //----------------------------
        // Finish
        //
        // This is called once after all events have been processed. You may,
        // for example, want to close an output file here.
        // Only one thread will call this.
        void Finish() override;

    private:

        TH1F *m_h1d_gem_prerecon_x = nullptr;
        TH1F *m_h1d_gem_prerecon_y = nullptr;

        size_t m_events_count = 0;

        GemMapping *fMapping;

        void FillEventRawData(uint64_t event_number, TDirectory *hists_dir, std::vector<const DGEMSRSWindowRawData *> srs_data);

        void FillApvDecodedData(uint64_t event_number, TDirectory *pDirectory, const ml4fpga::gem::ApvDecodedData* data);

        void FillPreReconData(uint64_t event_number, TDirectory *pDirectory, const ml4fpga::gem::PreReconData* data);

        void FillEventPlaneData(uint64_t event_number, TDirectory *directory, const PlaneDecodedData *data);
        void FillIntegralPlaneData(uint64_t event_number, TDirectory *directory, const PlaneDecodedData *data);

        std::shared_ptr<DataQualityMonitor_service> m_dqm_service;
        std::string m_name_plane_x = "URWELLX";
        std::string m_name_plane_y = "URWELLY";


        void FillEventClusters(uint64_t event_num, TDirectory *directory, std::vector<const SFclust *> clusters);

        void FillIntegralClusters(uint64_t evt_num, TDirectory *directory, std::vector<const SFclust *> clusters);

        void FillEventPeaks(uint64_t evt_num, TDirectory *directory, const ml4fpga::gem::PlanePeakFindingResult *pf_result);

        void FillIntegralPeaks(uint64_t evt_num, TDirectory *directory, const PlanePeakFindingResult *pf_result);

         TH1F* m_h1d_cluster_count = nullptr;
         TH1F* m_h1d_cluster_pos_x = nullptr;
         TH1F* m_h1d_cluster_pos_y = nullptr;
         TH2F* m_h2d_clust_pos_xy = nullptr;
         TH1F* m_h1d_cluster_idx_x = nullptr;
         TH1F* m_h1d_cluster_idx_y = nullptr;
         TH2F* m_h2d_clust_idx_xy = nullptr;
         TH1F* m_h2d_clust_amp = nullptr;
         TH1F* m_h2d_clust_energy = nullptr;
    };
}      // namespace ml4fpga::gem

