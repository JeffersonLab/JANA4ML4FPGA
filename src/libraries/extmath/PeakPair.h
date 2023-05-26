#pragma once

#include "Peak.h"

namespace ml4fpga::extmath {

// Data structure to hold a pair of matching peaks in the X and Y dimensions
    struct PeakPair {
        Peak x_data;  // The data for the peak in the X dimension
        Peak y_data;  // The data for the peak in the Y dimension

        PeakPair(Peak x_data, Peak y_data)
                : x_data(x_data), y_data(y_data) {}
    };
}