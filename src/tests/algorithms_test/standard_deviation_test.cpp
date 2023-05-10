#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include <extmath/StandardDeviation.h>

using namespace ml4fpga::extmath;
using namespace Catch;

TEST_CASE("StandardDeviation add and stddev calculation", "[StandardDeviation]") {
StandardDeviation stddev;

REQUIRE_THROWS_AS(stddev.stddev(), std::runtime_error);

stddev.add(1.0);
REQUIRE_THROWS_AS(stddev.stddev(), std::runtime_error);

stddev.add(2.0);
REQUIRE(stddev.stddev() == Approx(0.707106781));

stddev.add(3.0);
REQUIRE(stddev.stddev() == Approx(1.0));

stddev.add(4.0);
REQUIRE(stddev.stddev() == Approx(1.290994449));
}

TEST_CASE("StandardDeviation handles negative numbers", "[StandardDeviation]") {
StandardDeviation stddev;

stddev.add(-1.0);
stddev.add(2.0);
REQUIRE(stddev.stddev() == Approx(2.121320344));

stddev.add(-3.0);
stddev.add(4.0);
REQUIRE(stddev.stddev() == Approx(2.828427125));
}

TEST_CASE("StandardDeviation handles large numbers", "[StandardDeviation]") {
StandardDeviation stddev;

stddev.add(1e12);
stddev.add(2e12);
stddev.add(3e12);
REQUIRE(stddev.stddev() == Approx(1e12));

stddev.add(4e12);
REQUIRE(stddev.stddev() == Approx(1.290994449e12));
}

TEST_CASE("StandardDeviation implicit conversion to double", "[StandardDeviation]") {
StandardDeviation stddev;

stddev.add(1.0);
stddev.add(2.0);
stddev.add(3.0);

double sd = stddev;
REQUIRE(sd == Approx(1.0));
}