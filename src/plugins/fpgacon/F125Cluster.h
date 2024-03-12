//
// Created by Dmitry Romanov on 3/8/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once


namespace ml4fpga::fpgacon {

    struct F125Cluster {
        int id = 0;
        float pos_x = 0;
        float pos_y = 0;
        float pos_z = 0;
        float dedx = 0;
        float size;
        float width[3];     // y1, y2, dy ; strips
        float length[3];    // x1, x2, dx ; time
    };

}
