#include "TestCDaqProcessor.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include <services/root_output/RootFile_service.h>





//------------------
// OccupancyAnalysis (Constructor)
//------------------
TestCDaqProcessor::TestCDaqProcessor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void TestCDaqProcessor::Init()
{
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

    // Get Log level from user parameter or default
    InitLogger(plugin_name);

    logger()->info("This plugin name is: " + GetPluginName());
    logger()->info("TestCDaqProcessor initialization is done");
}


//------------------
// Process
//------------------
// This function is called every event
void TestCDaqProcessor::Process(const std::shared_ptr<const JEvent>& event)
{
    m_log->trace("TestCDaqProcessor event");



}


//------------------
// Finish
//------------------
void TestCDaqProcessor::Finish()
{
//    m_log->trace("TestCDaqProcessor finished\n");

}

