#include "SrsRecord.h"
#include "AlignedArraysIO.h"
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

    // Get Log level from user parameter or default
    InitLogger(GetPluginName());
    logger()->info("This plugin name is: " + GetPluginName());

    // Ask service locator a file to write histograms to
    auto root_file_service = app->GetService<RootFile_service>();

    // Get TDirectory for histograms root file
    m_glb_root_lock = app->GetService<JGlobalRootLock>();
    m_glb_root_lock->acquire_write_lock();
    try {
        auto file = root_file_service->GetHistFile();
        file->cd();
        mEventTree = new TTree("events","jana4ml4fpga_tree_v1");
        m_ios.push_back(m_srs_record_io);

        for(auto &io: m_ios) {
            io.get().bindToTree(mEventTree);
        }

        m_glb_root_lock->release_lock();
    }
    catch (...) {
        m_glb_root_lock->release_lock();
        // Ideally we'd use the STL read-write lock (std::shared_mutex/lock) intead of the pthreads one.
        // However for now we are limited to C++14 (we would need C++17).
        throw;
        // It is important to re-throw exceptions so that the framework can handle them correctly
    }

    logger()->info("Initialization is done");
}


//------------------
// Process
//------------------
// This function is called every event
void FlatTreeWriterProcessor::Process(const std::shared_ptr<const JEvent> &event) {

    m_glb_root_lock->acquire_write_lock();
    try {
        m_log->debug("=======================");
        m_log->debug("Event number {}", event->GetEventNumber());

        // Clear writing classes on new event
        for(auto &io: m_ios) {
            io.get().clear();
        }

        // Print what we have in events
        if(m_log->level() <= spdlog::level::debug) {
            m_log->debug("Available data (factories actually)");
            m_log->debug("    {:<30}  {}", "[name]", "[objects count]");
            for (auto factory: event->GetFactorySet()->GetAllFactories()) {
                m_log->debug("    {:<30}  {}",  factory->GetObjectName(), factory->GetNumObjects());
            }
        }

        for (auto factory: event->GetFactorySet()->GetAllFactories()) {

            // Df125Config
            if(factory->GetObjectName() == "Df125Config" && factory->GetNumObjects() > 0) {
                // pass
            }

            if(factory->GetObjectName() == "Df125FDCPulse" && factory->GetNumObjects() > 0) {
                // pass
            }

            if(factory->GetObjectName() == "Df125WindowRawData" && factory->GetNumObjects() > 0) {

                auto f125_records = event->Get<Df125WindowRawData>();
                for (auto record: f125_records) {
                    //record->samples
                }


//                // Fill data into event_table
//
//                    int x = 72 * (record->slot - slot_shift) + record->channel;
//                    for (int sample_i = 0; sample_i < record->samples.size(); sample_i++) {
//                        if(x < max_x && sample_i < max_y) {
//                            float sample = record->samples[sample_i];
//                            if(sample < zero_fill) sample = zero_fill;  // Fill 0 values with zero_fill
//                            event_table[x][sample_i] = sample;
//                        }
//                    }
//                }
            }

            if(factory->GetObjectName() == "DGEMSRSWindowRawData" && factory->GetNumObjects() > 0) {

                auto srs_data = event->Get<DGEMSRSWindowRawData>();
                m_log->trace("Writing DGEMSRSWindowRawData data items: {} ", srs_data.size());

                for(auto srs_item: srs_data) {
                    flatio::SrsRecord srs_save;
                    srs_save.roc = srs_item->rocid;
                    srs_save.slot = srs_item->slot;
                    srs_save.channel = srs_item->channel;
                    srs_save.apv_id = srs_item->apv_id;
                    srs_save.channel_apv = srs_item->channel_apv;
                    srs_save.samples = srs_item->samples;
                    srs_save.best_sample = findBestSrsSamle(srs_item->samples);
                    m_srs_record_io.add(srs_save);
                }
            }
        }

        // Fill the tree
        mEventTree->Fill();
        m_glb_root_lock->release_lock();
    }
    catch (...) {
        m_glb_root_lock->release_lock();
        // Ideally we'd use the STL read-write lock (std::shared_mutex/lock) intead of the pthreads one.
        // However, for now we are limited to C++14 (we would need C++17).
        throw;
        // It is important to re-throw exceptions so that the framework can handle them correctly
    }


    m_log->debug("Event number {}", event->GetEventNumber());
}


//------------------
// Finish
//------------------
void FlatTreeWriterProcessor::Finish() {

    try {
        mEventTree->Write();
        m_glb_root_lock->release_lock();
    }
    catch (...) {
        m_glb_root_lock->release_lock();
        throw;
    }



}

uint16_t FlatTreeWriterProcessor::findBestSrsSamle(std::vector<uint16_t> samples) {
    // Examining data, there might be the next number of samples:
    // 1, 3, 4, n
    if(samples.size() == 0) {
        // empty array
        return 0;
    }

    if(samples.size() == 4) {
        // when 4 values are saved, the first is wrongly big, and 3 are to be chosen from
        std::vector<uint16_t> real_samples = {samples[1], samples[2], samples[3]};
        return findBestSrsSamle(real_samples);
    }

    // If we are here, just find the biggest sample
    uint16_t best_sample = samples[0];
    for(auto sample: samples) {
        if(sample > best_sample) best_sample = sample;
    }
    return best_sample;
}


