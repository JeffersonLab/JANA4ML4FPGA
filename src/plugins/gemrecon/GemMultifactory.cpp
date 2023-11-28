//
// Created by romanov on 11/21/2023.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "GemMultifactory.h"

#include "DecodedData.h"
#include "PlanePeak.h"
#include "SFclust.h"


namespace ml4fpga {
namespace gem {


    GemMultifactory::GemMultifactory() {
        // We need to declare upfront what our Multifactory produces.
        DeclareOutput<SFclust>("");
        DeclareOutput<PlaneDecodedData>("");
        DeclareOutput<PlanePeak>("");
    }

    void GemMultifactory::Process(const std::shared_ptr<const JEvent>& element) {


    }
} // gem
} // ml4fpga