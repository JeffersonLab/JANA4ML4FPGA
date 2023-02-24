
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <string>
#include <cinttypes>

#include <JANA/JBlockedEventSource.h>
#include <JANA/JLogger.h>
#include <evio/HDEVIO.h>

#include "EVIOBlockedEvent.h"

#define UNSET_EVIO_FILE "#"
#define TEST_EVIO_FILE "/gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_002.evio"

#define DEFAULT_READ_BUFF_LEN 4000000  // number taken from the CDAQtcp example

class EVIOBlockedEventSource : public JBlockedEventSource<EVIOBlockedEvent> {

    int m_block_number = 0; // what's this for
    JLogger m_logger;

    /**
     * Open an evio file.
     */
    virtual void Initialize();

    /**
     * Use @var m_buff to ead a chunk of evio file, copy it to @param block, and @return reading status.
     * One call to HDEVIO::readNoFileBuff() gets one block.
     * Need to consider the case where the buff_len is not big enough that a second read is necessary.
     */
    virtual Status NextBlock(EVIOBlockedEvent &block);

    /**
     * Process an EVIOBlockedEvent block and extract JObjetcs from the block.
     * TODO
     */
    virtual std::vector <std::shared_ptr<JEvent>> DisentangleBlock(MyBlock &block, JEventPool &pool);

    ~EVIOBlockedEventSource();

protected:
    std::string m_filename = UNSET_EVIO_FILE;
    HDEVIO* m_hdevio = nullptr;

    uint32_t* m_buff = nullptr;
    unit32_t m_buff_len = DEFAULT_READ_BUFF_LEN;

    void OpenEVIOFile();

    void SetEVIOFileName(std::string filename);

    Status ReadOneBlock(EVIOBlockedEvent &block);
};
