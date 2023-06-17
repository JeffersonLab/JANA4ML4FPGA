#include <filesystem>

#include <spdlog/spdlog.h>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <rawdataparser/DGEMSRSWindowRawData.h>
#include <rawdataparser/Df125WindowRawData.h>
#include <rawdataparser/Df125FDCPulse.h>
#include <plugins/gemrecon/old_code/GEMOnlineHitDecoder.h>

#include <services/dqm/DataQualityMonitor_service.h>

#include "RawData.h"
#include "Pedestal.h"
#include "GemReconDqmProcessor.h"
#include "ApvDecodedDataFactory.h"


namespace ml4fpga::gem {
//-------------------------------------
// GemReconDqmProcessor (Constructor)
//-------------------------------------
    GemReconDqmProcessor::GemReconDqmProcessor(JApplication *app) :
            JEventProcessor(app) {
        m_events_count = 0;
    }

//-------------------------------------
// Init
//-------------------------------------
    void GemReconDqmProcessor::Init() {
        std::string plugin_name = GetPluginName();

        // Get JANA application
        auto app = GetApplication();

        // Ask service locator a file to write histograms to
        m_dqm_service = app->GetService<DataQualityMonitor_service>();

        // Get Log level from user parameter or default
        InitLogger(plugin_name + ":" + JTypeInfo::demangle<GemReconDqmProcessor>());

        m_dqm_service->GetIntegralDir()->cd();
        m_trd_integral_h2d = new TH2F("gemtrd_events", "TRD events from Df125WindowRawData integral", 250, -0.5, 249.5, 300, -0.5, 299.5);

        //  D O N E
        logger()->info("initialization done");
    }


//------------------
// Process
//------------------
// This function is called every event
    void GemReconDqmProcessor::Process(const std::shared_ptr<const JEvent> &event) {
        m_log->debug(" new event");
        if(m_events_count++ > 500) {
            return;
        }
        try {
            fMapping = GemMapping::GetInstance();

            auto event_gem_raq_dir = m_dqm_service->GetPerEventSubDir(event->GetEventNumber(), "gem_raw");

            // Raw GEM data
            auto evio_raw_data = event->Get<DGEMSRSWindowRawData>();

            // ID why but sometimes there is no the last sample data
            if(evio_raw_data.size() > 0) {
                int samples_size_0 = evio_raw_data[0]->samples.size();
                for(auto srs_item: evio_raw_data) {

                    if (samples_size_0 != srs_item->samples.size()) {
                        m_log->warn("samples_size_0 = {} != srs_item->samples.size() = {} at event# = {}", samples_size_0,  srs_item->samples.size(), event->GetEventNumber());
                        continue;
                    }
                }
                FillRawData(event->GetEventNumber(), event_gem_raq_dir, evio_raw_data);
            }

//            // Data decoded for each APV
//            auto apv_data = event->GetSingle<ApvDecodedData>();
//            if(!apv_data) {
//                m_log->warn("No DecodedData for GemReconDqmProcessor");
//                return;
//            }
//            FillApvDecodedData(event->GetEventNumber(), event_hist_dir, apv_data);
//
            // Data decoded for a plane
            auto plane_data = event->GetSingle<PlaneDecodedData>();
            if(!plane_data) {
                m_log->warn("No DecodedData for GemReconDqmProcessor");
                return;
            }

            auto event_gem_plane_dir = m_dqm_service->GetPerEventSubDir(event->GetEventNumber(), "gem_plane");

            FillPlaneDecodedData(event->GetEventNumber(), event_gem_plane_dir, plane_data);
//
//            auto raw_data = event->Get<ml4fpga::gem::RawData>();
//            auto pedestal = event->GetSingle<ml4fpga::gem::Pedestal>();
//            m_log->trace("data items: {} ", raw_data.size());
        }
        catch (std::exception &exp) {
            std::string no_factory_message = "Could not find JFactoryT<DGEMSRSWindowRawData>";
            if (std::string(exp.what()).find(no_factory_message) != std::string::npos) {
                // The first events might be some technical data and maybe DGEMSRSWindowRawData is not yet there so
                // we skip this. For later events we make it a warning
                std::string log_message = fmt::format(no_factory_message + " at event {}", event->GetEventNumber());
                if(event->GetEventNumber() < 10) {
                    m_log->debug(log_message);
                } else {
                    m_log->warn(log_message);
                }
            }
            m_log->error("Error during process");
            m_log->error("Exception what()='{}', type='{}'", exp.what(), typeid(exp).name());
        }
    }


//------------------
// Finish
//------------------
    void GemReconDqmProcessor::Finish() {
//    m_log->trace("GemReconDqmProcessor finished\n");

    }


