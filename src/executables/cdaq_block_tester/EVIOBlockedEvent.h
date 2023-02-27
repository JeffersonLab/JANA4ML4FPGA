
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once 

#include <cctype>
#include <cinttypes>

struct EVIOBlockedEvent {
    int block_number = 1;
    std::vector<uint32_t> data;
    bool swap_needed = false;
};
