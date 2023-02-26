#include "CDaqEventSource.h"

#include <future>
#include <thread>
#include <chrono>
#include <JANA/JEvent.h>
#include <tcp_daq/tcp_thread.h>
#include <services/log/Log_service.h>
#include "CDaqRawData.h"

#define MAXCLNT 32  //--  used 32 bit word   !!!!
#define MAXMOD  15L      //-- max 15 modules/crates
#define MAXDATA_DC  300000L  //-- in words;
#define MAXDATA 65536L   //-- in words;  400kB max (RAW CDC) => 100000L

#define   BORE_TRIGGERID  0x55555555
#define   INFO_TRIGGERID  0x5555AAAA
#define   EPICS_TRIGGERID 0xAAAA5555
#define   EORE_TRIGGERID  0xAAAAAAAA

CDaqEventSource::CDaqEventSource(std::string resource_name, JApplication* app) : JEventSource(resource_name, app) {
    SetTypeName(NAME_OF_THIS); // Provide JANA with class name
}

void CDaqEventSource::Open() {

	m_port = 20249;
	m_remote_host = "localhost";
	GetApplication()->SetDefaultParameter("cdaq:port", m_port, "TCP port on remote host to connect to");
	GetApplication()->SetDefaultParameter("cdaq:host", m_remote_host, "TCP port on remote host to connect to");


    m_log = GetApplication()->GetService<Log_service>()->logger(GetPluginName());
    m_log->info("GetResourceName() = {}", GetResourceName());

    //================================================================

    m_log->info("Binding socket... {}", getpid());

    /*--- LISTEN is implied -----*/
    m_log->info("CDaqEventSource::Open() - open socket, bind port = {}", m_port);

    // Create an endpoint for communication
    // Get socket file descriptor
    m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket_fd == -1) {
        ThrowOnErrno("ERROR creating endpoint at 'socket(...)'");
    }
    m_log->debug("  Obtained socket file descriptor [socket_fd={}]", m_socket_fd);


    // Set SO_REUSEADDR the socket options
    m_log->debug("  Setting SO_REUSEADDR socket option for [socket_fd={}]", m_socket_fd);
    int opt_value = 1;
    if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value)) < 0) {
        close(m_socket_fd);
        ThrowOnErrno("ERROR Setting SO_REUSEADDR option");
    }

    // Trying to bind socket.
    struct sockaddr_in sock_addr{};
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(m_port);

    m_log->debug("  Binding socket [socket_fd={}]", m_socket_fd);
    /* bind the socket to the port number */
    if (bind(m_socket_fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == -1) {
        close(m_socket_fd);
        ThrowOnErrno("ERROR binding socket at bind(...)");
    }

    // HERE means SUCCESS!
    m_log->info("Socket binding success. Listening port {} [socket_fd={}]", m_port, m_socket_fd);
    m_log->info("m_receive_fd.is_lock_free() {}", m_receive_fd.is_lock_free());

    // Start a thread that will wait for a client connection
    m_receive_fd = -1;
    m_listen_thread = std::thread([this](){this->WaitForClient();});
    m_listen_thread.detach();
}


void CDaqEventSource::WaitForClient() {

    char host_name[128];
    int len = 128;

    m_log->info("Waiting for clients");
    m_log->debug("   socket_fd={}", m_socket_fd);
    m_log->debug("   port={}", m_port);
    m_log->debug("   Start listening");

    // Start listening
    auto result = listen(m_socket_fd, 5);
    if (result == -1) {
        ThrowOnErrno("ERROR at socket listen");
    }

    /* wait for a client to talk to us */
    struct sockaddr_in pin{};
    socklen_t addr_len = sizeof(pin);
    int receive_fd = accept(m_socket_fd, (struct sockaddr *) &pin, &addr_len);
    if (receive_fd == -1) {
        ThrowOnErrno("ERROR at socket accept");
    }
    m_log->info("Client connected. Checking...");
    int remote_port = ntohs(pin.sin_port);
    m_log->info("  Remote port: {}", remote_port);
    m_log->info("  Try to get host by address: {}", inet_ntoa(pin.sin_addr));

    hostent *hp = gethostbyaddr(&pin.sin_addr, sizeof(pin.sin_addr), AF_INET);
    if (hp == nullptr) {
        m_log->warn("gethostbyaddr failed to get host address");
        strncpy(host_name, inet_ntoa(pin.sin_addr), len);
        host_name[len - 1] = 0;
    } else {
        m_log->info("Client host name: {}", hp->h_name);
        strncpy(host_name, hp->h_name, len);
        host_name[len - 1] = 0;
    }

    m_log->debug("Connection info:");
    m_log->debug("  Host: {}({})", host_name, inet_ntoa(pin.sin_addr));
    m_log->debug("  Port: {}", remote_port);
    m_log->debug("  Accepted sock_sd: {}", receive_fd);
    m_log->debug("  m_socket_sd     : {}", m_socket_fd);

    m_receive_fd = receive_fd;
}


