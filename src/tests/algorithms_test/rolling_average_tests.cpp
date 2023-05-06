//
// Created by romanov on 5/5/2023.
//
#include "catch_amalgamated.hpp"
#include "extmath/RollingAverage.h"

using namespace ml4fpga::extmath;
using namespace Catch;

TEST_CASE("RollingMean constructor and window size validation", "[RollingMean]") {
REQUIRE_THROWS_AS(RollingAverage(0), std::invalid_argument);
REQUIRE_NOTHROW(RollingAverage(1));
}

TEST_CASE("RollingMean add and mean calculation", "[RollingMean]") {
RollingAverage rollingMean(3);

REQUIRE_THROWS_AS(rollingMean.mean(), std::runtime_error);

rollingMean.add(1.0);
REQUIRE(rollingMean.mean() == CATCH_APPROX_HPP_INCLUDED (1.0));

rollingMean.add(2.0);
REQUIRE(rollingMean.mean() == Approx(1.5));

rollingMean.add(3.0);
REQUIRE(rollingMean.mean() == Approx(2.0));

rollingMean.add(4.0);
REQUIRE(rollingMean.mean() == Approx(3.0));
}

TEST_CASE("RollingMean handles negative numbers", "[RollingMean]") {
RollingAverage rollingMean(3);

rollingMean.add(-1.0);
rollingMean.add(2.0);
rollingMean.add(-3.0);
REQUIRE(rollingMean.mean() == Approx(-0.6666666666667));

rollingMean.add(4.0);
REQUIRE(rollingMean.mean() == Approx(1.0));
}

TEST_CASE("RollingMean handles large numbers", "[RollingMean]") {
RollingAverage rollingMean(3);

rollingMean.add(1e12);
rollingMean.add(2e12);
rollingMean.add(3e12);
REQUIRE(rollingMean.mean() == Approx(2e12));

rollingMean.add(4e12);
REQUIRE(rollingMean.mean() == Approx(3e12));
}

TEST_CASE("RollingMean handles more values than window size", "[RollingMean]") {
    RollingAverage rollingMean(3);

    rollingMean.add(1.0);
    rollingMean.add(2.0);
    rollingMean.add(3.0);
    REQUIRE(rollingMean.mean() == Approx(2.0));

    rollingMean.add(4.0);
    REQUIRE(rollingMean.mean() == Approx(3.0));

    rollingMean.add(5.0);
    REQUIRE(rollingMean.mean() == Approx(4.0));
}