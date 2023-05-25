#include "PedestalFactory.h"
#include "RawData.h"
#include <JANA/JEvent.h>
#include "GemMapping.h"
#include "Pedestal.h"
#include "plugins/gemrecon/old_code/Constants.h"

namespace ml4fpga::gem {
    void PedestalFactory::Init() {
        InitLogger(GetPluginName() + ":gem:PedestalFactory");

        auto mapping = GemMapping::GetInstance();

        // Initialize list of average channels per ID
        for(auto &apv_pair: mapping->GetAPVFromIDMap()) {
            auto apv_id = apv_pair.first;
            m_apv_averages[apv_id] = std::vector<extmath::Average>(Constants::ChannelsCount);
            m_apv_std[apv_id] = std::vector<extmath::StandardDeviation>(Constants::ChannelsCount);
        }
    }

    void PedestalFactory::BeginRun(const std::shared_ptr<const JEvent>& event) {
        m_events_counter = 0;

    }

    void PedestalFactory::Process(const std::shared_ptr<const JEvent> &event) {

        // Get raw data
        auto srs_data = event->GetSingle<RawData>();

        auto result = new Pedestal();
        // Process each APV
        for(auto& apv_pair: srs_data->data) {
            int apv_id = apv_pair.first;
            auto apv_data = apv_pair.second;
            auto timebins = apv_data.AsTimebins();

            // This will be result for this APV

            // Go over timebins and channels
            for(size_t timebin_index = 0; timebin_index < timebins.size(); timebin_index++) {
                for(size_t channel_index = 0; channel_index < Constants::ChannelsCount; channel_index++) {
                    double data = timebins[timebin_index][channel_index];

                    // TODO test for peaks

                    // Add data to averaging and std
                    if(m_events_counter < m_events_to_average) {
                        m_apv_averages[apv_id][channel_index].add(data);
                        m_apv_std[apv_id][channel_index].add(data);
                    }
                }
            }

            std::vector<double> offsets;
            std::vector<double> noises;
            // Fill pedestal with averaged values... or constant value
            for(size_t ch_i=0; ch_i < Constants::ChannelsCount; ch_i++) {
                auto noise = m_events_counter < m_minimal_events ? m_constant_noise : m_apv_std[apv_id][ch_i].stddev();
                auto offset = m_events_counter < m_minimal_events ? m_constant_offset : m_apv_averages[apv_id][ch_i].mean();
                noises.push_back(noise);
                offsets.push_back(offset);
            }

            result->noises[apv_id] = noises;
            result->offsets[apv_id] = offsets;
        }
        Insert(result);
        m_events_counter++;
    }

    void PedestalFactory::Finish() {

    }


} // ml4fpga