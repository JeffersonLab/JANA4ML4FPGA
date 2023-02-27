
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#ifndef _MyObject_h_
#define _MyObject_h_

#include <cctype>
#include <cinttypes>

#include <JANA/JObject.h>

struct MyObject : public JObject {
    uint32_t datum;

    MyObject(uint32_t datum) : datum(datum) {}

    const std::string className() const override {
        return NAME_OF_THIS;
    }

    void Summarize(JObjectSummary& summary) const override {
        summary.add(datum, NAME_OF(datum), "%d", "Datum");
    }
};


#endif // _MyObject_h_
