
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
