#include <filesystem>

#include <spdlog/spdlog.h>

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <rawdataparser/DGEMSRSWindowRawData.h>
#include <rawdataparser/Df125WindowRawData.h>
#include <rawdataparser/Df125FDCPulse.h>
#include <plugins/gemrecon/old_code/GEMOnlineHitDecoder.h>
#include <extensions/root/DirectorySwitcher.h>
#include <services/dqm/DataQualityMonitor_service.h>

#include "RawData.h"
#include "Pedestal.h"
#include "GemReconDqmProcessor.h"
#include "ApvDecodedDataFactory.h"
#include "ClusterFactory.h"
#include "GemMappingService.h"


//-------------------------------------
// GemReconDqmProcessor (Constructor)
//-------------------------------------
ml4fpga::gem::GemReconDqmProcessor::GemReconDqmProcessor(JApplication *app): JEventProcessor(app)
{
    m_events_count = 0;
}

//-------------------------------------
// Init
//-------------------------------------
void ml4fpga::gem::GemReconDqmProcessor::Init() {
    std::string plugin_name = GetPluginName();

    // Get JANA application
    auto app = GetApplication();

    // Ask service locator a file to write histograms to
    m_dqm_service = app->GetService<DataQualityMonitor_service>();

    // Spawn gem reco directory
    auto dir_gem_reco = m_dqm_service->GetIntegralSubDir("gem_reco");

    // Get Log level from user parameter or default
    InitLogger(plugin_name + ":" + JTypeInfo::demangle<GemReconDqmProcessor>());

    // Get mapping
    fMapping = app->GetService<GemMappingService>()->GetMapping();

    // Create histograms
    int nbins = 2 * Constants::ChannelsCount;
    double plane_size_x = fMapping->GetPlaneSize(m_name_plane_x);
    double plane_size_y = fMapping->GetPlaneSize(m_name_plane_y);
    m_h1d_cluster_count = new TH1F("gem_clust_count", "Number of clusters in event", 10, -0.5, 9.5);
    m_h1d_cluster_pos_x = new TH1F("gem_clust_pos_x", "Reconstructed clusters for plane X", 100, -plane_size_x, plane_size_x);
    m_h1d_cluster_pos_y = new TH1F("gem_clust_pos_y", "Reconstructed clusters for plane Y", 100, -plane_size_y, plane_size_y);
    m_h2d_clust_pos_xy = new TH2F("gem_clust_pos_xy", "Reconstructed clusters XY", 128, -plane_size_x, plane_size_x, 128, -plane_size_y, plane_size_y);
    m_h1d_cluster_idx_x = new TH1F("gem_clust_idx_x", "Reconstructed clusters for plane X", nbins, -0.5, nbins - 0.5);
    m_h1d_cluster_idx_y = new TH1F("gem_clust_idx_y", "Reconstructed clusters for plane Y", nbins, -0.5, nbins - 0.5);
    m_h2d_clust_idx_xy = new TH2F("gem_cluster_idx_xy", "Reconstructed clusters XY indexes", nbins, -0.5, nbins - 0.5, nbins, -0.5, nbins - 0.5);
    m_h2d_clust_energy = new TH1F("gem_cluster_energy", "Reconstructed clusters Amplitude",  100, 0, 100);
    m_h2d_clust_amp = new TH1F("gem_cluster_amp", "Reconstructed clusters Amplitude",  100, 0, 3000);

    //  D O N E
    logger()->info("Initialization done");
}


