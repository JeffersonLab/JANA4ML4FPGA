//
// Created by xme@jlab.org on 2/9/23.
//

#include <rawdataparser/EVIOBlockedEventParser.h>

#include "CDAQEVIOFileSource.h"

#define DEFAULT_READ_BUFF_LEN 4000000

using namespace std;
using namespace std::chrono;

/**
 * Constructor
 */
CDAQEVIOFileSource::CDAQEVIOFileSource(std::string resource_name, JApplication *app) : JEventSource(
        resource_name, app) {
    SetTypeName(NAME_OF_THIS); // Provide JANA with class name
}
/**
 * Destructor
 */
CDAQEVIOFileSource::~CDAQEVIOFileSource() {
    m_log->info("Closing EVIO file {}", m_hdevio->filename);

    // Delete HDEVIO and print stats
    if (m_hdevio) {
        m_hdevio->PrintStats();
        delete m_hdevio;
    }

    m_evio_filename = "";
}

void CDAQEVIOFileSource::OpenEVIOFile(std::string filename) {
    m_hdevio = new HDEVIO(filename, true, 2);  // 2 for VERBOSE level
    if (!m_hdevio->is_open) {
        cerr <<m_hdevio->err_mess.str() << endl;
        throw JException("Failed to open EVIO file: " + filename, __FILE__, __LINE__);
        // throw exception indicating error
    }
    m_log->info("Open EVIO file {} successfully", filename);
}

EVIOBlockedEvent CDAQEVIOFileSource::GetBlockFromBuffer(uint32_t *buffer, uint32_t event_len) {
    EVIOBlockedEvent block;

    block.swap_needed = m_hdevio->swap_needed;
    block.data.insert(block.data.begin(), buffer, buffer + event_len);

    return block;
}

/**
 * Read the evio file with HDEVIO::readNoFileBuff().
 * Handle three cases: read_ok, HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL, HDEVIO::HDEVIO_EOF.
 */
void CDAQEVIOFileSource::GetEvent(std::shared_ptr<JEvent> event) {
    uint32_t block_counter = 1;
    uint32_t buff_len = DEFAULT_READ_BUFF_LEN;
    uint32_t* buff = new uint32_t[buff_len];

    while (m_hdevio->readNoFileBuff(buff, buff_len) || m_hdevio->err_code == HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL) {
        uint32_t cur_len = m_hdevio->last_event_len;
        m_log->info("Read block {} status: {}", block_counter, m_hdevio->err_code);

        //Buffer is too small, enlarge buffer
        if (m_hdevio->err_code == HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL) {
            delete[] buff;
            buff_len = cur_len;
            buff = new uint32_t[buff_len];
            continue;
        }

        EVIOBlockedEvent cur_block = GetBlockFromBuffer(buff, cur_len);
        cur_block.block_number = block_counter++;

        // Get events from current block
        EVIOBlockedEventParser parser;
        auto events = parser.ParseEVIOBlockedEvent(cur_block, event);

        // Trace events for debugging
        m_log->info("Parsed block {} had {} events, swap_needed={}",
                     cur_block.block_number, events.size(), cur_block.swap_needed);
        for (size_t i = 0; i < events.size(); i++) {
            auto &parsed_event = events[i];
            m_log->trace("  Event #{} got event-number={}:", i, parsed_event->GetEventNumber());

            for (auto factory: parsed_event->GetFactorySet()->GetAllFactories()) {
                m_log->trace("    Factory = {:<30}  NumObjects = {}", factory->GetObjectName(),
                             factory->GetNumObjects());
            }
        }

        // Reset buff to prevent memory leakage
        delete[] buff;
        buff_len = DEFAULT_READ_BUFF_LEN;
        buff = new uint32_t[buff_len];
    }

    // Handle HDEVIO::HDEVIO_EOF
    if (m_hdevio->err_code == HDEVIO::HDEVIO_EOF) {
        throw RETURN_STATUS::kNO_MORE_EVENTS;
    } else {
        throw JException("Unhandled EVIO file reading return status " + m_hdevio->err_code, __FILE__, __LINE__);
    }

    m_log->info("Parsed {} blocks from {}", block_counter - 1, m_evio_filename);

    // clean buffer
    delete[] buff;
}

void CDAQEVIOFileSource::Open() {
    JApplication *app = GetApplication();

    m_log = app->GetService<Log_service>()->logger(GetPluginName());  // init spdlog
    m_evio_filename = GetResourceName();

    m_log->info("GetResourceName() = {}", m_evio_filename);

    CDAQEVIOFileSource::OpenEVIOFile(m_evio_filename);
}

std::string CDAQEVIOFileSource::GetDescription() {
    return "CDAQEVIOFileSource - Read an *.evio file and parse the blocks into JEvents.";
}


template<>
double JEventSourceGeneratorT<CDAQEVIOFileSource>::CheckOpenable(std::string resource_name) {

    /// CheckOpenable() decides how confident we are that this EventSource can handle this resource.
    ///    0.0        -> 'Cannot handle'
    ///    (0.0, 1.0] -> 'Can handle, with this confidence level'

    /// To determine confidence level, feel free to open up the file and check for magic bytes or metadata.
    /// Returning a confidence <- {0.0, 1.0} is perfectly OK!

    if (resource_name.find(".evio") != std::string::npos) return 0.5;

//    return (resource_name == "CDAQEVIOFileSource") ? 1.0 : 0.0;

}

