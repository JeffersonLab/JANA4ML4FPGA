#include "PrintEvioProcessor.h"
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
PrintEvioProcessor::PrintEvioProcessor(JApplication *app) :
        JEventProcessor(app) {
}

//------------------
// Init
//------------------
void PrintEvioProcessor::Init() {

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
    logger()->info("PrintEvioProcessor initialization is done");
}


//------------------
// Process
//------------------
// This function is called every event
void PrintEvioProcessor::Process(const std::shared_ptr<const JEvent> &event) {

    m_log->info("=======================");
    m_log->info("Event number {}", event->GetEventNumber());
    m_log->info("Available data (factories actually)");
    m_log->info("    {:<30}  {}", "[name]", "[objects count]");

    // Print what we have in events
    for (auto factory: event->GetFactorySet()->GetAllFactories()) {
        m_log->info("    {:<30}  {}",
                    factory->GetObjectName(), factory->GetNumObjects());
    }

    for (auto factory: event->GetFactorySet()->GetAllFactories()) {

        // Df125Config
        if(factory->GetObjectName() == "Df125Config" && factory->GetNumObjects() > 0) {
            PrintF125Config(event);
        }

        if(factory->GetObjectName() == "Df125FDCPulse" && factory->GetNumObjects() > 0) {
            PrintF125FDCPulse(event);
        }

        if(factory->GetObjectName() == "Df125WindowRawData" && factory->GetNumObjects() > 0) {
            PrintF125WindowRawData(event);
        }

        if(factory->GetObjectName() == "DGEMSRSWindowRawData" && factory->GetNumObjects() > 0) {
            PrintGEMSRSWindowRawData(event);
        }
    }
}


//------------------
// Finish
//------------------
void PrintEvioProcessor::Finish() {
//    m_log->trace("PrintEvioProcessor finished\n");

}

void PrintEvioProcessor::PrintF125Config(const std::shared_ptr<const JEvent> &event) {
    auto config = event->GetSingle<Df125Config>();
    //config->
    m_log->info("F125 Config");
    m_log->info("  Num. samples before threshold crossing sample");
    m_log->info("  NSA {:<7} NSB {:<7} NSA_NSB {:<7} NPED {:<7} WINWIDTH {:<7} PL   {:<7}",
                config->NSA, config->NSB, config->NSA_NSB, config->NPED, config->WINWIDTH, config->PL);
    m_log->info("  NW  {:<7} NPK {:<7} P1      {:<7} P2   {:<7} PG       {:<7} IE   {:<7}",
                config->NW, config->NPK, config->P1, config->P2, config->PG, config->IE);
    m_log->info("  H   {:<7} TH  {:<7} TL      {:<7} IBIT {:<7} ABIT     {:<7} PBIT {:<7}",
                config->H, config->TH, config->TL, config->IBIT, config->ABIT, config->PBIT);
}

void PrintEvioProcessor::PrintF125FDCPulse(const std::shared_ptr<const JEvent> &event) {
    auto f125_pulse_data = event->Get<Df125FDCPulse>();

    logger()->info("Df125FDCPulse data items: {} ", f125_pulse_data.size());
    logger()->info("    [rocid] [slot]  [channel] [NPK]   [le_time] [peak_time] [peak_amp]");


    for (auto value: f125_pulse_data) {
        logger()->info("     {:<7} {:<7} {:<9} {:<7} {:<9} {:<11} {:<7}",
                       value->rocid, value->slot, value->channel,
                       value->NPK, value->le_time, value->peak_time, value->peak_amp);
    }
}

void PrintEvioProcessor::PrintGEMSRSWindowRawData(const std::shared_ptr<const JEvent> &event) {
    auto srs_data = event->Get<DGEMSRSWindowRawData>();
    m_log->info("DGEMSRSWindowRawData data items: {} ", srs_data.size());

    logger()->info("  [rocid] [slot]  [channel] [apv_id] [channel_apv] [samples size:val1,val2,...]");
    for(auto srs_item: srs_data) {
        std::string row = fmt::format("{:<7} {:<7} {:<9} {:<8} {:<13} {:<2}: ",
                                      srs_item->rocid, srs_item->slot, srs_item->channel,
                                      srs_item->apv_id, srs_item->channel_apv, srs_item->samples.size());

        for(auto sample: srs_item->samples) {
            row+=fmt::format(" {}", sample);
        }

        logger()->info("   {}", row);
    }
}

void PrintEvioProcessor::PrintF125WindowRawData(const std::shared_ptr<const JEvent> &event) {
    auto f125_data = event->Get<Df125WindowRawData>();
    m_log->info("Df125WindowRawData  data items: {} ", f125_data.size());
    logger()->info("  [rocid] [slot]  [channel] [invalid_samples] [overflow] [itrigger] [samples count]");
    for (auto value: f125_data) {
        logger()->info("   {:<7} {:<7} {:<9} {:<17} {:<9} {:<11} {}", value->rocid, value->slot, value->channel, value->invalid_samples, value->overflow, value->itrigger, value->samples.size());
    }
}




