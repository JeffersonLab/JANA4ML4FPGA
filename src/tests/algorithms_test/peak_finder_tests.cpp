#include "catch_amalgamated.hpp"
#include <extmath/PeakFinder.h>

using namespace ml4fpga::extmath;

TEST_CASE("find_peaks is tested", "[PeakFinder]") {
    // Initialize PeakFinder
    using namespace Catch;

    // Test data
    std::vector<double> input_data = {1.0, 2.0, 3.0, 2.0, 1.0, 0.5, 1.0, 2.0, 3.0, 2.0, 1.0};
    std::vector<double> noise_data = {0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2};
    double N = 3.0;
    int min_width = 1;
    int min_distance = 0;


    SECTION("find_peaks is tested") {
        std::vector<Peak> peaks = find_peaks(input_data, noise_data, N, min_width, min_distance);
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
        std::vector<Peak> x_peaks = find_peaks(input_data, noise_data, N, min_width, min_distance);
        std::vector<Peak> y_peaks = find_peaks(input_data, noise_data, N, min_width, min_distance);
        std::vector<PeakPair> matched_peaks = match_peaks(x_peaks, y_peaks, PeakFindingMode::AUTO);
        REQUIRE(matched_peaks.size() == 2);  // Change as per your expectations
        // Add more REQUIRE statements as per your expectations
    }
}


TEST_CASE("combine_common_peaks works correctly", "[PeakFinder]") {

    using namespace Catch;

    // Create a PeakFinder

    // Create some mock time slice data
    std::vector<std::vector<double>> time_slices = {
        {1, 2, 3, 2, 1, 0, 1, 2, 5, 2, 1, 0, 1, 2, 3, 2, 1},
        {1, 2, 3, 2, 1, 0, 1, 2, 7, 2, 1, 0, 1, 2, 3, 2, 1},
        {1, 2, 4, 2, 1, 0, 1, 2, 5, 2, 1, 0, 0, 0, 0, 0, 0}
    };
    std::vector<double> noise_data = {0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2};
    double n_sigmas = 3.0;
    int min_width = 2;
    int min_distance = 2;
    int max_channel_shift = 2;

    SECTION("combine_common_peaks is tested") {
        std::vector<std::vector<Peak>> peaks_by_time;
        for (size_t time_slice=0; time_slice < time_slices.size(); time_slice++) {
            auto& slice = time_slices[time_slice];
            auto found_peaks = find_peaks(slice, noise_data, n_sigmas, min_width, min_distance);
            for (auto& peak: found_peaks) {
                peak.time_index = time_slice;
            }
            peaks_by_time.push_back(found_peaks);
        }

        // Run find_common_peaks
        auto combined_peaks = combine_common_peaks(peaks_by_time, max_channel_shift);

        // Check that the output matches the expected results
        REQUIRE(combined_peaks.size() == 3);
        REQUIRE(combined_peaks[0].size() == 3);
        REQUIRE(combined_peaks[1].size() == 3);
        REQUIRE(combined_peaks[2].size() == 2);
    }

    SECTION("find_common_peaks tested") {
        // Run find_common_peaks
        std::vector<Peak> common_peaks = find_common_peaks(time_slices, noise_data, n_sigmas, min_width, min_distance, max_channel_shift);

        // Check that the output matches the expected results
        REQUIRE(common_peaks.size() == 2);

        REQUIRE(common_peaks[0].index == 2);
        REQUIRE(common_peaks[0].width == Approx(5.0).margin(0.1));
        REQUIRE(common_peaks[0].height == Approx(4.0).margin(0.1));
        REQUIRE(common_peaks[0].time_index == 2);


        REQUIRE(common_peaks[1].index == Approx(8.0).margin(0.1));
        REQUIRE(common_peaks[1].width == Approx(4.0).margin(0.1));
        REQUIRE(common_peaks[1].height == Approx(7.0).margin(0.1));
        REQUIRE(common_peaks[1].time_index == 1);
    }
}