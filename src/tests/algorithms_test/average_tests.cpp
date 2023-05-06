#include "catch_amalgamated.hpp"
#include <extmath/Average.h>

using namespace Catch;
using namespace ml4fpga::extmath;

TEST_CASE("Average add and mean calculation", "[Average]") {
Average average;

REQUIRE_THROWS_AS(average.mean(), std::runtime_error);

average.add(1.0);
REQUIRE(average.mean() == Approx(1.0));

average.add(2.0);
REQUIRE(average.mean() == Approx(1.5));

average.add(3.0);
REQUIRE(average.mean() == Approx(2.0));

average.add(4.0);
REQUIRE(average.mean() == Approx(2.5));
}

TEST_CASE("Average handles negative numbers", "[Average]") {
Average average;

average.add(-1.0);
average.add(2.0);
average.add(-3.0);
REQUIRE(average.mean() == Approx(-0.6666666666667));

average.add(4.0);
REQUIRE(average.mean() == Approx(0.5));
}

TEST_CASE("Average handles large numbers", "[Average]") {
Average average;

average.add(1e12);
average.add(2e12);
average.add(3e12);
REQUIRE(average.mean() == Approx(2e12));

average.add(4e12);
REQUIRE(average.mean() == Approx(2.5e12));
}

TEST_CASE("Average implicit conversion to double", "[Average]") {
Average average;

average.add(1.0);
average.add(2.0);
average.add(3.0);

double avg = average;
REQUIRE(avg == Approx(2.0));
}