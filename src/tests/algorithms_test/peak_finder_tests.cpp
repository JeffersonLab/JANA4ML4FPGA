#include "catch_amalgamated.hpp"
#include <extmath/PeakFinder.h>


TEST_CASE("PeakFinder is tested", "[PeakFinder]") {
    // Initialize PeakFinder
    PeakFinder peakFinder;

    // Test data
    std::vector<double> input_data = {1.0, 2.0, 3.0, 2.0, 1.0, 0.5, 1.0, 2.0, 3.0, 2.0, 1.0};
    std::vector<double> noise_data = {0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2};
    double N = 3.0;
    int min_width = 2;
    int min_distance = 2;
    double min_prominence = 0.5;

    SECTION("find_peaks is tested") {
        std::vector<SingleDimensionPeakData> peaks = peakFinder.find_peaks(input_data, noise_data, N, min_width, min_distance);
        REQUIRE(peaks.size() == 2);  // Change as per your expectations
        // Add more REQUIRE statements as per your expectations
    }

    SECTION("match_peaks is tested") {
        std::vector<SingleDimensionPeakData> x_peaks = peakFinder.find_peaks(input_data, noise_data, N, min_width, min_distance);
        std::vector<SingleDimensionPeakData> y_peaks = peakFinder.find_peaks(input_data, noise_data, N, min_width, min_distance);
        std::vector<Peak> matched_peaks = peakFinder.match_peaks(x_peaks, y_peaks, PeakFindingMode::AUTO);
        REQUIRE(matched_peaks.size() == 2);  // Change as per your expectations
        // Add more REQUIRE statements as per your expectations
    }
}