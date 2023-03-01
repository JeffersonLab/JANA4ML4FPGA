//
// Created by xmei on 2/24/23.
//

#include "EVIOBlockProcessor.h"

// #include "MyObject.h"
#include <JANA/JLogger.h>

EVIOBlockProcessor::EVIOBlockProcessor() {
    SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
}

void EVIOBlockProcessor::Init() {
    LOG << "EVIOBlockProcessor::Init" << LOG_END;
    // Open TFiles, set up TTree branches, etc
}


void EVIOBlockProcessor::Process(const std::shared_ptr<const JEvent> &event) {
    LOG << "EVIOBlockProcessor::Process, Event #" << event->GetEventNumber() << LOG_END;

    // auto objs = event->Get<MyObject>();
    // std::lock_guard<std::mutex>lock(m_mutex);

    // for (const MyObject* obj : objs) {
    //     LOG << obj->datum << LOG_END;
    // }
}

void EVIOBlockProcessor::Finish() {
    // Close any resources
    LOG << "EVIOBlockProcessor::Finish" << LOG_END;
}
