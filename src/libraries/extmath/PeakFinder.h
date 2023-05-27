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
#include "Peak.h"
#include "PeakPair.h"
#include "PeakFinder.h"


namespace ml4fpga::extmath {
// Enumeration for the different modes of peak matching
    enum class PeakFindingMode {
        AUTO,             // Automatically choose the mode based on the number of peaks in each dimension
        SORTING,          // Sort peaks by area and match them in order
        AREA_COMPARISON   // Match peaks based on the closest area
    };


    /// Find peaks in the input data that exceed the noise level by a factor of N, have a minimum width,
    /// are separated by at least a minimum distance, and have a minimum prominence.
    /// Return a vector of SingleDimensionPeakData, each representing a peak found in the data.
    inline std::vector<Peak>
    find_peaks(std::vector<double> input_data, std::vector<double> noise_data, double n_sigmas, int min_width,
               int min_distance);

    /// @brief Identifies common peaks across multiple time slices of ADC data.
    ///
    /// The find_common_peaks function is designed to identify common peaks across multiple time slices of ADC data.
    ///
    /// Here's a step-by-step explanation of what the function does:
    ///  1. Detect peaks in each time slice: The function begins by detecting peaks in each time slice.
    ///     It uses the N, min_width, min_distance, and noise_data for peak finding algorithm for each time slice.
    ///     It stores the peaks from each time slice in a two-dimensional vector.
    ///
    ///  2. Check for common peaks: Next, the function iterates through each detected peak in each time slice.
    ///     For each peak, it checks if there is a corresponding peak in all other time slices.
    ///     A corresponding peak is defined as a peak whose position and width are within the specified tolerance
    ///     level of the original peak's position and width. If a peak is found in all time slices, it is considered
    ///     a common peak and added to the common_peaks vector.
    ///
    ///  3.  Select peaks with the maximum amplitude: Finally, the function iterates through the common_peaks vector
    ///  and keeps only one peak for each unique position - the one with the highest amplitude.
    ///  If a new peak is found at the same position as an existing peak in the common_peaks vector,
    ///  and its amplitude is higher than the existing peak's amplitude, the existing peak is replaced by the new peak.
    ///
    ///  The function then returns the common_peaks vector, which contains the common peaks with the highest amplitude for each unique position across all time slices.
    ///
    ///
    /// @param time_slices A 2D vector where each element represents a time slice of ADC readings. Each time slice is a 1D vector of double values representing ADC readings.
    /// @param noise_data A 1D vector of double values representing the standard deviations of the ADC readings, used to determine the noise level for each channel.
    /// @param n_sigmas A double value representing the threshold multiplier. This is used in conjunction with noise_data to determine the minimum height for a peak.
    /// @param min_width An integer representing the minimum width of a peak. Any detected peak with a width smaller than this value will be discarded.
    /// @param min_distance An integer representing the minimum distance between two peaks. Peaks closer than this value to each other will be treated as a single peak.
    /// @param max_time_shift A double value representing the tolerance for peak position when comparing peaks across different time slices. Peaks within this tolerance are treated as the same peak.
    ///
    /// @return Returns a vector of SingleDimensionPeakData objects, each representing a common peak detected across all time slices. For each unique peak position, only the peak with the highest amplitude is returned.
    ///
    inline std::vector<Peak>
    find_common_peaks(const std::vector<std::vector<double>> &time_slices, const std::vector<double> &noise_data, double n_sigmas,
                      int min_width, int min_distance, double max_time_shift);

    // Match peaks in the X and Y dimensions using the specified mode.
    // Return a vector of Peak, each representing a pair of matching peaks.
    inline  std::vector<PeakPair> match_peaks(const std::vector<Peak> &x_peaks,
                                      const std::vector<Peak> &y_peaks, PeakFindingMode mode);


    // Match peaks in the X and Y dimensions by sorting them by area and matching them in order.
    inline  std::vector<PeakPair>
    match_peaks_sorting(std::vector<Peak> x_peaks, std::vector<Peak> y_peaks);

    // Match peaks in the X and Y dimensions by comparing their areas and matching the closest pairs.
    inline  std::vector<PeakPair> match_peaks_area_comparison(std::vector<Peak> x_peaks,
                                                      std::vector<Peak> y_peaks);


