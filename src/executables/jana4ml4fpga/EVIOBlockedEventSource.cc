
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#include <rawdataparser/EVIOBlockedEventParser.h>
#include "EVIOBlockedEventSource.h"

using namespace std;

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

EVIOBlockedEventSource::~EVIOBlockedEventSource() {
    LOG_INFO(m_logger) << "Closing hdevio event source \"" << m_filename << "\"" << LOG_END;
    m_filename = std::string();

    // Delete HDEVIO and print stats
    if (m_hdevio) {
        m_hdevio->PrintStats();
        delete m_hdevio;
    }

    // no if - C++98, guarantees that delete or delete[] on a nullpointer has no effect.
    delete[] m_buff;

}

void EVIOBlockedEventSource::Initialize() {
    LOG_INFO(m_logger) << "Initializing EVIOBlockedEventSource" << LOG_END;

    // Open file.
    // n.b. his is not fully integrated into the standard JANA
    // application at this point so we are not getting passed the
    // event source names like JEventSource classes are.
    m_hdevio = new HDEVIO(m_filename, true, 2);   // 2 for VERBOSE level
    if (!m_hdevio->is_open) {
        cerr << m_hdevio->err_mess.str() << endl;
        throw JException("Failed to open EVIO file: " + m_filename, __FILE__, __LINE__);
        // throw exception indicating error
    }
    LOG_INFO(m_logger) << "Success opening event source \"" << m_filename << "\"!" << LOG_END;
}

JBlockedEventSource<EVIOBlockedEvent>::Status EVIOBlockedEventSource::NextBlock(EVIOBlockedEvent &block) {
    LOG_INFO(m_logger) <<  "JBlockedEventSource::NextBlock" << LOG_END;

    EVIOBlockedEventSource::Status status;
    m_buff = new uint32_t[m_buff_len];

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

    bool read_ok = m_hdevio->readNoFileBuff(m_buff, m_buff_len);
    uint32_t cur_len = m_hdevio->last_event_len;

    if (read_ok) { // copy data to block
        LOG_INFO(m_logger) << "Block " << m_block_number << " read_ok" << LOG_END;
        block.block_number = m_block_number++;
        block.swap_needed = m_hdevio->swap_needed;
        block.data.insert(block.data.begin(), m_buff, m_buff + cur_len);

        status = EVIOBlockedEventSource::Status::Success;
        m_buff_len = DEFAULT_READ_BUFF_LEN;
    } else {
        if (m_hdevio->err_code == HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL) {
            m_buff_len = cur_len;
            status = Status::FailTryAgain;
            LOG_INFO(m_logger) << "Block \"" << m_block_number << " HDEVIO_USER_BUFFER_TOO_SMALL" << LOG_END;
        } else if (m_hdevio->err_code == HDEVIO::HDEVIO_EOF) {
            status = Status::FailFinished;
            LOG_INFO(m_logger) << "No more blocks in \"" << m_filename << "\"!" << LOG_END;

            // FIXME: We should NOT need t ocall this! JANA does not seem to accept the FailedFinished status as a condition to end.
            japp->Quit(true);
        } else { // keep it ugly now
            throw JException("Unhandled EVIO file read return status: " , __FILE__, __LINE__);
        }
    }

    delete[] m_buff;
//        m_buff = nullptr;
    return status;
}

