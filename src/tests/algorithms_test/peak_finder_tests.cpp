#include "catch_amalgamated.hpp"
#include <extmath/PeakFinder.h>


TEST_CASE("PeakFinder is tested", "[PeakFinder]") {
    // Initialize PeakFinder
    using namespace Catch;


    // Test data
    std::vector<double> input_data = {1.0, 2.0, 3.0, 2.0, 1.0, 0.5, 1.0, 2.0, 3.0, 2.0, 1.0};
    std::vector<double> noise_data = {0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2};
    double N = 3.0;
    int min_width = 1;
    int min_distance = 0;


    SECTION("find_peaks is tested") {
        std::vector<SingleDimensionPeakData> peaks = find_peaks(input_data, noise_data, N, min_width, min_distance);
        // Check that the output matches the expected results
        REQUIRE(peaks.size() == 2);

        REQUIRE(peaks[0].index == Approx(2.0).margin(0.1));
        REQUIRE(peaks[0].width == Approx(5.0).margin(0.1));
        REQUIRE(peaks[0].height == Approx(3.0).margin(0.1));

        REQUIRE(peaks[1].index == Approx(8.0).margin(0.1));
        REQUIRE(peaks[1].width == Approx(5.0).margin(0.1));
        REQUIRE(peaks[1].height == Approx(3.0).margin(0.1));
    }

    SECTION("match_peaks is tested") {
        std::vector<SingleDimensionPeakData> x_peaks = find_peaks(input_data, noise_data, N, min_width, min_distance);
        std::vector<SingleDimensionPeakData> y_peaks = find_peaks(input_data, noise_data, N, min_width, min_distance);
        std::vector<Peak> matched_peaks = match_peaks(x_peaks, y_peaks, PeakFindingMode::AUTO);
        REQUIRE(matched_peaks.size() == 2);  // Change as per your expectations
        // Add more REQUIRE statements as per your expectations
    }
}


TEST_CASE("find_common_peaks works correctly", "[peak_finding]") {

    using namespace Catch;

    // Create a PeakFinder


    // Create some mock time slice data
    std::vector<std::vector<double>> time_slices = {
            {1, 2, 3, 2, 1, 0, 1, 2, 5, 2, 1, 0, 1, 2, 3, 2, 1},
            {1, 2, 3, 2, 1, 0, 1, 2, 7, 2, 1, 0, 1, 2, 3, 2, 1},
            {1, 2, 4, 2, 1, 0, 1, 2, 5, 2, 1, 0, 0, 0, 0, 0, 0}
    };
    std::vector<double> noise_data = {0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2};
    double N = 3.0;
    int min_width = 2;
    int min_distance = 2;

    // Run find_common_peaks
    std::vector<SingleDimensionPeakData> common_peaks = find_common_peaks(time_slices,  noise_data, N, min_width, min_distance,  1);

    // Check that the output matches the expected results
    REQUIRE(common_peaks.size() == 2);

    REQUIRE(common_peaks[0].index == Approx(2.0).margin(0.1));
    REQUIRE(common_peaks[0].width == Approx(5.0).margin(0.1));
    REQUIRE(common_peaks[0].height == Approx(4.0).margin(0.1));

    REQUIRE(common_peaks[1].index == Approx(8.0).margin(0.1));
    REQUIRE(common_peaks[1].width == Approx(4.0).margin(0.1));
    REQUIRE(common_peaks[1].height == Approx(7.0).margin(0.1));
}