//
// Created by Dmitry Romanov on 3/8/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once


class TH2F;

namespace ml4fpga::fpgacon {

    struct F125ClusterContext {
        bool is_electron = false;
        TH2F* hevt = nullptr;
        TH2F* hevtc = nullptr;
    };

}