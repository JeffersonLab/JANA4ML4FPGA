//
// Created by romanov on 5/25/2023.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

namespace ml4fpga::extmath {

/// Data structure to hold information about a single peak in one dimension
    struct Peak {
        int    index;        /// The index of the peak in the input data
        double height;       /// The height of the peak
        int    width;        /// The width of the peak (number of consecutive data points that are part of the peak)
        double area;         /// The area under the peak (sum of the data values that are part of the peak)
        int    time_index;   /// Index of a time slice where the peak has the maximum

        /// Equality operator for comparing peaks based on their data
        bool operator==(const Peak &other) const {
            return index == other.index && height == other.height && width == other.width && area == other.area;
        }
    };
}