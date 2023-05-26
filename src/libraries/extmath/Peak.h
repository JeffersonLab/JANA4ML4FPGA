#pragma once

#include "SingleDimensionPeakData.h"

// Data structure to hold a pair of matching peaks in the X and Y dimensions
struct Peak {
    SingleDimensionPeakData x_data;  // The data for the peak in the X dimension
    SingleDimensionPeakData y_data;  // The data for the peak in the Y dimension

    Peak(SingleDimensionPeakData x_data, SingleDimensionPeakData y_data)
            : x_data(x_data), y_data(y_data) {}
};