//
// Created by romanov on 5/5/2023.
//
#pragma once

#include <iostream>
#include <deque>
#include <stdexcept>

namespace ml4fpga::extmath {

    class RollingAverage {
    public:
        explicit RollingAverage(size_t windowSize) : windowSize(windowSize), currentSum(0), currentCount(0) {
            if (windowSize == 0) {
                throw std::invalid_argument("windowSize must be greater than 0");
            }
        }

        void add(double value) {
            buffer.push_back(value);
            currentSum += value;

            if (currentCount < windowSize) {
                currentCount++;
            } else {
                currentSum -= buffer.front();
                buffer.pop_front();
            }
        }

        double mean() const {
            if (currentCount == 0) {
                throw std::runtime_error("No values added yet");
            }
            return currentSum / currentCount;
        }

    private:
        size_t windowSize;
        std::deque<double> buffer;
        double currentSum;
        size_t currentCount;
    };
}