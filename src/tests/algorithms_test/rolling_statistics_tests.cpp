#include <extmath/RollingStatistics.h>
#include "catch_amalgamated.hpp"


using namespace ml4fpga::extmath;
using namespace Catch;
TEST_CASE("RollingStatistics functionality", "[RollingStatistics]") {
    SECTION("Correctly calculates average and standard deviation with full window") {
        RollingStatistics<double> stats(5);
        stats.add(1);
        stats.add(2);
        stats.add(3);
        stats.add(4);
        stats.add(5); // Full window: [1, 2, 3, 4, 5]

        REQUIRE(stats.getAverage() == Approx(3.0));
        REQUIRE(stats.getStandardDeviation() == Approx(std::sqrt(2.0)));
    }

    SECTION("Updates calculations correctly when old elements are replaced") {
        RollingStatistics<double> stats(3);
        stats.add(2);
        stats.add(4);
        stats.add(4); // Full window: [2, 4, 4]
        REQUIRE(stats.getAverage() == Approx(3.33333333333));
        REQUIRE(stats.getStandardDeviation() == Approx(1.24721912892));

        stats.add(6); // Updated window: [4, 4, 6]
        REQUIRE(stats.getAverage() == Approx(4.66666666667));
        REQUIRE(stats.getStandardDeviation() == Approx(1.24721912892));
    }

    SECTION("Correctly handles a window of size 1") {
        RollingStatistics<double> stats(1);
        stats.add(10); // Single element window

        REQUIRE(stats.getAverage() == Approx(10.0));
        REQUIRE(stats.getStandardDeviation() == Approx(0.0)); // Standard deviation of a single value is 0
    }

    SECTION("Returns zero for average and standard deviation with no data") {
        RollingStatistics<double> stats(5);

        REQUIRE(stats.getAverage() == Approx(0.0));
        REQUIRE(stats.getStandardDeviation() == Approx(0.0));
    }
}