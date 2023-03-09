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

#define DEFAULT_READ_BUFF_LEN 4000000

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
    unique_ptr<HDEVIO> m_hdevio;

    uint32_t *m_buff = nullptr;
    uint32_t m_buff_len = DEFAULT_READ_BUFF_LEN;
    int m_block_number = 1;

    void OpenEVIOFile(std::string filename);

    EVIOBlockedEvent GetBlockFromBuffer(uint32_t event_len);
};

template <>
double JEventSourceGeneratorT<CDAQEVIOFileSource>::CheckOpenable(std::string);

#endif //JANA4ML4FPGA_CDAQEVIOFILESOURCE_H
