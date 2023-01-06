

#ifndef _JEventSourceCDAQtcp_h_
#define  _JEventSourceCDAQtcp_h_

#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>

class JEventSourceCDAQtcp : public JEventSource {

    /// Add member variables here

public:
    JEventSourceCDAQtcp(std::string resource_name, JApplication* app);

    virtual ~JEventSourceCDAQtcp() = default;

    void Open() override;

    void GetEvent(std::shared_ptr<JEvent>) override;
    
    static std::string GetDescription();
    
    int m_port;
    std::string m_remote_host;
    int m_sd;

};

template <>
double JEventSourceGeneratorT<JEventSourceCDAQtcp>::CheckOpenable(std::string);

#endif // _JEventSourceCDAQtcp_h_