    std::vector<Peak>
    find_peaks(std::vector<double> input_data, std::vector<double> noise_data, double n_sigmas, int min_width, int min_distance) {
        std::vector<Peak> peaks;
        if (input_data.empty()) return peaks;        // No data no peaks

        double min_since_last_peak = input_data[0];

        for (int i = 0; i < input_data.size() - min_width;) {
            if (input_data[i] > n_sigmas * noise_data[i]) {
                int width = 1;
                while (i + width < input_data.size() && input_data[i + width] > n_sigmas * noise_data[i + width]) {
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


    std::vector<PeakPair> match_peaks(const std::vector<Peak> &x_peaks,
                                      const std::vector<Peak> &y_peaks, PeakFindingMode mode) {
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

    std::vector<PeakPair>
    match_peaks_sorting(std::vector<Peak> x_peaks, std::vector<Peak> y_peaks) {
        std::vector<PeakPair> matched_peaks;

        // Sort the peaks by area
        std::sort(x_peaks.begin(), x_peaks.end(), [](const Peak &a, const Peak &b) { return a.area > b.area; });
        std::sort(y_peaks.begin(), y_peaks.end(), [](const Peak &a, const Peak &b) { return a.area > b.area; });

        // Match peaks
        int num_peaks = std::min(x_peaks.size(), y_peaks.size());
        for (int i = 0; i < num_peaks; ++i) {
            matched_peaks.push_back({x_peaks[i], y_peaks[i]});
        }

        return matched_peaks;
    }

    std::vector<PeakPair> match_peaks_area_comparison(std::vector<Peak> x_peaks,
                                                      std::vector<Peak> y_peaks) {
        struct Match {
            Peak x_peak;
            Peak y_peak;
            double area_difference;

            Match(Peak x_peak, Peak y_peak)
                    : x_peak(x_peak), y_peak(y_peak), area_difference(std::abs(x_peak.area - y_peak.area)) {}

            bool operator<(const Match &other) const {
                return area_difference > other.area_difference;
            }
        };

        std::priority_queue<Match> matches;
        for (const auto &x_peak: x_peaks) {
            for (const auto &y_peak: y_peaks) {
                matches.push(Match(x_peak, y_peak));
            }
        }

        std::vector<PeakPair> matched_peaks;
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


    std::vector<Peak>
    find_common_peaks(const std::vector<std::vector<double>> &time_slices, const std::vector<double> &noise_data, double n_sigmas, int min_width, int min_distance, double max_time_shift) {
        // Step 1: Detect peaks in each time slice
        std::vector<std::vector<Peak>> all_peaks;
        for (auto &slice: time_slices) {
            all_peaks.push_back(find_peaks(slice, noise_data, n_sigmas, min_width, min_distance));
        }

        std::vector<Peak> common_peaks;

        // Step 2: Check for common peaks
        // Step 2: Check for common peaks
        for (size_t i = 0; i < all_peaks.size(); ++i) {
            for (auto &peak: all_peaks[i]) {
                bool common = true;

                // Check if this peak exists in all other time slices
                for (size_t j = 0; j < all_peaks.size(); ++j) {
                    if (i != j) {
                        if (std::none_of(all_peaks[j].begin(), all_peaks[j].end(),
                                         [&peak, max_time_shift](const Peak &other_peak) {
                                             return abs(peak.index - other_peak.index) <= max_time_shift;
                                         })) {
                            common = false;
                            break;
                        }
                    }
                }

                if (common) {
                    // This peak is common to all time slices, add it to the list
                    common_peaks.push_back(peak);
                }
            }
        }

        // Step 3: Keep only the peak with the maximum amplitude for each position
        std::vector<Peak> max_amplitude_peaks;
        for (auto &peak: common_peaks) {
            auto it = std::find_if(max_amplitude_peaks.begin(), max_amplitude_peaks.end(),
                                   [&peak, max_time_shift](const Peak &max_peak) {
                                       return abs(max_peak.index - peak.index) <= max_time_shift;
                                   });

            if (it == max_amplitude_peaks.end()) {
                // This position is not in the list yet, add this peak
                max_amplitude_peaks.push_back(peak);
            } else {
                // This position is already in the list, update the peak if this one has a higher amplitude
                if (peak.height > it->height) {
                    *it = peak;
                }
            }
        }

        return max_amplitude_peaks;
    }
}