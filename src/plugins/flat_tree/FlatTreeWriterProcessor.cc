#include "FlatTreeWriterProcessor.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "rawdataparser/Df125WindowRawData.h"
#include "rawdataparser/Df125FDCPulse.h"
#include "rawdataparser/Df125Config.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include <services/root_output/RootFile_service.h>


//------------------
// OccupancyAnalysis (Constructor)
//------------------
FlatTreeWriterProcessor::FlatTreeWriterProcessor(JApplication *app) :
        JEventProcessor(app) {
}

//------------------
// Init
//------------------
void FlatTreeWriterProcessor::Init() {

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
    m_dir_main = file->mkdir(GetPluginName().c_str());
    m_dir_main->cd();

    // Get Log level from user parameter or default
    InitLogger(GetPluginName());

    logger()->info("This plugin name is: " + GetPluginName());
    logger()->info("FlatTreeWriterProcessor initialization is done");
}


//------------------
// Process
//------------------
// This function is called every event
void FlatTreeWriterProcessor::Process(const std::shared_ptr<const JEvent> &event) {

    m_log->debug("Event number {}", event->GetEventNumber());
}


//------------------
// Finish
//------------------
void FlatTreeWriterProcessor::Finish() {
//    m_log->trace("FlatTreeWriterProcessor finished\n");

}


