#include "TestEventProcessor.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "rawdataparser/Df125WindowRawData.h"
#include "rawdataparser/Df125FDCPulse.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include <services/root_output/RootFile_service.h>


//------------------
// OccupancyAnalysis (Constructor)
//------------------
TestEventProcessor::TestEventProcessor(JApplication *app) :
        JEventProcessor(app) {
}

//------------------
// Init
//------------------
void TestEventProcessor::Init() {
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
    m_dir_main->cd();

    // Get Log level from user parameter or default
    InitLogger(plugin_name);

    logger()->info("This plugin name is: " + GetPluginName());
    logger()->info("TestEventProcessor initialization is done");

    m_histo_1d = new TH1F("test_histo", "Test histogram", 100, -10, 10);
    m_histo_2d = new TH2F("test_histo2d", "Test histogram2d", 400, -0.5, 399.5, 300, -0.5, 299.5);
}


//------------------
// Process
//------------------
// This function is called every event
void TestEventProcessor::Process(const std::shared_ptr<const JEvent> &event) {
    m_log->debug("new event");
    try {
        auto f125_records = event->Get<Df125WindowRawData>();
        for (auto value: f125_records) {
            int x = 72 * (value->slot - 3) + value->channel;
            for (int sample_iter = 0; sample_iter < value->samples.size(); sample_iter++) {
                m_histo_2d->Fill(x, sample_iter, value->samples[sample_iter]);
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
void TestEventProcessor::Finish() {
//    m_log->trace("TestEventProcessor finished\n");

}

