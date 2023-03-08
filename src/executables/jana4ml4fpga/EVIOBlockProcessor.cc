//
// Created by xmei on 2/24/23.
//

#include <JANA/Services/JGlobalRootLock.h>
#include "EVIOBlockProcessor.h"
#include "services/root_output/RootFile_service.h"


EVIOBlockProcessor::EVIOBlockProcessor() {
    SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
    m_log = spdlog::default_logger();
}

void EVIOBlockProcessor::Init() {
    m_log->debug("EVIOBlockProcessor::Init");

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

    m_log->info("This plugin name is: " + GetPluginName());
    m_log->info("TestEventProcessor initialization is done");
}


void EVIOBlockProcessor::Process(const std::shared_ptr<const JEvent> &event) {
    m_log->debug("EVIOBlockProcessor::Process");
    // auto objs = event->Get<MyObject>();
    // std::lock_guard<std::mutex>lock(m_mutex);

    // for (const MyObject* obj : objs) {
    //     LOG << obj->datum << LOG_END;
    // }
}

void EVIOBlockProcessor::Finish() {
    m_log->debug("EVIOBlockProcessor::Finish");
    // Close any resources
    // LOG << "EVIOBlockProcessor::Finish" << LOG_END;
}
