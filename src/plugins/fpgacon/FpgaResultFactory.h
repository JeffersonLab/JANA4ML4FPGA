//
// Created by Dmitry Romanov on 3/8/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <extensions/spdlog/SpdlogMixin.h>
#include <JANA/JFactoryT.h>
#include "FpgaResult.h"

namespace ml4fpga::fpgacon {
    class FpgaResultFactory :
        public JFactoryT<FpgaResult>,
        public spdlog::extensions::SpdlogMixin<FpgaResultFactory>
    {
    public:
        FpgaResultFactory() = default;
        void Init() override;
        void Process(const std::shared_ptr<const JEvent>&) override;
        void Finish() override;
    private:
        int m_cfg_use_tcp; // Send messages to FPGA via TCP

    };
}

