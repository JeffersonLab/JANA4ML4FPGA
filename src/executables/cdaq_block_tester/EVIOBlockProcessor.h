//
// Created by xmei on 2/24/23.
//

#pragma once

#include <JANA/JEventProcessor.h>

class EVIOBlockProcessor : public JEventProcessor {

    // Shared state (e.g. histograms, TTrees, TFiles) live
    std::mutex m_mutex;

public:

    EVIOBlockProcessor();
    virtual ~EVIOBlockProcessor() = default;

    void Init() override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void Finish() override;

};

