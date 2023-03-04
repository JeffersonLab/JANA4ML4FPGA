

#include "JEventSourceCDAQtcp.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <tcp_daq/tcp_thread.h>

static int g_CDAQ_PORT = 10000;

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JEventSourceGeneratorT<JEventSourceCDAQtcp>);

        // User should be able to add this plugin with nothing else and
        // have it work as an event source. Add an event source name to
        // ensure this.
        app->SetDefaultParameter("CDAQtcp:port", g_CDAQ_PORT, "TCP port on remote host to connect to");
        std::stringstream ss;
        ss << "CDAQtcp:" << g_CDAQ_PORT;
        app->Add(ss.str());
    }
}

JEventSourceCDAQtcp::JEventSourceCDAQtcp(std::string resource_name, JApplication* app) : JEventSource(resource_name, app) {
    SetTypeName(NAME_OF_THIS); // Provide JANA with class name
}

void JEventSourceCDAQtcp::Open() {

    // Use tcp_daq library to open socket and listen on port for TCP connection
	// m_sd = tcp_open_th(m_port, (char*)m_remote_host.c_str());
    LOG << "CDQtcp: Opening port " << g_CDAQ_PORT << LOG_END;
    m_sd = tcp_open_local_th(g_CDAQ_PORT);
    char host_name[256];
    int host_name_max_len = 255;
    auto err = tcp_listen3(m_sd, host_name, host_name_max_len, &m_sd_current);
    if( err<0 ) throw JException("Error listening on port");
}

void JEventSourceCDAQtcp::GetEvent(std::shared_ptr <JEvent> event) {

    /// Calls to GetEvent are synchronized with each other, which means they can
    /// read and write state on the JEventSource without causing race conditions.
    
    /// Configure event and run numbers
    static size_t current_event_number = 1;
    event->SetEventNumber(current_event_number++);
    event->SetRunNumber(22);
	
	int bufflen = 100000;
	int buff[bufflen];
	
	auto Nread = tcp_get(m_sd_current, buff, bufflen);

    /// Insert whatever data was read into the event
    // std::vector<Hit*> hits;
    // hits.push_back(new Hit(0,0,1.0,0));
    // event->Insert(hits);

    /// If you are reading a file of events and have reached the end, terminate the stream like this:
    // // Close file pointer!
    // throw RETURN_STATUS::kNO_MORE_EVENTS;

    /// If you are streaming events and there are no new events in the message queue,
    /// tell JANA that GetEvent() was temporarily unsuccessful like this:
    // throw RETURN_STATUS::kBUSY;
}

std::string JEventSourceCDAQtcp::GetDescription() {

    /// GetDescription() helps JANA explain to the user what is going on
    return "JANA event source that reads from tcp socket that CDAQ sends to.";
}


template <>
double JEventSourceGeneratorT<JEventSourceCDAQtcp>::CheckOpenable(std::string resource_name) {

    /// CheckOpenable() decides how confident we are that this EventSource can handle this resource.
    ///    0.0        -> 'Cannot handle'
    ///    (0.0, 1.0] -> 'Can handle, with this confidence level'
    
    /// To determine confidence level, feel free to open up the file and check for magic bytes or metadata.
    /// Returning a confidence <- {0.0, 1.0} is perfectly OK!
    
    return (resource_name == "JEventSourceCDAQtcp") ? 1.0 : 0.0;
}
