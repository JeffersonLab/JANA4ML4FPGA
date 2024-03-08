#pragma once

#include <iostream>
#include <deque>
#include <numeric> // For std::accumulate
#include <type_traits> // For std::is_integral

namespace ml4fpga::extmath {

template<typename T = double>
class RollingAverage {
    static_assert(!std::is_integral<T>::value || sizeof(T) != 0, 
                  "MovingAverage cannot be instantiated with zero-sized types or integral without explicit handling for division.");

public:
    explicit RollingAverage(size_t capacity)
        : m_capacity(capacity), m_sum(0) {
        if (capacity == 0) {
            throw std::invalid_argument("Capacity must be greater than zero.");
        }
    }

    void add(T value) {
        if (m_values.size() == m_capacity) {
            m_sum -= m_values.front();
            m_values.pop_front();
        }

        m_values.push_back(value);
        m_sum += value;
    }

    [[nodiscard]] double average() const {
        if (m_values.empty()) return 0.0; // Return 0.0 if no elements have been added
        
        // If T is an integral type, this ensures that we perform floating-point division
        return static_cast<double>(m_sum) / m_values.size();
    }

private:
    size_t m_capacity;
    std::deque<T> m_values;
    T m_sum;
};

}   // namespace