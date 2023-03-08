//
// Created by xmei on 2/24/23.
//

#include "EVIOBlockProcessor.h"


EVIOBlockProcessor::EVIOBlockProcessor() {
    SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
    InitLogger("jana4ml4fpga");
}

void EVIOBlockProcessor::Init() {
    m_log->debug("EVIOBlockProcessor::Init");
}


void EVIOBlockProcessor::Process(const std::shared_ptr<const JEvent> &event) {
    m_log->debug("EVIOBlockProcessor::Process");
    // auto objs = event->Get<MyObject>();
    // std::lock_guard<std::mutex>lock(m_mutex);

    // for (const MyObject* obj : objs) {
    //     LOG << obj->datum << LOG_END;
    // }
}

void EVIOBlockProcessor::Finish() {
    m_log->debug("EVIOBlockProcessor::Finish");
    // Close any resources
    // LOG << "EVIOBlockProcessor::Finish" << LOG_END;
}
