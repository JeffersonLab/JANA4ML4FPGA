//
// Created by xmei on 2/24/23.
//

#pragma once

#include <JANA/JEventProcessor.h>

#include <spdlog/spdlog.h>
#include <TDirectory.h>

class EVIOBlockProcessor :
        public JEventProcessor
{

    // Shared state (e.g. histograms, TTrees, TFiles) live
    std::mutex m_mutex;

public:

    EVIOBlockProcessor();
    virtual ~EVIOBlockProcessor() = default;

    void Init() override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void Finish() override;
private:
    std::shared_ptr<spdlog::logger> m_log;
    /// Directory to store histograms to
    TDirectory *m_dir_main{};

};

