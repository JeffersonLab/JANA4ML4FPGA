// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

namespace ml4fpga::gem {
    struct Constants{
        static const int ChannelsCount = 128;

        static constexpr int ApvChannelCorrection(int channel) {
            return (32 * (channel % 4)) + (8 * (int) (channel / 4)) - (31 * (int) (channel / 16));
        }
    };
}
