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

        static constexpr double CalculateStripPosition(int strip_id, double plane_size, int num_apv_on_plane) {
            const double pitch =  plane_size / ( ChannelsCount * num_apv_on_plane);
            return -0.5 * (plane_size - pitch) + (pitch * strip_id);
        }
    };
}
