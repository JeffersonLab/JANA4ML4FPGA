//
// Created by romanov on 6/17/2023.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef JANA4ML4FPGA_APVDECODINGRESULT_H
#define JANA4ML4FPGA_APVDECODINGRESULT_H

#include "SFclust.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "GemReconDqmProcessor.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "plugins/gemrecon/old_code/GemConfiguration.h"
#include "plugins/gemrecon/old_code/GEMPedestal.h"
#include <TH2F.h>
#include "plugins/gemrecon/old_code/GEMCluster.h"
#include <JANA/JFactoryT.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JMultifactory.h>

namespace ml4fpga::gem {
    class GemApvDecodingResult {
    public:
        std::vector<float> PedestalOffsets;
        std::vector<float> PedestalNoises;
        std::vector<std::vector<int>> RawData;
        std::vector<double> CommonModeOffsets;
        std::vector<double> RawDataAverage;
    };
}


#endif //JANA4ML4FPGA_APVDECODINGRESULT_H
