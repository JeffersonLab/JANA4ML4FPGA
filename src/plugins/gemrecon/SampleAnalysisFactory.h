//
// Created by romanov on 3/7/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <extensions/spdlog/SpdlogMixin.h>
#include <JANA/JFactoryT.h>

#include "SampleAnalysisData.h"


namespace ml4fpga::gem {

    class SampleAnalysisFactory  :
                public JFactoryT<SampleAnalysisData>,
                public spdlog::extensions::SpdlogMixin<SampleAnalysisFactory>  {
    public:
        SampleAnalysisFactory()=default;
        void Init() override;
        void Process(const std::shared_ptr<const JEvent>&) override;
        void Finish() override;


};

} // gem
// ml4fpga
