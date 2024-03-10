#include "SrsRawRecord.h"

#include "FlatTreeWriterProcessor.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "rawdataparser/Df125WindowRawData.h"
#include "rawdataparser/Df125FDCPulse.h"
#include "rawdataparser/Df250PulseData.h"
#include "rawdataparser/Df125Config.h"
#include "rawdataparser/Df250WindowRawData.h"
#include "F250WindowRawRecord.h"
#include "plugins/gemrecon/DecodedData.h"

#include "plugins/gemrecon/SFclust.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include <services/root_output/RootFile_service.h>
#include <plugins/gemrecon/Constants.h>

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
        m_main_dir = gDirectory;

        mEventTree = new TTree("events","jana4ml4fpga_tree_v1");
        m_ios.push_back(m_srs_record_io);
        m_ios.push_back(m_f125_wraw_io);
        m_ios.push_back(m_f250_wraw_io);
        m_ios.push_back(m_f125_pulse_io);
        m_ios.push_back(m_f250_pulse_io);
        m_ios.push_back(m_gem_scluster_io);
        m_ios.push_back(m_srs_prerecon_io);
        m_ios.push_back(m_gem_peak_io);
        mEventTree->SetDirectory(m_main_dir);

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

        bool has_srs_raw_window_data = false;
        bool has_gem_reconstruction = false;
        bool has_f125_reconstruction = false;

        // What factories do we have?
        for (auto factory: event->GetFactorySet()->GetAllFactories()) {
            const std::string &obj_name = factory->GetObjectName();
            auto obj_num = factory->GetNumObjects();

            // Df125Config
            if(obj_name == "Df125Config" && obj_num > 0) {
                // pass
            }

            if(obj_name == "Df125FDCPulse" && obj_num > 0) {
                auto f125_pulse_records = event->Get<Df125FDCPulse>();
                SaveF125FDCPulse(f125_pulse_records);
            }

            if(obj_name == "Df250PulseData" && obj_num > 0) {
                auto f250_pulse_records = event->Get<Df250PulseData>();
                SaveF250FDCPulse(f250_pulse_records);
            }

            if(obj_name == "Df125WindowRawData" && obj_num > 0) {
                auto f125_wraw_records = event->Get<Df125WindowRawData>();
                SaveF125WindowRawData(f125_wraw_records);
            }

            if(obj_name == "Df250WindowRawData" && obj_num > 0) {
                auto f250_wraw_records = event->Get<Df250WindowRawData>();
                SaveF250WindowRawData(f250_wraw_records);
            }

            if(obj_name == "DGEMSRSWindowRawData" && obj_num > 0) {
                auto srs_data = event->Get<DGEMSRSWindowRawData>();
                SaveGEMSRSWindowRawData(srs_data);
                has_srs_raw_window_data = true;

                try
                {
                    // TODO fix it and check for factory
                    auto clusters = event->Get<ml4fpga::gem::SFclust>();
                    SaveGEMSimpleClusters(clusters);
                    auto plane_decoded_data = event->GetSingle<ml4fpga::gem::PlaneDecodedData>();
                    SaveGEMDecodedData(plane_decoded_data);
                    auto peaks = event->Get<ml4fpga::gem::PlanePeak>();
                    SaveGEMPlanePeak(peaks);
                }
                catch(std::exception ex) {
                    //m_log->error("event->Get<SFclust>() problem: {}", ex.what());
                    // It will fail without gemrecon plugin
                    // TODO fix it fix it fix it !!!!111oneone
                }
            }
        }

        // Fill the tree
        m_main_dir->cd();
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
        m_main_dir->cd();
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

void FlatTreeWriterProcessor::SaveF125FDCPulse(const std::vector<const Df125FDCPulse *>& records) {
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

void FlatTreeWriterProcessor::SaveF250FDCPulse(const std::vector<const Df250PulseData *>& records) {
    for (auto record: records) {
        flatio::F250FDCPulseRecord save_struct{};
        save_struct.roc = record->rocid;
        save_struct.slot = record->slot;
        save_struct.channel = record->channel;
        save_struct.event_within_block = record->event_within_block;
        save_struct.qf_pedestal = record->QF_pedestal;
        save_struct.pedestal = record->pedestal;
        save_struct.integral = record->integral;
        save_struct.qf_nsa_beyond_ptw = record->QF_NSA_beyond_PTW;
        save_struct.qf_overflow = record->QF_overflow;
        save_struct.qf_underflow = record->QF_underflow;
        save_struct.nsamples_over_threshold = record->nsamples_over_threshold;
        save_struct.course_time = record->course_time;
        save_struct.fine_time = record->fine_time;
        save_struct.pulse_peak = record->pulse_peak;
        save_struct.qf_vpeak_beyond_nsa = record->QF_vpeak_beyond_NSA;
        save_struct.qf_vpeak_not_found = record->QF_vpeak_not_found;
        save_struct.qf_bad_pedestal = record->QF_bad_pedestal;
        save_struct.pulse_number = record->pulse_number;
        save_struct.nsamples_integral = record->nsamples_integral;
        save_struct.nsamples_pedestal = record->nsamples_pedestal;
        save_struct.emulated = record->emulated;
        save_struct.integral_emulated = record->integral_emulated;
        save_struct.pedestal_emulated = record->pedestal_emulated;
        save_struct.time_emulated = record->time_emulated;
        save_struct.course_time_emulated = record->course_time_emulated;
        save_struct.fine_time_emulated = record->fine_time_emulated;
        save_struct.pulse_peak_emulated = record->pulse_peak_emulated;
        save_struct.qf_emulated = record->QF_emulated;
        m_f250_pulse_io.add(save_struct);
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
        srs_save.channel_apv = ml4fpga::gem::Constants::ApvChannelCorrection(srs_item->channel_apv);

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


void FlatTreeWriterProcessor::SaveGEMSimpleClusters(std::vector<const ml4fpga::gem::SFclust *> clusters) {
    for(auto cluster: clusters) {
        flatio::GemSimpleCluster cluster_save;
        cluster_save.x = cluster->pos_x;
        cluster_save.y = cluster->pos_y;

        cluster_save.energy = cluster->energy;
        cluster_save.adc = cluster->amplitude;
        m_gem_scluster_io.add(cluster_save);
    }
}


void FlatTreeWriterProcessor::SaveGEMDecodedData(const ml4fpga::gem::PlaneDecodedData *data) {
    const auto& plane_data_x = data->plane_data.at("URWELLX");
    const auto& plane_data_y = data->plane_data.at("URWELLY");

    for(size_t time_i=0; time_i < plane_data_x.data.size(); time_i++) {
        for(size_t adc_i=0; adc_i < plane_data_x.data[time_i].size(); adc_i++) {
            flatio::SrsPreReconRecord record;
            record.x = plane_data_x.data[time_i][adc_i];
            record.y = plane_data_y.data[time_i][adc_i];
            m_srs_prerecon_io.add(record);
        }
    }
}

void FlatTreeWriterProcessor::SaveGEMPlanePeak(const std::vector<const ml4fpga::gem::PlanePeak *> &peaks) {
    for(const auto peak: peaks) {
        flatio::GemPlanePeak save_peak;
        save_peak.apv_id = peak->apv_id;
        save_peak.plane_id = peak->plane_id;
        save_peak.time_id = peak->time_id;
        save_peak.plane_name = peak->plane_name;
        save_peak.height = peak->height;
        save_peak.width = peak->width;
        save_peak.area = peak->area;
        save_peak.index = peak->index;
        save_peak.real_pos = peak->real_pos;
        m_gem_peak_io.add(save_peak);
    }
}


