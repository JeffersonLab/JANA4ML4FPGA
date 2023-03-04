//
// Created by Xinxin Mei (xmei@jlab.org) on 3/3/23.
//

#ifndef JANA4ML4FPGA_CDAQ3WHEADER_H
#define JANA4ML4FPGA_CDAQ3WHEADER_H

#include <cinttypes>

#define   BORE_TRIGGERID  0x55555555
#define   INFO_TRIGGERID  0x5555AAAA
#define   EPICS_TRIGGERID 0xAAAA5555
#define   EORE_TRIGGERID  0xAAAAAAAA

/**
 * The 3-word header sent before every event block.
 */
class CDAQ3WHeader : {

public:
    uint32_t cdaq_header[3];

    uint32_t GetTriggerId() {
        return cdaq_header[1];
    }

    uint32_t GetModID() {
        return (cdaq_header[0] >> 24) & 0xff;
    }

    uint32_t GetEventSize() {
        return cdaq_header[2];
    }
};

#endif //JANA4ML4FPGA_CDAQ3WHEADER_H
