

#include "JEventSourceCDAQtcp.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include "tcp_thread.h"

/// Include headers to any JObjects you wish to associate with each event
// #include "Hit.h"

/// There are two different ways of instantiating JEventSources
/// 1. Creating them manually and registering them with the JApplication
/// 2. Creating a corresponding JEventSourceGenerator and registering that instead
///    If you have a list of files as command line args, JANA will use the JEventSourceGenerator
///    to find the most appropriate JEventSource corresponding to that filename, instantiate and register it.
///    For this to work, the JEventSource constructor has to have the following constructor arguments:

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JEventSourceGeneratorT<JEventSourceCDAQtcp>);
    }
}


JEventSourceCDAQtcp::JEventSourceCDAQtcp(std::string resource_name, JApplication* app) : JEventSource(resource_name, app) {
    SetTypeName(NAME_OF_THIS); // Provide JANA with class name
}

void JEventSourceCDAQtcp::Open() {

	m_port = 10000;
	m_remote_host = GetResourceName();
	GetApplication()->SetDefaultParameter("CDAQtcp:port", m_port, "TCP port on remote host to connect to");
	GetApplication()->SetDefaultParameter("CDAQtcp:remote_host", m_remote_host, "TCP port on remote host to connect to");

	m_sd = tcp_open_th(m_port, (char*)m_remote_host.c_str());
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
	
	auto Nread = tcp_get_th(m_sd, buff, bufflen);

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
    return "";
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
