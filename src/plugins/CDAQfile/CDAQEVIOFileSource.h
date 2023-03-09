//
// Created by xmei@jlab.org on 2/9/23.
//

#ifndef JANA4ML4FPGA_CDAQEVIOFILESOURCE_H
#define JANA4ML4FPGA_CDAQEVIOFILESOURCE_H

#include <string>
#include <atomic>
#include <chrono>
#include <cinttypes>

#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <JANA/Compatibility/jerror.h>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JFactory.h>
#include <JANA/Compatibility/JStreamLog.h>

#include <evio/HDEVIO.h>
#include <evio/DModuleType.h>
#include <rawdataparser/EVIOBlockedEvent.h>

#include <extensions/spdlog/SpdlogMixin.h>

class CDAQEVIOFileSource :
        public JEventSource,
        public spdlog::extensions::SpdlogMixin<CDAQEVIOFileSource>
        {

public:

    CDAQEVIOFileSource(std::string resource_name, JApplication* app);

    virtual ~CDAQEVIOFileSource();

    void Open() override;

    void GetEvent(std::shared_ptr<JEvent>);

    static std::string GetDescription();

private:
    std::string m_evio_filename = "";
    HDEVIO *m_hdevio = nullptr;
    std::shared_ptr<spdlog::logger> m_log;

    void OpenEVIOFile(std::string filename);

    EVIOBlockedEvent GetBlockFromBuffer(uint32_t *buffer, uint32_t event_len);
};

template <>
double JEventSourceGeneratorT<CDAQEVIOFileSource>::CheckOpenable(std::string);

#endif //JANA4ML4FPGA_CDAQEVIOFILESOURCE_H
