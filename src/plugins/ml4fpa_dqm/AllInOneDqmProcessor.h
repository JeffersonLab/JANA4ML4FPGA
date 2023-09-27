#pragma once

#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <TDirectory.h>
#include <TH1F.h>
#include <TH2F.h>
#include "services/dqm/DataQualityMonitorService.h"

class JEvent;
class JApplication;

class AllInOneDqmProcessor:
        public JEventProcessor,
        public spdlog::extensions::SpdlogMixin<AllInOneDqmProcessor>   // this automates proper Log initialization
{
public:
    explicit AllInOneDqmProcessor(JApplication *);
    ~AllInOneDqmProcessor() override = default;

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
    void Process(const std::shared_ptr<const JEvent>& event) override;

    //----------------------------
    // Finish
    //
    // This is called once after all events have been processed. You may,
    // for example, want to close an output file here.
    // Only one thread will call this.
    void Finish() override;

private:

    /// Directory to store histograms to
    TDirectory *m_trd_integral_dir{};
    TH2F* m_trd_integral_h2d;
    TDirectory *m_dir_event_hists;
    std::shared_ptr<DataQualityMonitorService> m_dqm_service;
};

