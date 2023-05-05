#include <map>
#include <vector>
#pragma once

namespace ml4fpga::gem {
    class PedestalCalculator {
    public:
       std::map<int, std::vector<double>> CalculateEventPedestals(std::vector<std::vector<int>> raw_data);
    private:

    };
}

