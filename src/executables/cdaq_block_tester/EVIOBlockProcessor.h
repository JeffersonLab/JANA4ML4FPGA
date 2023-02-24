//
// Created by xmei on 2/24/23.
//

#ifndef JANA4ML4FPGA_EVIOBLOCKPROCESSOR_H
#define JANA4ML4FPGA_EVIOBLOCKPROCESSOR_H


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



#endif //JANA4ML4FPGA_EVIOBLOCKPROCESSOR_H