    void GemReconDqmProcessor::FillRawData(uint64_t event_number, TDirectory *hists_dir, std::vector<const DGEMSRSWindowRawData *> srs_data) {

        // check srs data is valid. At least assertions of what we have
        assert(srs_data.size() > 0);
        assert(srs_data[0]->samples.size() > 0);

        int samples_size = srs_data[0]->samples.size();
        int nbins = samples_size * 128 + (samples_size-1);

        // This is going to be map<apvId, map<channelNum, vector<samples>>>
        std::map<int, std::map<int, std::vector<int> > > event_map;

        // Making a map
        for(auto srs_item: srs_data) {

            if(samples_size != srs_item->samples.size()) continue;

            // Dumb copy of samples because one is int and the other is uint16_t
            std::vector<int> samples;
            for(size_t i=0; i< srs_item->samples.size(); i++) {
                samples.push_back(srs_item->samples[i]);
            }

            event_map[(int)srs_item->apv_id][(int)srs_item->channel_apv] = samples;
        }

        auto saved_gdir = gDirectory;  // save current directory
        hists_dir->cd();

        // Now lets go over this map
        for(auto apvPair: event_map) {
            auto apv_id = apvPair.first;
            auto apv_channels = apvPair.second;

            // Crate histogram
            std::string apv_name = fMapping->GetAPVFromID(apv_id);
            std::string histo_name = fmt::format("raw_{}", apv_name);
            std::string histo_title = fmt::format("SRSRawWindowSata for Event# {} APV {}-{}", event_number, apv_id, apv_name);

            auto histo= new TH1F(histo_name.c_str(), histo_title.c_str(), nbins, -0.5, nbins - 0.5);
            histo->SetDirectory(hists_dir);

            // go over channels
            for(auto channel_pair: apv_channels) {
                auto apv_channel = channel_pair.first;
                auto samples = channel_pair.second;
                // go over samples
                for(size_t sample_i=0; sample_i < samples.size(); sample_i++){
                    int bin_index = sample_i*128 + sample_i + fMapping->APVchannelCorrection(apv_channel);  // + sample_i to make gaps in between samples
                    histo->SetBinContent(bin_index, samples[sample_i]);
                }
            }

            //histo->Write();
        }

        // Restore current directory
        if(saved_gdir) saved_gdir->cd();
    }

    void GemReconDqmProcessor::FillApvDecodedData(uint64_t event_number, TDirectory *hists_dir, const ml4fpga::gem::ApvDecodedData* data) {
        auto saved_gdir = gDirectory;  // save current directory
        hists_dir->cd();

        // Now lets go over this
        for(auto apv_pair: data->apv_data) {
            auto apv_id = apv_pair.first;
            auto &timebins = apv_pair.second.data;

            size_t nbins = timebins.size() * 128 + (timebins.size()-1);

            // Crate histogram
            std::string histo_name = fmt::format("evt_{}_dec_{}", event_number, fMapping->GetAPVFromID(apv_id));
            std::string histo_title = fmt::format("SRS Raw Window data for Event# {} APV {}", event_number, apv_id);

            auto histo= new TH1F(histo_name.c_str(), histo_title.c_str(), nbins, -0.5, nbins - 0.5);
            histo->SetDirectory(hists_dir);

            // go over channels
            for(size_t time_i = 0; time_i < timebins.size(); time_i++) {
                for(size_t ch_i = 0; ch_i < Constants::ChannelsCount; ch_i++)
                {
                    // go over samples
                    int bin_index = time_i*128 + time_i + ch_i;  // + time_i to make gaps in between samples
                    histo->SetBinContent(bin_index, timebins[time_i][ch_i]);
                }
            }
        }

        if(saved_gdir) saved_gdir->cd();  // restore gDirectory
    }

