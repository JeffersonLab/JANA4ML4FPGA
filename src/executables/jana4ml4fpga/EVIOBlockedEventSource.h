
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <string>
#include <cinttypes>

#include <JANA/JBlockedEventSource.h>
#include <JANA/JLogger.h>
#include <evio/HDEVIO.h>

#include <rawdataparser/EVIOBlockedEvent.h>

#define DEFAULT_READ_BUFF_LEN 4000000  // number taken from the CDAQtcp example

class EVIOBlockedEventSource : public JBlockedEventSource<EVIOBlockedEvent> {

public:
    /**
    * Constructor
    * @param resource_name - a file name to open
    */
    explicit EVIOBlockedEventSource(const std::string& resource_name) {
        m_filename = resource_name;
    }


    ~EVIOBlockedEventSource();

    /**
     * Open an evio file.
     */
    void Initialize() override;

    /**
     * Read a chunk of evio file, store it in @param block, and @return reading status.
     * One call to HDEVIO::readNoFileBuff() gets one block.
     * Need to consider the case where the bufflen is not big enough that a second read is necessary.
     */
    Status NextBlock(EVIOBlockedEvent &block) override;

    /**
     * Process an EVIOBlockedEvent block and extract events from the block.
     */
    std::vector <std::shared_ptr<JEvent>> DisentangleBlock(EVIOBlockedEvent &block, JEventPool &pool) override;


private:
    int m_block_number = 1;
    JLogger m_logger;
    std::string m_filename;         // File name to open. Initializes with a empty string
    HDEVIO *m_hdevio = nullptr;
    uint32_t *m_buff = nullptr;
    uint32_t m_buff_len = DEFAULT_READ_BUFF_LEN;
};

