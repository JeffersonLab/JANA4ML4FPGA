//
// Created by romanov on 2/27/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once


namespace ml4fpga::gem {
    class SampleData {
    public:
        uint64_t id;
        uint32_t apv;
        uint32_t channel;
        uint32_t encoded_channel;
        uint32_t time_bin;
        double value;
        double rolling_average;
        double rolling_std;
    };
}



