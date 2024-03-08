//
// Created by romanov on 2/27/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <extmath/Average.h>
#include <extmath/StandardDeviation.h>

#include "SampleData.h"

namespace ml4fpga::gem {
    class SampleAnalyzer {
    public:
        void clear() {

        }
        void add_data(const SampleData &data) {
            //m_average.add(data.);

        }
    private:
        /// Array of average channels for each APV
        extmath::Average m_average;
        extmath::StandardDeviation m_std;
    };
}
