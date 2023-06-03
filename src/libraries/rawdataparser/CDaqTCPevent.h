

#pragma once

#include <JANA/JObject.h>

// This class is used to hold information from a TCP event read via the 
// tcp_daq library's tcp_event class. Its purpose is to encapsulate all of
// data from an event read from TCP and make it available via the standard
// JANA factory mechanism.
//
// This is also used to write data to a TCP socket so the program can be used
// as a source of network events.

struct CDaqTCPevent : public JObject {
    char HOST2[256];
    std::vector<uint32_t> DATA;
    int lenDATA;
    int Nr_Modules;
    int ModuleID;
    unsigned int TriggerID;


    /// Make it convenient to construct one of these things
    CDaqTCPevent(int lenDATA) : lenDATA(lenDATA) { DATA.resize(lenDATA); };


    /// Override className to tell JANA to store the exact name of this class where we can
    /// access it at runtime. JANA provides a NAME_OF_THIS macro so that this will return the correct value
    /// even if you rename the class using automatic refactoring tools.

    const std::string className() const override {
        return NAME_OF_THIS;
    }

    /// Override Summarize to tell JANA how to produce a convenient string representation for our JObject.
    /// This can be used called from user code, but also lets JANA automatically inspect its own data. For instance,
    /// adding JCsvWriter<Hit> will automatically generate a CSV file containing each hit. Warning: This is obviously
    /// slow, so use this for debugging and monitoring but not inside the performance critical code paths.

    void Summarize(JObjectSummary& summary) const override {
        summary.add(HOST2, NAME_OF(HOST2), "%s", "Host that sent us the event");
        summary.add(DATA.size(), NAME_OF(DATA), "%d", "Size of event");
    }
};



