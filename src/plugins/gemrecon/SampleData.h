//
// Created by romanov on 2/27/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once


namespace ml4fpga::gem {
    class SampleData {
    public:
        int apv;
        int channel;
        int encoded_channel;
        int time_bin;
        double value;
    };
}



