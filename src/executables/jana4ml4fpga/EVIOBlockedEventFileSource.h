
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

class EVIOBlockedEventFileSource : public JBlockedEventSource<EVIOBlockedEvent> {

public:
    /**
    * Constructor
    * @param options_filenames: filenames to open
    */
    explicit EVIOBlockedEventFileSource(const std::vector<std::string> filenames) {
        m_filenames = filenames;
    }

    ~EVIOBlockedEventFileSource();

    void Initialize() override;

    /**
     * Read a chunk of evio file, store it in @param block, and @return reading status.
     * One call to HDEVIO::readNoFileBuff() gets one block.
     * Need to consider the case where user buffer is not big enough that a second read is necessary.
     */
    Status NextBlock(EVIOBlockedEvent &block) override;

    /**
     * Process an EVIOBlockedEvent block and extract events from the block.
     */
    std::vector<std::shared_ptr<JEvent>> DisentangleBlock(EVIOBlockedEvent &block, JEventPool &pool) override;


private:
    int m_block_number = 1;

    JLogger m_logger;

    std::vector<std::string> m_filenames;   // filenames to open, copied from cli inputs
    std::string cur_file;                   // current EVIO file to read

    std::unique_ptr<HDEVIO> m_hdevio;

    uint32_t *m_buff = nullptr;             // buff to read EVIO block
    uint32_t m_buff_len = DEFAULT_READ_BUFF_LEN;

    /**
     * Open an .evio file indicated by the top element of @var m_filenames。
     * @note: make sure @var m_hdevio is nullptr before calling this func.
     */
    void OpenNextEVIOFile();

    /**
     * Close an evio file indicated by @var cur_file.
     */
    void CloseEVIOFile();
};