//------------------
// Process
//------------------
// This function is called every event
void ml4fpga::gem::GemReconDqmProcessor::Process(const std::shared_ptr<const JEvent> &event) {
    m_log->debug(" new event");
    try {
        fMapping = GemMapping::GetInstance();

        bool can_fill_event = event->GetEventNumber() < 500;

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
            if(can_fill_event) {
                auto event_gem_raw_dir = m_dqm_service->GetPerEventSubDir(event->GetEventNumber(), "gem_raw");
                FillEventRawData(event->GetEventNumber(), event_gem_raw_dir, evio_raw_data);
            }
        }

//            // Data decoded for each APV
//            auto apv_data = event->GetSingle<ApvDecodedData>();
//            if(!apv_data) {
//                m_log->warn("No DecodedData for GemReconDqmProcessor");
//                return;
//            }
//            FillApvDecodedData(event->GetEventNumber(), event_hist_dir, apv_data);
//
        // -----------------------------------------------
        // Data decoded for a plane
        auto plane_data = event->GetSingle<PlaneDecodedData>();
        if(!plane_data) {
            m_log->warn("No DecodedData for GemReconDqmProcessor");
            return;
        }

        if(can_fill_event) {
            auto dir_evt_plane = m_dqm_service->GetPerEventSubDir(event->GetEventNumber(), "gem_plane");
            FillEventPlaneData(event->GetEventNumber(), dir_evt_plane, plane_data);
        }

        auto dir_int_plane = m_dqm_service->GetIntegralSubDir("gem_plane");
        FillIntegralPlaneData(event->GetEventNumber(), dir_int_plane, plane_data);

//
//            auto raw_data = event->Get<ml4fpga::gem::RawData>();
//            auto pedestal = event->GetSingle<ml4fpga::gem::Pedestal>();
//            m_log->trace("data items: {} ", raw_data.size());

        auto clusters = event->Get<SFclust>();
        if(can_fill_event) {
            auto dir_evt_reco = m_dqm_service->GetPerEventSubDir(event->GetEventNumber(), "gem_reco");
            FillEventClusters(event->GetEventNumber(), dir_evt_reco, clusters);
        }

        auto dir_int_reco = m_dqm_service->GetIntegralSubDir("gem_reco");
        FillIntegralClusters(event->GetEventNumber(), dir_int_reco, clusters);

        auto peaks = event->GetSingle<PlanePeakFindingResult>();
        if(can_fill_event) {
            auto dir_evt_peaks = m_dqm_service->GetPerEventSubDir(event->GetEventNumber(), "gem_reco");
            FillEventPeaks(event->GetEventNumber(), dir_evt_peaks, peaks);
        }
        auto dir_int_peaks = m_dqm_service->GetIntegralSubDir("gem_reco");
        FillIntegralPeaks(event->GetEventNumber(), dir_int_peaks, peaks);
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
void ml4fpga::gem::GemReconDqmProcessor::Finish() {
        m_log->trace("Finished\n");
}


void ml4fpga::gem::GemReconDqmProcessor::FillEventRawData(uint64_t event_number, TDirectory *hists_dir, std::vector<const DGEMSRSWindowRawData *> srs_data) {

    if(m_events_count++ > 500) {
        return;
    }

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

void ml4fpga::gem::GemReconDqmProcessor::FillApvDecodedData(uint64_t event_number, TDirectory *hists_dir, const ml4fpga::gem::ApvDecodedData* data) {
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


void ml4fpga::gem::GemReconDqmProcessor::FillPreReconData(uint64_t event_number, TDirectory *hists_dir, const ml4fpga::gem::PreReconData* data) {
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

void ml4fpga::gem::GemReconDqmProcessor::FillEventPlaneData(uint64_t event_number, TDirectory *directory, const PlaneDecodedData *data) {

    if(m_events_count++ > 500) {
        return;
    }

    auto saved_gdir = gDirectory;  // save current directory
    directory->cd();

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

void ml4fpga::gem::GemReconDqmProcessor::FillIntegralPlaneData(uint64_t event_number, TDirectory *directory, const ml4fpga::gem::PlaneDecodedData *data) {

    extensions::root::SwitchTDirectory switch_dir(directory);

    const auto& plane_data_x = data->plane_data.at(m_name_plane_x);
    const auto& plane_data_y = data->plane_data.at(m_name_plane_y);

    int times_count = plane_data_x.data.size();
    int plane_adc_count = plane_data_x.data[0].size();
    auto nbins = times_count*plane_adc_count + times_count;

    // We create m_h1d_gem_prerecon_[x,y] here as only here we know their true nbins size
    if(nullptr == m_h1d_gem_prerecon_x) {
        m_h1d_gem_prerecon_x = new TH1F("gem_prerecon_x", "Pre reconstruction ADC for URWELLX", nbins, -0.5, nbins - 0.5);
    }

    if(nullptr == m_h1d_gem_prerecon_y) {
        m_h1d_gem_prerecon_y = new TH1F("gem_prerecon_y", "Pre reconstruction ADC for URWELLY", nbins, -0.5, nbins - 0.5);
    }

    for(size_t time_i=0; time_i < plane_data_x.data.size(); time_i++) {
        for(size_t adc_i=0; adc_i < plane_data_x.data[time_i].size(); adc_i++) {
            int index = plane_adc_count*time_i + time_i + adc_i;
            if(event_number > 50) {
                m_h1d_gem_prerecon_x->AddBinContent(index, plane_data_x.data[time_i][adc_i]);
                m_h1d_gem_prerecon_y->AddBinContent(index, plane_data_x.data[time_i][adc_i]);
            }
        }
    }
}

void ml4fpga::gem::GemReconDqmProcessor::FillEventClusters(uint64_t event_num, TDirectory *directory, std::vector<const SFclust *> clusters)
{


}

void ml4fpga::gem::GemReconDqmProcessor::FillIntegralClusters(uint64_t evt_num, TDirectory *directory, std::vector<const SFclust *> clusters)
{
    m_h1d_cluster_count->Fill(clusters.size());

    for(auto &cluster: clusters) {
        m_h1d_cluster_pos_x->Fill(cluster->pos_x);
        m_h1d_cluster_pos_y->Fill(cluster->pos_y);
        m_h2d_clust_pos_xy->Fill(cluster->pos_x, cluster->pos_y);
        m_h1d_cluster_idx_x->Fill(cluster->index_x);
        m_h1d_cluster_idx_y->Fill(cluster->index_y);
        m_h2d_clust_idx_xy->Fill(cluster->index_x, cluster->index_y);
        m_h2d_clust_amp->Fill(cluster->amplitude);
        m_h2d_clust_energy->Fill(cluster->energy);
    }
}

void ml4fpga::gem::GemReconDqmProcessor::FillEventPeaks(uint64_t evt_num, TDirectory *directory, const ml4fpga::gem::PlanePeakFindingResult *pf_result)
{

}

void ml4fpga::gem::GemReconDqmProcessor::FillIntegralPeaks(uint64_t evt_num, TDirectory *directory, const ml4fpga::gem::PlanePeakFindingResult *pf_result)
{
    const auto& x_peaks = pf_result->peaks_by_plane.at(m_name_plane_x);
    const auto& y_peaks = pf_result->peaks_by_plane.at(m_name_plane_y);

    for(auto & x_peak: x_peaks) {
    }

    for(auto & y_peak: y_peaks) {

    }

    for(auto & x_peak: x_peaks) {
        for(auto & y_peak: y_peaks) {

        }
    }


    double plane_size_x = fMapping->GetPlaneSize(m_name_plane_x);
    double plane_size_y = fMapping->GetPlaneSize(m_name_plane_y);

}

