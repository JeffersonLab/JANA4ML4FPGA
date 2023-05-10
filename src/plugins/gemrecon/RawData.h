#pragma once
#include <vector>
#include <map>
#include <string>


namespace ml4fpga::gem {

    class RawSingleApvData {
    public:
        int apv_id;
        std::string det_name;

        /// Samples sorted by time-channel.
        /// So it is [128 samples from all channels at time1, 128 channels at time2, ...]
        std::vector<int> samples;
    };

    /// Cleaned because
    class RawData {
    public:
        /// Map of apv and samples. Samples sorted by time-channel.
        /// So it is {APV: [128 samples from all channels at time1, 128 channels at time2, ...]}
        std::map<int, RawSingleApvData > data;
    };


}
