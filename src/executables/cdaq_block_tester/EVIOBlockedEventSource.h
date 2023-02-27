
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <string>
#include <cinttypes>

#include <JANA/JBlockedEventSource.h>
#include <JANA/JLogger.h>
#include <evio/HDEVIO.h>

#include <rawdataparser/EVIOBlockedEvent.h>

#define UNSET_EVIO_FILE "#"
#define TEST_EVIO_FILE "/gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_002.evio"

#define DEFAULT_READ_BUFF_LEN 4000000  // number taken from the CDAQtcp example

class EVIOBlockedEventSource : public JBlockedEventSource<EVIOBlockedEvent> {

    int m_block_number = 1;
    JLogger m_logger;

    std::string m_filename = UNSET_EVIO_FILE;
    HDEVIO *m_hdevio = nullptr;

    uint32_t *m_buff = nullptr;
    uint32_t m_buff_len = DEFAULT_READ_BUFF_LEN;

    ~EVIOBlockedEventSource(){
        LOG_INFO(m_logger) << "Closing hdevio event source \"" << m_filename << "\"" << LOG_END;
        m_filename = UNSET_EVIO_FILE;

        // Delete HDEVIO and print stats
        if (m_hdevio) {
            m_hdevio->PrintStats();
            delete m_hdevio;
        }
        if (m_buff) {
            delete[] m_buff;
        }
    }

    /**
     * Open an evio file.
     */
    void Initialize() {

        LOG_INFO(m_logger) << "Initializing EVIOBlockedEventSource" << LOG_END;
        m_filename = TEST_EVIO_FILE;
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

    /**
     * Read a chunk of evio file, store it in @param block, and @return reading status.
     * One call to HDEVIO::readNoFileBuff() gets one block.
     * Need to consider the case where the bufflen is not big enough that a second read is necessary.
     */
    Status NextBlock(EVIOBlockedEvent &block) {
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
        m_buff = nullptr;
        return status;
    }

    /**
     * Process an EVIOBlockedEvent block and extract events from the block.
     * Complicated part.
     * TODO
     */
    std::vector <std::shared_ptr<JEvent>> DisentangleBlock(EVIOBlockedEvent &block, JEventPool &pool);

};
