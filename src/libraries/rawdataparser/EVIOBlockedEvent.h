
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once 

#include <cctype>
#include <cinttypes>


struct EVIOBlockedEvent {

    std::vector<uint32_t> data;

    bool swap_needed = false;  // do we need this?
};

