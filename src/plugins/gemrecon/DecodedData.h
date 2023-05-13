//
// Created by romanov on 5/9/2023.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <map>
#include <vector>

namespace ml4fpga::gem {
    class SingleApvDecodedData {
    public:
        std::vector<double> PedestalOffsets;
        std::vector<double> PedestalNoises;
        std::vector<std::vector<double>> raw_data;
        std::vector<double> CommonModeOffsets;
        std::vector<double> RawDataAverage;
        std::vector<std::vector<double>> data;
    };


    struct DecodedData {
        std::map<int, SingleApvDecodedData> apv_data;
    };
}
