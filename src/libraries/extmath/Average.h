//
// Created by Dmitry Romanov on 5/5/2023.
//
#pragma once

#include <stdexcept>

namespace ml4fpga::extmath {

    class Average {
    public:
        Average() : currentSum(0), currentCount(0) {}

        void add(double value) {
            currentSum += value;
            currentCount++;
        }

        double mean() const {
            if (currentCount == 0) {
                throw std::runtime_error("No values added yet");
            }
            return currentSum / currentCount;
        }

        // User-defined conversion operator
        operator double() const {
            return mean();
        }

    private:
        double currentSum;
        size_t currentCount;
    };
}

