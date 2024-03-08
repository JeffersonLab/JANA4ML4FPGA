//
// Created by romanov on 3/7/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "SampleAnalysisFactory.h"

namespace ml4fpga {
namespace gem {
    void SampleAnalysisFactory::Init() {
        InitLogger(GetPluginName() + ":" + JTypeInfo::demangle<SampleAnalysisFactory>());
    }

    void SampleAnalysisFactory::Process(const std::shared_ptr<const JEvent>& element) {
        JFactoryT<SampleAnalysisData>::Process(element);
    }

    void SampleAnalysisFactory::Finish() {
        JFactoryT<SampleAnalysisData>::Finish();
    }
} // gem
} // ml4fpga