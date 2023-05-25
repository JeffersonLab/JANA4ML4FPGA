//
// Created by romanov on 5/10/2023.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <vector>

namespace ml4fpga::gem {
    struct PreReconData {
        int num_time_samples;
        std::vector<double> samples_x;  /// Samples for all time slots. [[time1 256 channels], [time2 256 channels],...)
        std::vector<double> samples_y;

        /// Returns [[samples timebins 1], [samples timebin 2], ...] for X samples
        std::vector<std::vector<double>> AsTimebinsX() {
            return PreReconData::AsTimebins(num_time_samples, samples_x);
        }

        /// Returns [[samples timebins 1], [samples timebin 2], ...] for Y samples
        std::vector<std::vector<double>> AsTimebinsY() {
            return PreReconData::AsTimebins(num_time_samples, samples_y);
        }

        /// Returns [[samples timebins 1], [samples timebin 2], ...]
        static std::vector<std::vector<double>> AsTimebins(int ntsamples, std::vector<double> samples) {
            int channels_per_time = samples.size() / ntsamples;

            std::vector<std::vector<double>> result;
            for(size_t time_i=0; time_i < ntsamples; time_i++) {
                std::vector<double> samples;
                for(size_t ch_i=0; ch_i < channels_per_time; ch_i++) {
                    size_t sample_index = time_i * channels_per_time + ch_i;
                    samples.push_back(samples[sample_index]);
                }
                result.push_back(samples);
            }
            return result;
        }
    };
}
