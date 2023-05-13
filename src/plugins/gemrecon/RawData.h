#pragma once
#include <vector>
#include <map>
#include <string>
#include <cassert>
#include "Constants.h"


namespace ml4fpga::gem {

    class RawSingleApvData {
    public:
        int apv_id;
        std::string det_name;

        /// Samples sorted by time-channel.
        /// So it is [128 samples from all channels at time1, 128 channels at time2, ...]
        std::vector<double> all_samples;

        /// Returns [[samples timebins 1], [samples timebin 2], ...]
        std::vector<std::vector<double>> AsTimebins() {
            assert(all_samples.size() % Constants::ChannelsCount == 0);
            size_t time_bins = all_samples.size() / Constants::ChannelsCount;

            std::vector<std::vector<double>> result;
            for(size_t time_i=0; time_i < time_bins; time_i++) {
                std::vector<double> samples;
                for(size_t ch_i=0; ch_i < Constants::ChannelsCount; ch_i++) {
                    size_t sample_index = time_i*128 + ch_i;
                    samples.push_back(all_samples[sample_index]);
                }
                result.push_back(samples);
            }
            return result;
        }
    };

    /// Cleaned because
    class RawData {
    public:
        /// Map of apv and samples. Samples sorted by time-channel.
        /// So it is {APV: [128 samples from all channels at time1, 128 channels at time2, ...]}
        std::map<int, RawSingleApvData > data;
    };
}
