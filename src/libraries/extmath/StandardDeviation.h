//
// Created by romanov on 5/9/2023.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <stdexcept>
#include <cmath>

namespace ml4fpga::extmath {
    class StandardDeviation {
    public:
        StandardDeviation() : sum(0), sumSquares(0), count(0) {}

        void add(double value) {
            sum += value;
            sumSquares += value * value;
            count++;
        }

        double stddev() const {
            if (count < 2) {
                return 0;
            }
            double mean = sum / count;
            double variance = (sumSquares / count) - (mean * mean);
            return std::sqrt(variance);
        }

        operator double() const {
            return stddev();
        }

    private:
        double sum;
        double sumSquares;
        size_t count;
    };
}