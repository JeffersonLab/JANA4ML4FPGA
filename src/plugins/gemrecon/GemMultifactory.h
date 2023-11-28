//
// Created by romanov on 11/21/2023.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <JANA/JMultifactory.h>


namespace ml4fpga {
namespace gem {

    class GemMultifactory : public JMultifactory {
    public:
        GemMultifactory();

        void Process(const std::shared_ptr<const JEvent>&) override;
    };

} // gem
} // ml4fpga
