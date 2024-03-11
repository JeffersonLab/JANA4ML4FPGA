//
// Created by Dmitry Romanov on 3/8/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <extensions/jana/CozyFactory.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <JANA/JFactoryT.h>

#include "F125Cluster.h"
#include "FpgaTrackFit.h"
#include "FpgaHitsToTrack.h"

namespace ml4fpga::fpgacon {
    class FpgaExchangeFactory : public CozyFactory<>
    {
    public:
        FpgaExchangeFactory() = default;
        void CozyInit() override;
        void CozyProcess(uint64_t run_number, uint64_t event_number) override;


        Input<F125Cluster> m_input_clusters;
        Output<FpgaHitsToTrack> m_output_hits_to_track;
        Output<FpgaTrackFit> m_output_trak_fit;

    private:
        int m_cfg_use_tcp; // Send messages to FPGA via TCP
        size_t m_cfg_fpga_max_hits = 50;   // Max hits to be sent to FPGA

    };
}

