
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "EVIOBlockedEventSource.h"

//-----------------------------------------
// Initialize
//-----------------------------------------
virtual void EVIOBlockedEventSource::Initialize() {
    LOG_INFO(m_logger) <<  "Initializing JBlockedEventSource" << LOG_END;

    // Open file.
    // n.b. his is not fully integrated into the standard JANA
    // application at this point so we are not getting passed the
    // event source names like JEventSource classes are.
    m_filename = TEST_EVIO_FILE;
    evio = new HDEVIO(m_filename, false, VERBOSE);

}

//-----------------------------------------
// NextBlock
//-----------------------------------------
virtual Status EVIOBlockedEventSource::NextBlock(EVIOBlockedEvent& block) {
    LOG_DEBUG(m_logger) <<  "JBlockedEventSource::NextBlock" << LOG_END;

    // Read buffer containing blocked event into the given "block" object.
    //
    // If we are unable to read a new block, but may be able to later
    // (e.g. network source) then return Status::FailTryAgain
    //
    // If we are unable to read a new block and will never get another
    // (e.g. end of file) then return Status::FailFinish
    //
    // If we successfully read a block then retun Status::Success
    //
    // HDEVIO->readNoFileBuff(block.buff, block.buff_len);


    return Status::Success;
}

//-----------------------------------------
// DisentangleBlock
//-----------------------------------------
virtual std::vector<std::shared_ptr<JEvent>> EVIOBlockedEventSource::DisentangleBlock(MyBlock& block, JEventPool& pool) {

    // Disentangle block into multiple events

    // Insert all events into events from the event pool (pool will
    // create as needed.)
    std::vector<std::shared_ptr<JEvent>> events;
//		for (auto datum : block.data) {
//			auto event = pool.get(0);  // TODO: Make location be transparent to end user
//			event->Insert(new MyObject(datum));
//			events.push_back(event);
//		}

    // Insert events into the single events queue
    return events;
}

//-----------------------------------------
// ~EVIOBlockedEventSource
//-----------------------------------------
~EVIOBlockedEventSource(){
    LOG << "Closing hdevio event source \"" << m_filename << "\"" <<endl;

    // Delete HDEVIO and print stats
    if(evio){
        evio->PrintStats();
        delete evio;
    }
}
