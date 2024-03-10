//
// Created by Dmitry Romanov on 3/8/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <extensions/jana/CozyFactory.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <JANA/JFactoryT.h>
#include "FpgaHitsToTrack.h"

namespace ml4fpga::fpgacon {
    class FpgaExchangeFactory : public CozyFactory<>
    {
    public:
        FpgaExchangeFactory() = default;
        void Init() override;
        void Process(const std::shared_ptr<const JEvent>&) override;

    private:
        int m_cfg_use_tcp; // Send messages to FPGA via TCP
        size_t m_cfg_fpga_max_hits = 50;   // Max hits to be sent to FPGA

    };
}

