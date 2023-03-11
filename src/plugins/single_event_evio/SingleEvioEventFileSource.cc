//
// Created by Dmitry Romanov
//

#include "SingleEvioEventFileSource.h"
#include "rawdataparser/EVIOBlockedEvent.h"
#include "rawdataparser/EVIOBlockedEventParser.h"

#include <memory>

using namespace std;
using namespace std::chrono;

void SingleEvioEventFileSource::Open() {
    InitLogger("SingleEvioEventFileSource");
    logger()->debug("Hello from {}", GetDescription());

    m_hdevio = std::make_unique<HDEVIO>(GetName(), true, 2);   // 2 for VERBOSE level

    if (!m_hdevio->is_open) {
        auto message = "Failed to open EVIO file: " + GetName();
        logger()->error(message);
        throw JException(message, __FILE__, __LINE__);  // throw exception indicating error
    }

    logger()->info("Success opening {}", GetName());
}

std::string SingleEvioEventFileSource::GetDescription() {
    return "SingleEvioEventFileSource - Reads from *.evio file that contains blocks of single events";
}

void SingleEvioEventFileSource::GetEvent(std::shared_ptr<JEvent> event) {
    m_log->debug("SingleEvioEventFileSource::GetEvent");

    // Read buffer containing blocked event into the given "block" object.
    //
    // If we are unable to read a new block, but may be able to later
    // (e.g. network source) then return Status::FailTryAgain
    //
    // If we are unable to read a new block and will never get another
    // (e.g. end of file) then return Status::FailFinish
    //
    // If we successfully read a block then retun Status::Success
    //
    // HDEVIO->readNoFileBuff(block.buff, block.buff_len);

    EVIOBlockedEvent block;
    block.data.resize(m_buff_len);
    uint32_t* buff = block.data.data();
    bool read_ok = m_hdevio->readNoFileBuff(buff, m_buff_len);
    uint32_t cur_len = m_hdevio->last_event_len;

    if (read_ok) { // copy data to block
        m_log->debug("Block {} read_ok!", m_block_number);
        block.block_number = m_block_number++;
        block.swap_needed = m_hdevio->swap_needed;  /// var not in use, may delete later
        // Disentangle block into multiple events
        EVIOBlockedEventParser parser; // TODO: make this persistent // (xmei@jlab.org) what does this mean

        auto events = parser.ParseEVIOBlockedEvent(block, event);
        m_log->trace("Parsed block had {} events:", events.size());

        if(m_log->level() <= spdlog::level::trace) {
            for (size_t i = 0; i < events.size(); i++) {
                auto &parsed_event = events[i];
                m_log->trace("  Event #{} in block with event-number={}:", i, parsed_event->GetEventNumber());

                for (auto factory: parsed_event->GetFactorySet()->GetAllFactories()) {
                    m_log->trace("    Factory = {:<30}  NumObjects = {}", factory->GetObjectName(),
                                 factory->GetNumObjects());
                }
            }
        }
        return;
    }

    // (!) So read_ok is NOT OK

    // End of file?
    if (m_hdevio->err_code == HDEVIO::HDEVIO_EOF) {
        // End of file
        m_log->info("No more blocks in {}. This is the end, my only friend", GetName());
        throw RETURN_STATUS::kNO_MORE_EVENTS;
    }

    // Large event?
    if (m_hdevio->err_code == HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL) {
        m_log->error("Block {} HDEVIO_USER_BUFFER_TOO_SMALL. Ejecting without parachute");
        throw RETURN_STATUS::kERROR;
    }

    // keep it ugly now
    throw JException("Unhandled EVIO file read return status: " , __FILE__, __LINE__);
}


template<>
double JEventSourceGeneratorT<SingleEvioEventFileSource>::CheckOpenable(std::string resource_name) {

    /// CheckOpenable() decides how confident we are that this EventSource can handle this resource.
    ///    0.0        -> 'Cannot handle'
    ///    (0.0, 1.0] -> 'Can handle, with this confidence level'
    if(resource_name.find(".evio") != std::string::npos) {
        return 0.75;
    }
    return 0;
}