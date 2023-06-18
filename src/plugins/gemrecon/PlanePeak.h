// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

namespace ml4fpga::gem {
    struct PlanePeak {
        uint32_t plane_id;         /// Plane id
        std::string plane_name;    /// Plane name
        int index;                 /// The index of the peak in the input data
        int apv_id;                /// ID of APV with this adc
        double height;             /// The height of the peak
        int width;                 /// The width of the peak (number of consecutive data points that are part of the peak)
        double area;               /// The area under the peak (sum of the data values that are part of the peak)
        double real_pos;           /// Real position calculated from plane size
    };

    struct PlanePeakFindingResult {
        std::map<std::string, std::vector<ml4fpga::gem::PlanePeak>> peaks_by_plane;
    };
}