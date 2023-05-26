//
// Created by romanov on 5/25/2023.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithm>
#include <queue>
#include <cmath>
#include <stdexcept>
#include <vector>
#include "SingleDimensionPeakData.h"
#include "Peak.h"
#include "PeakFinder.h"


// Enumeration for the different modes of peak matching
enum class PeakFindingMode {
    AUTO,             // Automatically choose the mode based on the number of peaks in each dimension
    SORTING,          // Sort peaks by area and match them in order
    AREA_COMPARISON   // Match peaks based on the closest area
};


class PeakFinder {
public:
    // Find peaks in the input data that exceed the noise level by a factor of N, have a minimum width,
    // are separated by at least a minimum distance, and have a minimum prominence.
    // Return a vector of SingleDimensionPeakData, each representing a peak found in the data.
    std::vector<SingleDimensionPeakData> find_peaks(std::vector<double> input_data, std::vector<double> noise_data, double N, int min_width, int min_distance);

    // Match peaks in the X and Y dimensions using the specified mode.
    // Return a vector of Peak, each representing a pair of matching peaks.
    std::vector<Peak> match_peaks(std::vector<SingleDimensionPeakData> x_peaks, std::vector<SingleDimensionPeakData> y_peaks, PeakFindingMode mode);

private:
    // Match peaks in the X and Y dimensions by sorting them by area and matching them in order.
    std::vector<Peak> match_peaks_sorting(std::vector<SingleDimensionPeakData> x_peaks, std::vector<SingleDimensionPeakData> y_peaks);

    // Match peaks in the X and Y dimensions by comparing their areas and matching the closest pairs.
    std::vector<Peak> match_peaks_area_comparison(std::vector<SingleDimensionPeakData> x_peaks, std::vector<SingleDimensionPeakData> y_peaks);
};



std::vector<SingleDimensionPeakData> PeakFinder::find_peaks(std::vector<double> input_data, std::vector<double> noise_data, double N, int min_width, int min_distance) {
    std::vector<SingleDimensionPeakData> peaks;
    if(input_data.size() == 0) return peaks;        // No data no peaks

    double min_since_last_peak = input_data[0];

    int i = min_width;
    while (i < input_data.size() - min_width) {
        if (input_data[i] > N * noise_data[i]) {
            int width = 1;
            while (i + width < input_data.size() && input_data[i + width] > N * noise_data[i + width]) {
                width++;
            }
            if (width >= min_width) {
                double max_value = input_data[i];
                double area = 0.0;
                int max_index = i;
                for (int j = i; j < i + width; ++j) {
                    if (input_data[j] > max_value) {
                        max_value = input_data[j];
                        max_index = j;
                    }
                    area += input_data[j];
                }
                peaks.push_back({max_index, max_value, width, area});
            }
            i += width + min_distance;
        } else {
            if (input_data[i] < min_since_last_peak) {
                min_since_last_peak = input_data[i];
            }
            i++;
        }
    }

    return peaks;
}

std::vector<Peak> PeakFinder::match_peaks(std::vector<SingleDimensionPeakData> x_peaks, std::vector<SingleDimensionPeakData> y_peaks, PeakFindingMode mode) {
    if (mode == PeakFindingMode::AUTO) {
        if (x_peaks.size() == y_peaks.size()) {
            mode = PeakFindingMode::SORTING;
        } else {
            mode = PeakFindingMode::AREA_COMPARISON;
        }
    }

    switch (mode) {
        case PeakFindingMode::SORTING:
            return match_peaks_sorting(x_peaks, y_peaks);
        case PeakFindingMode::AREA_COMPARISON:
            return match_peaks_area_comparison(x_peaks, y_peaks);
        default:
            throw std::invalid_argument("Invalid peak finding mode");
    }
}

std::vector<Peak> PeakFinder::match_peaks_sorting(std::vector<SingleDimensionPeakData> x_peaks, std::vector<SingleDimensionPeakData> y_peaks) {
    std::vector<Peak> matched_peaks;

    // Sort the peaks by area
    std::sort(x_peaks.begin(), x_peaks.end(), [](const SingleDimensionPeakData& a, const SingleDimensionPeakData& b) {return a.area > b.area; });
    std::sort(y_peaks.begin(), y_peaks.end(), [](const SingleDimensionPeakData& a, const SingleDimensionPeakData& b) {return a.area > b.area;});

    // Match peaks
    int num_peaks = std::min(x_peaks.size(), y_peaks.size());
    for (int i = 0; i < num_peaks; ++i) {
        matched_peaks.push_back({x_peaks[i], y_peaks[i]});
    }

    return matched_peaks;
}

std::vector<Peak> PeakFinder::match_peaks_area_comparison(std::vector<SingleDimensionPeakData> x_peaks, std::vector<SingleDimensionPeakData> y_peaks) {
    struct Match {
        SingleDimensionPeakData x_peak;
        SingleDimensionPeakData y_peak;
        double area_difference;

        Match(SingleDimensionPeakData x_peak, SingleDimensionPeakData y_peak)
                : x_peak(x_peak), y_peak(y_peak), area_difference(std::abs(x_peak.area - y_peak.area)) {}

        bool operator<(const Match& other) const {
            return area_difference > other.area_difference;
        }
    };

    std::priority_queue<Match> matches;
    for (const auto& x_peak : x_peaks) {
        for (const auto& y_peak : y_peaks) {
            matches.push(Match(x_peak, y_peak));
        }
    }

    std::vector<Peak> matched_peaks;
    while (!matches.empty()) {
        Match match = matches.top();
        matches.pop();

        auto x_it = std::find(x_peaks.begin(), x_peaks.end(), match.x_peak);
        auto y_it = std::find(y_peaks.begin(), y_peaks.end(), match.y_peak);
        if (x_it != x_peaks.end() && y_it != y_peaks.end()) {
            matched_peaks.push_back({*x_it, *y_it});

            x_peaks.erase(x_it);
            y_peaks.erase(y_it);
        }
    }

    return matched_peaks;
}