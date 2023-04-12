#include <JANA/JFactoryT.h>

#pragma once

#include "GEMCluster.h"
#include <extensions/spdlog/SpdlogMixin.h>

namespace ml4fpga {

    class SrsGEMDecoder_factory:public JFactoryT<GEMCluster>,
                                public spdlog::extensions::SpdlogMixin<SrsGEMDecoder_factory>   // this automates proper Log initialization
    {
    };

} // ml4fpga


