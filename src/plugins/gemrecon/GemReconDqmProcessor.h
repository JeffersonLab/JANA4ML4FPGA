#pragma once

#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <TDirectory.h>
#include <TH1F.h>
#include <TH2F.h>
#include "GemConfiguration.h"
#include "GEMPedestal.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "GEMHit.h"
#include "DecodedData.h"
#include "PreReconData.h"

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

        /// Directory to store histograms to
        TDirectory *m_dir_main{};
        TH1F *m_histo_1d;
        TH2F *m_trd_integral_h2d;
        TDirectory *m_dir_event_hists;
        size_t m_events_count = 0;

        GemMapping *fMapping;

        void FillRawData(uint64_t event_number, TDirectory *hists_dir, std::vector<const DGEMSRSWindowRawData *> srs_data);

        void FillDecodedData(uint64_t number, TDirectory *pDirectory, const ml4fpga::gem::DecodedData* data);

        void FillPreReconData(uint64_t event_number, TDirectory *pDirectory, const ml4fpga::gem::PreReconData* data);
    };
}      // namespace ml4fpga::gem

