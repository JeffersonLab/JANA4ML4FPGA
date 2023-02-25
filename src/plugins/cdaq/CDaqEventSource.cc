

#include "CDaqEventSource.h"

#include <JANA/JEvent.h>
#include <tcp_daq/tcp_thread.h>
#include <services/log/Log_service.h>

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

}

void CDaqEventSource::GetEvent(std::shared_ptr <JEvent> event) {

    /// Calls to GetEvent are synchronized with each other, which means they can
    /// read and write state on the JEventSource without causing race conditions.
    
    /// Configure event and run numbers
    static size_t current_event_number = 1;
    event->SetEventNumber(current_event_number++);
    event->SetRunNumber(22);

    // Read next
//    if (TCP_FLAG == 0) {
//        printf(" EXIT PORT=%d \n", socket_fd);
//        return 1;
//    }

    m_log->trace("Wait new HEADER [socket_fd={}]", m_socket_fd);

    // READ 10 bytes block
    int header_data[10];
    int rc = TcpReadData(m_socket_fd, header_data, sizeof(header_data));
    if (rc < 0) {
        m_log->error("TcpReadData error. Return Code = {}. It is -1, right? Sigh... why this error message...", rc);
        TCP_FLAG = 0;
        throw RETURN_STATUS::kERROR;
    }

    unsigned long int read_data_len = 0;
    read_data_len += sizeof(header_data);

    // We-HaVe-LIkE-tHis-NaMiNGconvEnTIOn-HeEeEerE
    int REQUEST = header_data[0];
    int MARKER = header_data[1];
    int LENEVENT = header_data[2];
    int xxx = header_data[3];     // ???
    int TriggerID = header_data[4];
    int Nr_Modules = header_data[5];
    int ModuleID = header_data[6];
    int RunNum = header_data[7];
    int status = header_data[8];
    //printf("run_child recv::hdr ==> REQ=0x%X MARKER=0x%X LEN=%d TRG=0x%X Nmod=%d modID=%d \n",REQUEST,MARKER,LENEVENT,TriggerID,Nr_Modules,ModuleID);
    if (MARKER == 0xABCDEF00) {
        printf("!!!!! event-builder:: Test Connection MARKER=%X,  return 0xABCDEF11 \n", MARKER);
        //-- send HEADER --
        header_data[1] = 0xABCDEF11;
        header_data[2] = 0;
        header_data[3] = 0;
        header_data[4] = 0;
        header_data[5] = 0;
        header_data[6] = 0;
        if (tcp_send_th(m_socket_fd, header_data, sizeof(header_data))) {
            perror("send");
            TCP_FLAG = 0;
        }
        throw RETURN_STATUS::kTRY_AGAIN;
    }

    if (TriggerID < 100) {
        printf(" >> RECV:0: REQUEST=0x%x, TriggerID=0x%x  evtSize=%d   (%d) ModID=%d  \n",
               REQUEST, header_data[4], header_data[2], RunNum, header_data[6]);
    }

    //=================================================================================================================================
    //                                                               ---  recv BUFFERED ---
    //=================================================================================================================================
    if (REQUEST == 0x5) {  //---  recv BUFFERED ---;
        m_log->trace("get_from_client:: MARKER={}", MARKER);

        if (LENEVENT > MAXDATA) {
            printf(" %c  \033[1m\033[31m  ERROR RECV:: event size > buffsize %d %lu trig=%d mod=%d \n", 7, LENEVENT,
                   MAXDATA, TriggerID, ModuleID);

            close(m_socket_fd);
            TCP_FLAG = 0;
            throw RETURN_STATUS::kERROR;;
        }

        //---------------- get only header ; 3 words  !!!!!  --------
        // rc=tcp_get_th(sd_current,BUFFER,LENEVENT*4);
        int PACKET[MAXDATA_DC + 10];
        int *BUFFER = &PACKET[10];
        rc = tcp_get(m_socket_fd, BUFFER, 3 * 4);
        if (rc < 0) {
            printf(" 2 get errorr rc<0 (%d)......\n", rc);
            TCP_FLAG = 0;
            throw RETURN_STATUS::kERROR;;
        }
        else if (rc > 0) {
            printf("Need to get more data=%d \n", rc);
            TCP_FLAG = 0;
            throw RETURN_STATUS::kERROR;;
        };

        read_data_len += LENEVENT * 4;

        unsigned int evtTrigID = BUFFER[1];
        unsigned int evtModID = (BUFFER[0] >> 24) & 0xff;
        unsigned int evtSize = BUFFER[2];

        if (evtTrigID == EORE_TRIGGERID ||
            evtTrigID == BORE_TRIGGERID) {    //-------------       for memory book          -----------

            printf(" >> RECV:: TriggerID=0x%x (0x%x) evtSize=%d  got Run Number = %d ModID=%d \n", TriggerID,
                   evtTrigID, evtSize, RunNum, evtModID);
        }


        if (evtTrigID == BORE_TRIGGERID) {    //-------------         New run           -----------
            printf(" >> RECV:: BOR:: TriggerID=0x%x (0x%x)  \n", TriggerID, evtTrigID, RunNum);


        }   //--------  if evtTrigID==BORE_TRIGGERID ------


        if (is_verbose || DEBUG_RECV > 0 || TriggerID < 0) {
            printf("+++>>> recv::hdr:: TrID=%d(0x%x) ModNr=%d len=%d; evt:: TrgID=%d, ModID=%d, len=%d (%d bytes)\n",
                   TriggerID, TriggerID, ModuleID, LENEVENT, evtTrigID, evtModID, evtSize, evtSize * 4);
            struct EvtHeader *hdr = (EvtHeader *) BUFFER;
            //printf("      header:: EvSize=%d EvType=%d ModID=%d DevType=%d Trigger=%d \n"
            //	   , hdr->EventSize, hdr->EventType, hdr->ModuleNo, hdr->DeviceType, hdr->Triggernumber);
        }

        if (evtSize != LENEVENT)
            printf("*****> %c ERROR RECV:: TrigID=%d, event size::  %d != %d (LENEVENT)\n", 7, evtTrigID, evtSize,
                   LENEVENT);
        /*
      else
      printf("*****> %c OK    RECV:: TrigID=%d, event size::  %d != %d (LENEVENT)\n",7,evtTrigID,evtSize,LENEVENT);
        */
        if (LENEVENT > MAXDATA) {
            printf("*****> %c ERROR RECV:: LENEVENT=%d  MAXDATA=%lu  bytes\n", 7, LENEVENT, MAXDATA);
        }

        gettimeofday(&tm[1], NULL);


        //     wait_buff:;
        //---- Copy recv Buffer to SHMEM -----
        if (DEBUG_RECV > 8) printf(" RECV: wait shmem\n");

        icc++;

        // fifo->AddEvent( BUFFER, LENEVENT);

        //unsigned int * wrptr = fifo->AddEvent3(LENEVENT);
        //---------------- get only header ; 3 words  !!!!!  --------
        rc = tcp_get(socket_fd, BUFFER,
                     (LENEVENT - 3) * 4);  // <<<<---------------- THIS IS DATA !!!! -----------------------
        //memcpy(wrptr,BUFFER,3*4);  //--- words to bytes
	

}

std::string CDaqEventSource::GetDescription() {

    /// GetDescription() helps JANA explain to the user what is going on
    return "";
}


template <>
double JEventSourceGeneratorT<CDaqEventSource>::CheckOpenable(std::string resource_name) {

    /// CheckOpenable() decides how confident we are that this EventSource can handle this resource.
    ///    0.0        -> 'Cannot handle'
    ///    (0.0, 1.0] -> 'Can handle, with this confidence level'
    
    /// To determine confidence level, feel free to open up the file and check for magic bytes or metadata.
    /// Returning a confidence <- {0.0, 1.0} is perfectly OK!
    
    return (resource_name == "JEventSourceCDAQtcp") ? 1.0 : 0.0;
}

void CDaqEventSource::ThrowOnErrno(const std::string &comment) {
    auto error_message = fmt::format("{}: {}",comment, strerror(errno));
    m_log->error(error_message);
    throw std::runtime_error(error_message);
}
