//
// Created by xmei on 2/24/23.
//

#ifndef JANA4ML4FPGA_EVIOBLOCKEDSOURCE_H
#define JANA4ML4FPGA_EVIOBLOCKEDSOURCE_H


class EVIOBlockedSource: public JBlockedEventSource<MyBlock> {

    int m_block_number = 1;
    JLogger m_logger;

    virtual void Initialize() {
        LOG_INFO(m_logger) <<  "Initializing JBlockedEventSource" << LOG_END;
    }

    virtual Status NextBlock(MyBlock& block) {
        LOG_DEBUG(m_logger) <<  "JBlockedEventSource::NextBlock" << LOG_END;

        if (m_block_number >= 10) {
            return Status::FailFinished;
        }

        block.block_number = m_block_number++;
        block.data.push_back(block.block_number*10 + 1);
        block.data.push_back(block.block_number*10 + 2);
        block.data.push_back(block.block_number*10 + 3);
        return Status::Success;
    }

    virtual std::vector<std::shared_ptr<JEvent>> DisentangleBlock(MyBlock& block, JEventPool& pool) {

        LOG_DEBUG(m_logger) <<  "JBlockedEventSource::DisentangleBlock" << LOG_END;
        std::vector<std::shared_ptr<JEvent>> events;
        for (auto datum : block.data) {
            auto event = pool.get(0);  // TODO: Make location be transparent to end user
            event->Insert(new MyObject(datum));
            events.push_back(event);
        }
        return events;
    }

};



#endif //JANA4ML4FPGA_EVIOBLOCKEDSOURCE_H
