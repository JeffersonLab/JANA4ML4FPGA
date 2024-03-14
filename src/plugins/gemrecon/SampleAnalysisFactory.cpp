//
// Created by romanov on 3/7/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "SampleAnalysisFactory.h"

#include "Constants.h"

namespace ml4fpga::gem {
    void SampleAnalysisFactory::CozyInit() {
        // In general CozyFactories should not use direct App and GetParameter
        // But we do it to check the consistency and we are sure this parameter is set in gemrecon
        GetApplication()->GetParameter("daq:srs_window_raw:ntsamples", m_req_ntsamples);

        m_mapping = m_mapping_service().GetMapping();
        m_apv_names_by_id = m_mapping->GetAPVFromIDMap();
        m_mapping->GetAPVIndexOnPlaneFromIDMap();


    }

    void SampleAnalysisFactory::CozyProcess(uint64_t run_number, uint64_t event_number) {

        if(m_srs_raw_data().empty()) return;    // Nothing to do here

        // Check time bins size consistency
        auto evio_samples_size = m_srs_raw_data()[0]->samples.size();
        if (evio_samples_size != m_req_ntsamples) {
            logger()->warn("Parameter daq:srs_window_raw:ntsamples = {} != DGEMSRSWindowRawData.samples.size() from EVIO. Is parameter value correct? At event# = {}",
                m_req_ntsamples,  evio_samples_size, event_number);
        }

        for (auto srs_item: m_srs_raw_data()) {

            // It might be EVIO contains APV that we don't need to process
            if(!m_apv_names_by_id.count((int) srs_item->apv_id)) continue;

            // Dumb copy of samples because one is int and the other is uint16_t
            std::vector<int> samples;
            for (int sample_index=0; sample_index < srs_item->samples.size(); sample_index++) {
                auto sample = srs_item->samples[sample_index];
                auto result = new SampleData();
                result->apv = srs_item->apv_id;
                result->channel = Constants::ApvChannelCorrection(srs_item->channel_apv);
                if(m_mapping->GetAPVOrientation(result->apv)) {
                    result->channel = Constants::ChannelsCount - 1 - result->channel;

                }
                result->raw_channel = srs_item->channel_apv;
                result->value = sample;
                result->time_bin = sample_index;
                result->id = result->time_bin + result->channel*1000 + result->apv*1000000 + 1000000000;

                auto plane_name = m_mapping->GetPlaneFromAPVID(result->apv);
                auto plane_id = m_mapping->GetPlaneID(plane_name);
                result->plane = plane_id;
                auto det_name = m_mapping->GetDetectorFromAPVIDMap()[result->apv];
                result->detector = m_mapping->GetDetectorID(det_name);
                
                if(!m_sample_stats.count(result->id)) {
                    m_sample_stats[result->id] = extmath::RollingStatistics<double>(m_rolling_len());
                }
                auto &stat = m_sample_stats[result->id];
                stat.add(result->value);
                result->rolling_average = stat.getAverage();
                result->rolling_std = stat.getStandardDeviation();
                m_output().push_back(result);
            }
            // m_log->info("ch {:<4} : mapped: {}", srs_item->channel_apv, Constants::ApvChannelCorrection(srs_item->channel_apv));
            //apvid_chan_sampls[(int) srs_item->apv_id][Constants::ApvChannelCorrection(srs_item->channel_apv)] = samples;
            //int bin_index = fMapping->APVchannelCorrection(srs_item->channel_apv);
            //;
        }


    }
} // ml4fpga