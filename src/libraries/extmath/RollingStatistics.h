//
// Created by Dmitry Romanov on 3/7/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <iostream>
#include <deque>
#include <cmath> // For std::sqrt
#include <stdexcept> // For std::invalid_argument

namespace ml4fpga::extmath {
    template<typename T = double>
    class RollingStatistics {
    public:
        explicit RollingStatistics(size_t capacity=0)
            : m_capacity(capacity), m_sum(0), m_sum_of_squares(0) {
            if (capacity == 0) {

            }
        }

        void add(T value) {
            if(m_capacity == 0) return;
            if (m_values.size() == m_capacity) {
                T old_value = m_values.front();
                m_sum -= old_value;
                m_sum_of_squares -= old_value * old_value;
                m_values.pop_front();
            }

            m_values.push_back(value);
            m_sum += value;
            m_sum_of_squares += value * value;
        }

        double getAverage() const {
            if (m_values.empty()) return 0.0; // Return 0.0 if no elements
            return static_cast<double>(m_sum) / m_values.size();
        }

        double getStandardDeviation() const {
            if (m_values.size() <= 1) return 0.0; // Not enough data to compute standard deviation

            double mean = getAverage();
            double mean_of_squares = static_cast<double>(m_sum_of_squares) / m_values.size();
            double variance = mean_of_squares - (mean * mean);

            return std::sqrt(variance);
        }

    private:
        size_t m_capacity;
        std::deque<T> m_values;
        T m_sum;
        T m_sum_of_squares;
    };
}   // namespace