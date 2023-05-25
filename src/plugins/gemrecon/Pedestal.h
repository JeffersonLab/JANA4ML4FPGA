//
// Created by romanov on 5/9/2023.
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <vector>
#include <map>
#include "plugins/gemrecon/old_code/Constants.h"

namespace ml4fpga::gem {

    /// Pedestals for all APVs
    struct Pedestal {
        std::map<int, std::vector<double>> offsets;
        std::map<int, std::vector<double>> noises;
    };
}
