#include "AllInOneDqmProcessor.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "rawdataparser/Df125WindowRawData.h"
#include "rawdataparser/Df125FDCPulse.h"
#include "services/dqm/DataQualityMonitorService.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include <services/root_output/RootFile_service.h>
#include <extensions/root/DirectorySwitcher.h>

inline static void FillTrdHistogram(uint64_t event_number,
                      TDirectory *hists_dir,
                      std::vector<const Df125WindowRawData *> f125_records,
                      int slot_shift,
                      int zero_fill,
                      int max_x = 250,
                      int max_y = 300
                      ) {
    extensions::root::SwitchTDirectory switch_dir(hists_dir);
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
AllInOneDqmProcessor::AllInOneDqmProcessor(JApplication *app) :
        JEventProcessor(app) {
}

//------------------
// Init
//------------------
void AllInOneDqmProcessor::Init() {
    std::string plugin_name = GetPluginName();

    // Get JANA application
    auto app = GetApplication();

    // Ask service locator a DQM histograms to
    m_dqm_service = app->GetService<DataQualityMonitorService>();

    // Get TDirectory for histograms root file
//    auto globalRootLock = app->GetService<JGlobalRootLock>();
//    globalRootLock->acquire_write_lock();
//    auto file = root_file_service->GetHistFile();
//    globalRootLock->release_lock();

    // Create a directory for this plugin. And subdirectories for series of histograms
    // Get Log level from user parameter or default
    InitLogger(plugin_name);

    logger()->info("This plugin name is: " + GetPluginName());
    logger()->info("AllInOneDqmProcessor initialization is done");

    // BOOK INTEGRAL HISTOGRAMS
    // (by Integral means that are taken over many events)
    m_trd_integral_dir = m_dqm_service->GetIntegralSubDir("trd");
    m_trd_integral_dir->cd();

    m_trd_integral_h2d = new TH2F("trd_integral_events", "TRD events from Df125WindowRawData integral", 250, -0.5, 249.5, 300, -0.5, 299.5);

}


//------------------
// Process
//------------------
// This function is called every event
void AllInOneDqmProcessor::Process(const std::shared_ptr<const JEvent> &event) {
    m_log->debug("new event");
    try {
        if(!m_dqm_service->ShouldProcessEvent(event->GetEventNumber())) {
            return;
        }

        auto f125_records = event->Get<Df125WindowRawData>();

        // Fill TRD event histograms for the first 50 events
        auto event_gem_raw_dir = m_dqm_service->GetPerEventSubDir(event->GetEventNumber(), "trd");
        FillTrdHistogram(event->GetEventNumber(), event_gem_raw_dir, f125_records, 3, 10);


        // Fill integral histogram
        for (auto value: f125_records) {
            int x = 72 * (value->slot - 3) + value->channel;
            for (int sample_i = 0; sample_i < value->samples.size(); sample_i++) {
                float sample = value->samples[sample_i];

                m_log->trace("    sample x(channel)={} y(sample i)={} value={}", x, sample_i, sample);
                m_trd_integral_h2d->Fill(x, sample_i, sample);
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
void AllInOneDqmProcessor::Finish() {
//    m_log->trace("AllInOneDqmProcessor finished\n");

}




