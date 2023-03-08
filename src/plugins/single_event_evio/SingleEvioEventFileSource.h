//
// Created by Dmitry Romanov
//

#pragma once

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

#include <extensions/spdlog/SpdlogMixin.h>


class SingleEvioEventFileSource :
        public JEventSource,
        public spdlog::extensions::SpdlogMixin<SingleEvioEventFileSource>
{

public:
    SingleEvioEventFileSource(std::string resource_name, JApplication* app) : JEventSource(resource_name, app) {
        SetTypeName(NAME_OF_THIS); // Provide JANA with class name
    }
    void Open() override;

    void  GetEvent(std::shared_ptr<JEvent> event) override;

    static std::string GetDescription();

private:

    unique_ptr<HDEVIO> m_hdevio;
    uint32_t static const m_buff_len = 4000000;
    size_t m_block_number = 0;
};

template <>
double JEventSourceGeneratorT<SingleEvioEventFileSource>::CheckOpenable(std::string);