    void GemReconDqmProcessor::FillPreReconData(uint64_t event_number, TDirectory *hists_dir, const ml4fpga::gem::PreReconData* data) {
        hists_dir->cd();

        // Now lets go over this
        for(auto smpl_x: data->samples_x) {
            for(auto smpl_x: data->samples_y) {

//            }
//            auto apv_id = apv_pair.first;
//            auto &timebins = apv_pair.second.data;
//
//            size_t nbins = timebins.size() * 128 + (timebins.size()-1);
//
//            // Crate histogram
//            std::string histo_name = fmt::format("evt_{}_xy_{}", event_number, fMapping->GetAPVFromID(apv_id));
//            std::string histo_title = fmt::format("SRS Raw Window data for Event# {} APV {}", event_number, apv_id);
//
//            auto histo= new TH1F(histo_name.c_str(), histo_title.c_str(), nbins, -0.5, nbins - 0.5);
//            histo->SetDirectory(pDirectory);
//
//            // go over channels
//            for(size_t time_i = 0; time_i < timebins.size(); time_i++) {
//                for(size_t ch_i = 0; ch_i < Constants::ChannelsCount; ch_i++)
//                {
//                    // go over samples
//                    int bin_index = time_i*128 + time_i + ch_i;  // + time_i to make gaps in between samples
//                    histo->SetBinContent(bin_index, timebins[time_i][ch_i]);
//                }
//            }

                //histo->Write();
            }
        }
    }

    void GemReconDqmProcessor::FillPlaneDecodedData(uint64_t event_number, TDirectory *hists_dir, const PlaneDecodedData *data) {

        auto saved_gdir = gDirectory;  // save current directory
        hists_dir->cd();

        const auto& plane_data_x = data->plane_data.at("URWELLX");
        const auto& plane_data_y = data->plane_data.at("URWELLY");

        // Crate histogram
        std::string histo_name_x = fmt::format("plane_x", event_number);
        std::string histo_name_y = fmt::format("plane_y", event_number);
        std::string histo_title_x = fmt::format("URWELLX data for event {}", event_number);
        std::string histo_title_y = fmt::format("URWELLY data for event {}", event_number);

        int times_count = plane_data_x.data.size();
        int plane_adc_count = plane_data_x.data[0].size();
        auto nbins = times_count*plane_adc_count + times_count;

        auto histo_x = new TH1F(histo_name_x.c_str(), histo_title_x.c_str(), nbins, -0.5, nbins - 0.5);
        auto histo_y = new TH1F(histo_name_y.c_str(), histo_title_y.c_str(), nbins, -0.5, nbins - 0.5);

        // TODO
//        // We create m_h1d_gem_prerecon_[x,y] here as here we know their size
//        if(!m_h1d_gem_prerecon_x) {
//            m_h1d_gem_prerecon_x = new TH1F("gem_prerecon_x", "Pre reconstruction ADC for URWELLX", nbins, -0.5, nbins - 0.5);
//            m_h1d_gem_prerecon_x->SetDirectory(m_);
//        }
//
//        if(!m_h1d_gem_prerecon_y) {
//            m_h1d_gem_prerecon_y = new TH1F("gem_prerecon_y", "Pre reconstruction ADC for URWELLY", nbins, -0.5, nbins - 0.5);
//            m_h1d_gem_prerecon_y->SetDirectory(hists_dir);
//        }


        for(size_t time_i=0; time_i < plane_data_x.data.size(); time_i++) {
            for(size_t adc_i=0; adc_i < plane_data_x.data[time_i].size(); adc_i++) {
                int index = plane_adc_count*time_i + time_i + adc_i;
                histo_x->SetBinContent(index, plane_data_x.data[time_i][adc_i]);
                histo_y->SetBinContent(index, plane_data_y.data[time_i][adc_i]);

//                if(event_number > 50) {
//                    m_h1d_gem_prerecon_x->AddBinContent(index, plane_data_x.data[time_i][adc_i]);
//                    m_h1d_gem_prerecon_x->AddBinContent(index, plane_data_x.data[time_i][adc_i]);
//                }
            }
            if(time_i > 0) {

                histo_x->SetBinContent(plane_adc_count*time_i, 0);
                histo_y->SetBinContent(plane_adc_count*time_i, 0);

            }
        }
        // Restore current directory
        if(saved_gdir) saved_gdir->cd();
    }

}