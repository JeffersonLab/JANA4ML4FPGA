
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#include <rawdataparser/EVIOBlockedEventParser.h>
#include "EVIOBlockedEventSource.h"

using namespace std;

////-----------------------------------------
//// SetEVIOFileName
////-----------------------------------------
//void EVIOBlockedEventSource::SetEVIOFileName(std::string filename){
//    m_filename = filename;
//}
//
////-----------------------------------------
//// OpenEVIOFile
////-----------------------------------------
//void EVIOBlockedEventSource::OpenEVIOFile() {
//    m_hdevio = new HDEVIO(m_filename, true, 2);   // 2 for VERBOSE level
//    if (!m_hdevio->is_open) {
//        cerr << m_hdevio->err_mess.str() << endl;
//        throw JException("Failed to open EVIO file: " + m_filename, __FILE__, __LINE__);
//        // throw exception indicating error
//    }
//    LOG_INFO(m_logger) << "Success opening event source \"" << m_filename << "\"!" << LOG_END;
//}
//
////-----------------------------------------
//// Initialize
////-----------------------------------------
//void EVIOBlockedEventSource::Initialize() {
//
//    LOG_INFO(m_logger) << "Initializing EVIOBlockedEventSource" << LOG_END;
//
//    // Open file.
//    // n.b. his is not fully integrated into the standard JANA
//    // application at this point we are not getting passed the
//    // event source names like JEventSource classes are.
//    EVIOBlockedEventSource::SetEVIOFileName(TEST_EVIO_FILE);
//    EVIOBlockedEventSource::OpenEVIOFile();
//}
//
////-----------------------------------------
//// ReadOneBlock
////-----------------------------------------
//EVIOBlockedEventSource::Status EVIOBlockedEventSource::ReadOneBlock(EVIOBlockedEvent& block) {
//
//    // Read buffer containing blocked event into the given "block" object.
//    //
//    // If we are unable to read a new block, but may be able to later
//    // (e.g. network source) then return Status::FailTryAgain
//    //
//    // If we are unable to read a new block and will never get another
//    // (e.g. end of file) then return Status::FailFinish
//    //
//    // If we successfully read a block then retun Status::Success
//    //
//    EVIOBlockedEventSource::Status status;
//
//    if(! m_buff) m_buff = new uint32_t[m_buff_len];
//    bool read_status = m_hdevio->readNoFileBuff(m_buff, m_buff_len, true);  // true is default allow_swap value
//    uint32_t cur_len = m_hdevio->last_event_len;
//
//    if (read_status) { // read OK, copy the data from buff to EVIOBlockedEvent
//        block.data.reserve(cur_len);
//        copy(m_buff, m_buff + cur_len, back_inserter(block.data));
//        block.swap_needed = m_hdevio->swap_needed;
//
//        status = EVIOBlockedEventSource::Status::Success;
//    } else {
//        // If the above read failed then free the buffer in all cases
//        delete[] m_buff;
//        m_buff = nullptr;
//
//        if (m_hdevio->err_code == HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL) {
//            m_buff_len = cur_len;  // prepare to re-allocate correct size buffer on next call
//            status = EVIOBlockedEventSource::Status::FailTryAgain;
//        } else if (m_hdevio->err_code == HDEVIO::HDEVIO_EOF) {
//            LOG_DEBUG(m_logger) << "No more events in file." << LOG_END;
//            status = EVIOBlockedEventSource::Status::FailFinished;
//            japp->Quit(true); // FIXME: We should NOT need t ocall this! JANA does not seem to accept the FailedFinished status as a condition to end.
//        } else { // keep it simple for now
//            throw JException("Unhandled read status: " + m_filename, __FILE__, __LINE__);
//        }
//    }
//
//    return status;
//}
//
////-----------------------------------------
//// NextBlock
////-----------------------------------------
//EVIOBlockedEventSource::Status EVIOBlockedEventSource::NextBlock(EVIOBlockedEvent& block) {
//
//    return EVIOBlockedEventSource::ReadOneBlock(block);
//}

//-----------------------------------------
// DisentangleBlock
// Parsing part goes here. Extract JObjects from blocks. Good luck!
//-----------------------------------------
std::vector <std::shared_ptr<JEvent>>
EVIOBlockedEventSource::DisentangleBlock(EVIOBlockedEvent &block, JEventPool &pool) {
    // Disentangle block into multiple events
    EVIOBlockedEventParser parser; // TODO: make this persistent
    return parser.ParseEVIOBlockedEvent(block, pool);
}

////-----------------------------------------
//// ~EVIOBlockedEventSource
////-----------------------------------------
//EVIOBlockedEventSource::~EVIOBlockedEventSource() {
//    LOG_INFO(m_logger) << "Closing hdevio event source \"" << m_filename << "\"" << LOG_END;
//    m_filename = UNSET_EVIO_FILE;
//
//    // Delete HDEVIO and print stats
//    if (m_hdevio) {
//        m_hdevio->PrintStats();
//        delete[] m_hdevio;
//    }
//    if (m_buff) {
//        delete[] m_buff;
//    }
//}
