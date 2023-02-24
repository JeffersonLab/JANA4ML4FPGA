
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "EVIOBlockedEventSource.h"

using namespace std;

void EVIOBlockedEventSource::SetEVIOFileName(std::string filename){
    m_filename = filename;
}

void EVIOBlockedEventSource::OpenEVIOFile() {
    m_hdevio = new HDEVIO(m_filename, true, 2);   // 2 for VERBOSE level
    if (!m_hdevio->is_open) {
        cerr << m_hdevio->err_mess.str() << endl;
        throw JException("Failed to open EVIO file: " + m_filename, __FILE__, __LINE__);
        // throw exception indicating error
    }
    LOG_INFO(m_logger) << "Success opening event source \"" << m_filename << "\"!" << LOG_END;
}

//-----------------------------------------
// Initialize
//-----------------------------------------
virtual void EVIOBlockedEventSource::Initialize() {

    LOG_INFO(m_logger) << "Initializing EVIOBlockedEventSource" << LOG_END;

    // Open file.
    // n.b. his is not fully integrated into the standard JANA
    // application at this point we are not getting passed the
    // event source names like JEventSource classes are.
    EVIOBlockedEventSource::SetEVIOFileName(TEST_EVIO_FILE);
    EVIOBlockedEventSource::OpenEVIOFile();
}

EVIOBlockedEventSource::Status EVIOBlockedEventSource::ReadOneBlock(EVIOBlockedEvent& block) {

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
    EVIOBlockedEventSource::Status status;

    m_buff = new uint32_t[m_buff_len];
    bool read_status = m_hdevio->readNoFileBuff(m_buff, m_buff_len, true);  // true is default allow_swap value
    uint32_t cur_len = m_hdevio->last_event_len;

    if (read_status) { // read OK, copy the data from buff to EVIOBlockedEvent
        block.data.reserve(cur_len);
        copy(m_buff, m_buff + cur_len, back_inserter(block.data));
        block.swap_needed = m_hdevio->swap_needed;

        status = EVIOBlockedEventSource::Status::Success;
        m_buff_len = DEFAULT_READ_BUFF_LEN;  // reset buff_len
    } else {

        if (m_hdevio->err_code == HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL) {
            m_buff_len = cur_len;
            status = EVIOBlockedEventSource::Status::FailTryAgain;
        } else if (m_hdevio->err_code == HDEVIO::HDEVIO_EOF) {
            LOG_DEBUG(m_logger) << "No more events in file!" << LOG_END;
            status = EVIOBlockedEventSource::Status::FailFinished;
        } else { // keep is simple first
            delete[] m_buff;
            throw JException("Unhandled read status: " + m_filename, __FILE__, __LINE__);
        }
    }

    delete[] m_buff; // reset buff
    return status;
}

//-----------------------------------------
// NextBlock
//-----------------------------------------
virtual EVIOBlockedEventSource::Status EVIOBlockedEventSource::NextBlock(EVIOBlockedEvent& block) {

    LOG_DEBUG(m_logger) << "EVIOBlockedEventSource::NextBlock" << LOG_END;

    return EVIOBlockedEventSource::ReadOneBlock(block);
}

//-----------------------------------------
// DisentangleBlock
// Parsing part goes here. Extract JObjects from blocks. Good luck!
//-----------------------------------------
virtual std::vector <std::shared_ptr<JEvent>>
EVIOBlockedEventSource::DisentangleBlock(MyBlock &block, JEventPool &pool) {

    // Disentangle block into multiple events

    // Insert all events into events from the event pool (pool will
    // create as needed.)
    std::vector <std::shared_ptr<JEvent>> events;
    // TODO
    // Insert events into the single events queue
//    for (auto datum : block.data) {
//        auto event = pool.get(0);  // TODO: Make location be transparent to end user
//        event->Insert(new MyObject(datum));
//        events.push_back(event);
//    }
    return events;
}

//-----------------------------------------
// ~EVIOBlockedEventSource
//-----------------------------------------
EVIOBlockedEventSource::~EVIOBlockedEventSource() {
    LOG_INFO << "Closing hdevio event source \"" << m_filename << "\"" << endl;
    m_filename = UNSET_EVIO_FILE;

    // Delete HDEVIO and print stats
    if (m_hdevio) {
        m_hdevio->PrintStats();
        delete[] m_hdevio;
    }
    if (m_buff) {
        delete[] m_buff;
    }

}
