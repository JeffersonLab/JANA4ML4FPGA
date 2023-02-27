//
// Created by xmei on 2/24/23.
//

#include "EVIOBlockProcessor.h"

#include "MyObject.h"
#include <JANA/JLogger.h>

EVIOBlockProcessor::EVIOBlockProcessor() {
    SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
}

void EVIOBlockProcessor::Init() {
    LOG << "EVIOBlockProcessor::Init" << LOG_END;
    // Open TFiles, set up TTree branches, etc
}


void EVIOBlockProcessor::Process(const std::shared_ptr<const JEvent> &event) {
    _DBG__;
}

void EVIOBlockProcessor::Finish() {
    // Close any resources
    LOG << "EVIOBlockProcessor::Finish" << LOG_END;
}
