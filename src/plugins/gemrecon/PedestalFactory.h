#pragma once

#include <JANA/JFactoryT.h>
#include "extensions/spdlog/SpdlogMixin.h"
#include "Pedestal.h"
#include "libraries/extmath/Average.h"
#include "libraries/extmath/StandardDeviation.h"

namespace ml4fpga::gem {

    class PedestalFactory:
            public JFactoryT<ml4fpga::gem::Pedestal>,
            public spdlog::extensions::SpdlogMixin<PedestalFactory>  {
    public:
        PedestalFactory()=default;
        void Init() override;
        void BeginRun(const std::shared_ptr<const JEvent>&) override;
        void Process(const std::shared_ptr<const JEvent>&) override;
        void Finish() override;
    private:
        /// Array of average channels for each APV
        std::map<const int, std::vector<extmath::Average>> m_apv_averages;
        std::map<const int, std::vector<extmath::StandardDeviation>> m_apv_std;

        /// Number of events to average Pedestal
        size_t m_events_to_average = 500;
        size_t m_minimal_events = 15;
        double m_constant_offset = 3000;
        double m_constant_noise = 0;
        size_t m_events_counter = 0;
    };


} // ml4fpga


