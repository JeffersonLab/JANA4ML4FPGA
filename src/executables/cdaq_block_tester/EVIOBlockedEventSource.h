
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <string>

#include <JANA/JBlockedEventSource.h>
#include <JANA/JLogger.h>
#include <evio/HDEVIO.h>

#include "EVIOBlockedEvent.h"

#define UNSET_EVIO_FILE "#"
#define TEST_EVIO_FILE "/gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_002.evio"

class EVIOBlockedEventSource : public JBlockedEventSource<EVIOBlockedEvent> {

    int m_block_number = 1;
    JLogger m_logger;

    /**
     * Open an evio file.
     */
    virtual void Initialize();

    /**
     * Read a chunk of evio file, store it in @param block, and @return reading status.
     * One call to HDEVIO::readNoFileBuff() gets one block.
     * Need to consider the case where the bufflen is not big enough that a second read is necessary.
     */
    virtual Status NextBlock(EVIOBlockedEvent &block);

    /**
     * Process an EVIOBlockedEvent block and extract events from the block.
     * Complicated part.
     * TODO
     */
    virtual std::vector <std::shared_ptr<JEvent>> DisentangleBlock(MyBlock &block, JEventPool &pool);


protected:
    std::string m_filename = UNSET_EVIO_FILE;
    HDEVIO *evio = nullptr;

};
