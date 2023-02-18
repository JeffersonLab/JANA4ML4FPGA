#include "TestEventProcessor.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include <Services/OutputRootFile/RootFile_service.h>





//------------------
// OccupancyAnalysis (Constructor)
//------------------
TestEventProcessor::TestEventProcessor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void TestEventProcessor::Init()
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
    logger()->info("TestEventProcessor initialization is done");
}


//------------------
// Process
//------------------
// This function is called every event
void TestEventProcessor::Process(const std::shared_ptr<const JEvent>& event)
{
    m_log->trace("TestEventProcessor event");

}


//------------------
// Finish
//------------------
void TestEventProcessor::Finish()
{
//    m_log->trace("TestEventProcessor finished\n");

}

