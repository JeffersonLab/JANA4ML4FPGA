//
// Created by xme@jlab.org on 2/9/23.
//

#include <rawdataparser/EVIOBlockedEventParser.h>

#include "CDAQEVIOFileSource.h"

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

    m_hdevio->PrintStats();
    m_hdevio.reset();

    if( m_buff ) delete[] m_buff;
    m_evio_filename = "";
}

void CDAQEVIOFileSource::OpenEVIOFile(std::string filename) {
    m_hdevio = std::make_unique<HDEVIO>(filename, true, 2);   // 2 for VERBOSE level
    if (!m_hdevio->is_open) {
        cerr <<m_hdevio->err_mess.str() << endl;
        throw JException("Failed to open EVIO file: " + filename, __FILE__, __LINE__);
        // throw exception indicating error
    }
    m_log->info("Open EVIO file {} successfully", filename);
}

EVIOBlockedEvent CDAQEVIOFileSource::GetBlockFromBuffer(uint32_t event_len) {
    EVIOBlockedEvent block;

    block.block_number = m_block_number++;
    block.swap_needed = m_hdevio->swap_needed;
    block.data.insert(block.data.begin(), m_buff, m_buff + event_len);

    return block;
}

/**
 * Read the evio file with HDEVIO::readNoFileBuff().
 * Handle three cases: read_ok, HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL, HDEVIO::HDEVIO_EOF.
 */
void CDAQEVIOFileSource::GetEvent(std::shared_ptr<JEvent> event) {
    m_buff = new uint32_t[m_buff_len];
    bool read_ok = m_hdevio->readNoFileBuff(m_buff, m_buff_len);
    uint32_t cur_len = m_hdevio->last_event_len;

    // Handle not read_ok
    if (not read_ok) {
        if (m_hdevio->err_code == HDEVIO::HDEVIO_EOF) {
            throw RETURN_STATUS::kNO_MORE_EVENTS;
        } else if (m_hdevio->err_code == HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL) {
            m_buff_len = cur_len;
            delete[] m_buff;
            m_buff = nullptr;
            throw RETURN_STATUS::kTRY_AGAIN;
        } else {
            throw JException("Unhandled EVIO file reading return status " + m_hdevio->err_code, __FILE__, __LINE__);
        }
    }

    EVIOBlockedEvent cur_block = GetBlockFromBuffer(cur_len);

    // Get events from current block
    EVIOBlockedEventParser parser;
    auto events = parser.ParseEVIOBlockedEvent(cur_block, event);
    m_log->debug("Parsed block {} had {} events, swap_needed={}",
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
    delete[] m_buff;
    m_buff = nullptr;
    m_buff_len = DEFAULT_READ_BUFF_LEN;
}

void CDAQEVIOFileSource::Open() {
    InitLogger("CDAQEVIOFileSource");  // init spdlog
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
    return 0;

//    return (resource_name == "CDAQEVIOFileSource") ? 1.0 : 0.0;

}

