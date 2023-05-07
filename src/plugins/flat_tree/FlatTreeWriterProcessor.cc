#include "SrsRawRecord.h"

#include "FlatTreeWriterProcessor.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "rawdataparser/Df125WindowRawData.h"
#include "rawdataparser/Df125FDCPulse.h"
#include "rawdataparser/Df125Config.h"
#include "rawdataparser/Df250WindowRawData.h"
#include "F250WindowRawRecord.h"
#include <plugins/gemrecon/SFclust.h>

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
    // m_glb_root_lock = app->GetService<JGlobalRootLock>();
    // m_glb_root_lock->acquire_write_lock();
    try {
        auto file = root_file_service->GetHistFile();
        file->cd();
        mEventTree = new TTree("events","jana4ml4fpga_tree_v1");
        m_ios.push_back(m_srs_record_io);
        m_ios.push_back(m_f125_wraw_io);
        m_ios.push_back(m_f125_pulse_io);
        m_ios.push_back(m_f250_wraw_io);
        m_ios.push_back(m_gem_scluster_io);

        for(auto &io: m_ios) {
            io.get().bindToTree(mEventTree);
        }

        // m_glb_root_lock->release_lock();
    }
    catch (...) {
        // m_glb_root_lock->release_lock();
        // Ideally we'd use the STL read-write lock (std::shared_mutex/lock) intead of the pthreads one.
        // However, for now we are limited to C++14 (we would need C++17).
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

    // TODO // m_glb_root_lock->acquire_write_lock();
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

        // What factories do we have?
        for (auto factory: event->GetFactorySet()->GetAllFactories()) {

            // Df125Config
            if(factory->GetObjectName() == "Df125Config" && factory->GetNumObjects() > 0) {
                // pass
            }

            if(factory->GetObjectName() == "Df125FDCPulse" && factory->GetNumObjects() > 0) {
                auto f125_pulse_records = event->Get<Df125FDCPulse>();
                SaveF125FDCPulse(f125_pulse_records);
            }

            if(factory->GetObjectName() == "Df125WindowRawData" && factory->GetNumObjects() > 0) {
                auto f125_wraw_records = event->Get<Df125WindowRawData>();
                SaveF125WindowRawData(f125_wraw_records);
            }

            if(factory->GetObjectName() == "Df250WindowRawData" && factory->GetNumObjects() > 0) {
                auto f250_wraw_records = event->Get<Df250WindowRawData>();
                SaveF250WindowRawData(f250_wraw_records);
            }

            if(factory->GetObjectName() == "DGEMSRSWindowRawData" && factory->GetNumObjects() > 0) {
                auto srs_data = event->Get<DGEMSRSWindowRawData>();
                SaveGEMSRSWindowRawData(srs_data);

                auto clusters = event->Get<SFclust>();
                SaveGEMSimpleClusters(clusters);
            }

        }

        // Fill the tree
        mEventTree->Fill();
        // m_glb_root_lock->release_lock();
    }
    catch (...) {
        // m_glb_root_lock->release_lock();
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
        // m_glb_root_lock->release_lock();
    }
    catch (...) {
        // m_glb_root_lock->release_lock();
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

void FlatTreeWriterProcessor::SaveF125FDCPulse(std::vector<const Df125FDCPulse *> records) {
    for (auto record: records) {
        flatio::F125FDCPulseRecord save_struct{};
        save_struct.roc = record->rocid;
        save_struct.slot = record->slot;
        save_struct.channel = record->channel;
        save_struct.npk = record->NPK;
        save_struct.le_time = record->le_time;
        save_struct.time_quality_bit = record->time_quality_bit;
        save_struct.overflow_count = record->overflow_count;
        save_struct.pedestal = record->pedestal;
        save_struct.integral = record->integral;
        save_struct.peak_amp = record->peak_amp;
        save_struct.peak_time = record->peak_time;
        save_struct.word1 = record->word1;
        save_struct.word2 = record->word2;
        save_struct.nsamples_pedestal = record->nsamples_pedestal;
        save_struct.nsamples_integral = record->nsamples_integral;
        save_struct.emulated = record->emulated;
        save_struct.le_time_emulated = record->le_time_emulated;
        save_struct.time_quality_bit_emulated = record->time_quality_bit_emulated;
        save_struct.overflow_count_emulated = record->overflow_count_emulated;
        save_struct.pedestal_emulated = record->pedestal_emulated;
        save_struct.integral_emulated = record->integral_emulated;
        save_struct.peak_amp_emulated = record->peak_amp_emulated;
        save_struct.peak_time_emulated = record->peak_time_emulated;
        m_f125_pulse_io.add(save_struct);
    }
}

void FlatTreeWriterProcessor::SaveGEMSRSWindowRawData(std::vector<const DGEMSRSWindowRawData *> records) {
    m_log->trace("Writing DGEMSRSWindowRawData data items: {} ", records.size());
    for(auto srs_item: records) {
        flatio::SrsRawRecord srs_save{};
        srs_save.roc = srs_item->rocid;
        srs_save.slot = srs_item->slot;
        srs_save.channel = srs_item->channel;
        srs_save.apv_id = srs_item->apv_id;
        srs_save.channel_apv = srs_item->channel_apv;
        srs_save.best_sample = findBestSrsSamle(srs_item->samples);

        for(auto sample: srs_item->samples) {
            srs_save.samples.push_back(sample);
        }
        m_srs_record_io.add(srs_save);
    }
}

void FlatTreeWriterProcessor::SaveF125WindowRawData(std::vector<const Df125WindowRawData *> records) {
    for (auto record: records) {
        flatio::F125WindowRawRecord f125_wraw_save{};
        f125_wraw_save.roc = record->rocid;
        f125_wraw_save.slot = record->slot;
        f125_wraw_save.channel = record->channel;
        f125_wraw_save.overflow = record->overflow;
        f125_wraw_save.invalid_samples = record->invalid_samples;
        f125_wraw_save.itrigger = record->itrigger;
        f125_wraw_save.samples =  record->samples;
        m_f125_wraw_io.add(f125_wraw_save);
    }
}

void FlatTreeWriterProcessor::SaveF250WindowRawData(std::vector<const Df250WindowRawData *> records) {
    for (auto record: records) {
        flatio::F250WindowRawRecord f250_wraw_save{};
        f250_wraw_save.roc = record->rocid;
        f250_wraw_save.slot = record->slot;
        f250_wraw_save.channel = record->channel;
        f250_wraw_save.overflow = record->overflow;
        f250_wraw_save.invalid_samples = record->invalid_samples;
        f250_wraw_save.itrigger = record->itrigger;
        f250_wraw_save.samples =  record->samples;
        m_f250_wraw_io.add(f250_wraw_save);
    }
}

void FlatTreeWriterProcessor::SaveGEMSimpleClusters(std::vector<const SFclust *> clusters) {
    for(auto cluster: clusters) {
        flatio::GemSimpleCluster cluster_save;
        cluster_save.x = cluster->x;
        cluster_save.y = cluster->y;
        cluster_save.energy = cluster->E;
        cluster_save.adc = cluster->A;
        m_gem_scluster_io.add(cluster_save);
    }

}


