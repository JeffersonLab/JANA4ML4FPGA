//
// Created by xmei on 2/24/23.
//

#include "EVIOBlockProcessor.h"

<<<<<<< HEAD
// #include "MyObject.h"
=======
#include "MyObject.h"
>>>>>>> 19e12fd4a3cc53ec94a91c36a7df9bdfe74feac0
#include <JANA/JLogger.h>

EVIOBlockProcessor::EVIOBlockProcessor() {
    SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
}

void EVIOBlockProcessor::Init() {
    LOG << "EVIOBlockProcessor::Init" << LOG_END;
    // Open TFiles, set up TTree branches, etc
}

<<<<<<< HEAD
// The parsing etc code goes here?
void EVIOBlockProcessor::Process(const std::shared_ptr<const JEvent> &event) {
    LOG << "EVIOBlockProcessor::Process, Event #" << event->GetEventNumber() << LOG_END;

    // auto objs = event->Get<MyObject>();
    // std::lock_guard<std::mutex>lock(m_mutex);

    // for (const MyObject* obj : objs) {
    //     LOG << obj->datum << LOG_END;
    // }
=======
void EVIOBlockProcessor::Process(const std::shared_ptr<const JEvent> &event) {
    LOG << "EVIOBlockProcessor::Process, Event #" << event->GetEventNumber() << LOG_END;

    auto objs = event->Get<MyObject>();
    std::lock_guard<std::mutex>lock(m_mutex);

    for (const MyObject* obj : objs) {
        LOG << obj->datum << LOG_END;
    }
>>>>>>> 19e12fd4a3cc53ec94a91c36a7df9bdfe74feac0
}

void EVIOBlockProcessor::Finish() {
    // Close any resources
    LOG << "EVIOBlockProcessor::Finish" << LOG_END;
}
<<<<<<< HEAD

=======
>>>>>>> 19e12fd4a3cc53ec94a91c36a7df9bdfe74feac0