void CDaqEventSource::GetEvent(std::shared_ptr <JEvent> event) {

    /// Calls to GetEvent are synchronized with each other, which means they can
    /// read and write state on the JEventSource without causing race conditions.

    // m_receive_fd - is a file descriptor that could be used in recv function
    // it is set by WaitForClient when the client connects
    // JANA at the same time will be trying to get events
    // So this is the place where we wait for a client on this thread
    if (m_receive_fd == -1) {
        // Not yet connected. Wait more
        throw RETURN_STATUS::kTRY_AGAIN;
    } else {
        m_log->info("Getting date from the client");
    }
    int receive_fd = m_receive_fd;

    // If we are here, a client has connected

    // READ 10 bytes block
    int header_data[10];
    int rc = TcpReadData(receive_fd, header_data, sizeof(header_data));
    if (rc < 0) {
        m_log->error("TcpReadData error. Return Code = {}. It is -1, right? Sigh... why this error message...", rc);
        TCP_FLAG = 0;
        throw RETURN_STATUS::kERROR;
    }

    unsigned long int read_data_len = 0;
    read_data_len += sizeof(header_data);

    // We-HaVe-LIkE-tHis-NaMiNGconvEnTIOn-HeEeEerE
    int header_request = header_data[0];
    int header_marker = header_data[1];
    int header_event_len = header_data[2];
    int header_useless = header_data[3];     // ???
    int header_trigger_id = header_data[4];
    int header_num_modules = header_data[5];
    int header_module_id = header_data[6];
    int header_run_number = header_data[7];
    int header_status = header_data[8];

    /// Configure event and run numbers
    static size_t current_event_number = 1;
    event->SetEventNumber(current_event_number++);
    event->SetRunNumber(header_run_number);

    //printf("run_child recv::hdr ==> REQ=0x%X MARKER=0x%X LEN=%d TRG=0x%X Nmod=%d modID=%d \n",REQUEST,MARKER,LENEVENT,TriggerID,Nr_Modules,ModuleID);
    if (header_marker == 0xABCDEF00) {
        m_log->debug("Test Connection MARKER=0xABCDEF00  return 0xABCDEF11");
        //-- send HEADER --
        header_data[1] = 0xABCDEF11;
        header_data[2] = 0;
        header_data[3] = 0;
        header_data[4] = 0;
        header_data[5] = 0;
        header_data[6] = 0;
        if (tcp_send_th(receive_fd, header_data, sizeof(header_data))) {
            perror("send");
            TCP_FLAG = 0;
        }
        throw RETURN_STATUS::kTRY_AGAIN;
    }

    if (header_trigger_id < 100) {
        m_log->debug("header_trigger_id < 100: request=0x{:02X} trigger_id=0x{:02X} evt_size={} run={} mod_id={}",
                     header_request, header_data[4], header_data[2], header_run_number, header_data[6]);
    }

    //=================================================================================================================================
    //                                                               ---  recv BUFFERED ---
    //=================================================================================================================================
    if (header_request == 0x5) {  //---  recv BUFFERED ---;
        m_log->trace("get_from_client:: MARKER={}", header_marker);

        // Check event len is OK
        if (header_event_len > MAXDATA) {
            m_log->error("ERROR RECV:: event size={} > buffsize={} trig={} mod={}",
                         header_event_len, MAXDATA, header_trigger_id, header_module_id);

            close(m_receive_fd);
            throw RETURN_STATUS::kERROR;;
        }

        // First we read only 3 words from header
        int PACKET[MAXDATA_DC + 10];
        int *BUFFER = &PACKET[10];
        rc = TcpReadData(receive_fd, BUFFER, 3 * 4);
        if (rc < 0) {
            ThrowOnErrno("TCP receive error");
        } else if (rc > 0) {
            ThrowOnErrno("Need to get more data");
        }

        read_data_len += header_event_len * 4;

        unsigned int event_trigger_id = BUFFER[1];
        unsigned int event_mod_id = (BUFFER[0] >> 24) & 0xff;
        unsigned int event_size = BUFFER[2];

        if (event_trigger_id == EORE_TRIGGERID ||
            event_trigger_id == BORE_TRIGGERID) {    //-------------       for memory book          -----------
            // Who knows what is it? TODO check with Sergey
            printf(" >> RECV:: TriggerID=0x%x (0x%x) evtSize=%d  got Run Number = %d ModID=%d \n", header_trigger_id,
                   event_trigger_id, event_size, header_run_number, event_mod_id);
        }

        if (event_trigger_id == BORE_TRIGGERID) {    //-------------         New run           -----------
            // New run number?
            m_log->debug("evtTrigID == BORE_TRIGGERID {} {} {} New run number?", header_trigger_id, event_trigger_id, header_run_number);
        }


//        if (is_verbose || DEBUG_RECV > 0 || TriggerID < 0) {
//            printf("+++>>> recv::hdr:: TrID=%d(0x%x) ModNr=%d len=%d; evt:: TrgID=%d, ModID=%d, len=%d (%d bytes)\n",
//                   TriggerID, TriggerID, ModuleID, LENEVENT, evtTrigID, evtModID, evtSize, evtSize * 4);
//            struct EvtHeader *hdr = (EvtHeader *) BUFFER;
//            //printf("      header:: EvSize=%d EvType=%d ModID=%d DevType=%d Trigger=%d \n"
//            //	   , hdr->EventSize, hdr->EventType, hdr->ModuleNo, hdr->DeviceType, hdr->Triggernumber);
//        }

        if (event_size != header_event_len) {
            m_log->warn("event_size ({}) != header_event_len ({}), TrigID={}", event_size, header_event_len, event_trigger_id);
            // TODO event_size but header_event_len need renaming
        }

        if (header_event_len > MAXDATA) {
            printf("*****> %c ERROR RECV:: LENEVENT=%d  MAXDATA=%lu  bytes\n", 7, header_event_len, MAXDATA);
            // TODO make it with m_log. Should we throw kERROR here?
        }


        // Get the rest of DATA
        rc = TcpReadData(m_receive_fd, BUFFER, (header_event_len - 3) * 4);  // <<<<---------------- THIS IS DATA !!!! -----------------------
        //memcpy(wrptr,BUFFER,3*4);  //--- words to bytes
    }
}

std::string CDaqEventSource::GetDescription() {

    /// GetDescription() helps JANA explain to the user what is going on
    return "Reads EVIO events from CDaq from TCP";
}


template <>
double JEventSourceGeneratorT<CDaqEventSource>::CheckOpenable(std::string resource_name) {

    /// CheckOpenable() decides how confident we are that this EventSource can handle this resource.
    ///    0.0        -> 'Cannot handle'
    ///    (0.0, 1.0] -> 'Can handle, with this confidence level'
    
    /// To determine confidence level, feel free to open up the file and check for magic bytes or metadata.
    /// Returning a confidence <- {0.0, 1.0} is perfectly OK!

    // For convenience, we convert resource_name to lower case
    std::transform(resource_name.begin(), resource_name.end(), resource_name.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    // And also check all conceivable variants
    return ((resource_name == "tcpcdaqevio") || (resource_name == "tcp-cdaq-evio") || (resource_name == "tcp-cdaq-evio")) ? 1.0 : 0.0;
}

void CDaqEventSource::ThrowOnErrno(const std::string &comment) {
    auto error_message = fmt::format("{}: {}",comment, strerror(errno));
    m_log->error(error_message);
    throw JException(error_message);
}
