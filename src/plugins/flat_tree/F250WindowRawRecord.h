
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct F250WindowRawRecord
    {
        uint32_t roc;
        uint32_t slot;
        uint32_t channel;
        bool invalid_samples;
        bool overflow;
        uint32_t itrigger;
        std::vector<uint16_t> samples;
    };

    class F250WindowRawRecordIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("f250_wraw_count", &m_count, "f250_wraw_count/l");
            tree->Branch("f250_wraw_roc", &m_vect_roc);
            tree->Branch("f250_wraw_slot", &m_vect_slot);
            tree->Branch("f250_wraw_channel", &m_vect_channel);
            tree->Branch("f250_wraw_invalid_samples", &m_vect_invalid_samples);
            tree->Branch("f250_wraw_overflow", &m_vect_overflow);
            tree->Branch("f250_wraw_itrigger", &m_vect_itrigger);
            tree->Branch("f250_wraw_samples_index", &m_vect_samples_index);
            tree->Branch("f250_wraw_samples_count", &m_vect_samples_count);
            tree->Branch("f250_wraw_samples", &m_vect_samples);
        }

        void clear() override {
            m_count = 0;
            m_vect_samples_count.clear();
            m_vect_samples_index.clear();
            m_vect_roc.clear();
            m_vect_slot.clear();
            m_vect_channel.clear();
            m_vect_invalid_samples.clear();
            m_vect_overflow.clear();
            m_vect_itrigger.clear();
            m_vect_samples.clear();
        }

        void add(const F250WindowRawRecord& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add F250WindowRawRecord data because F250WindowRawRecordIO is not bound to tree");
            }
            m_count++;
            m_vect_roc.push_back(data.roc);
            m_vect_slot.push_back(data.slot);
            m_vect_channel.push_back(data.channel);
            m_vect_invalid_samples.push_back(data.invalid_samples);
            m_vect_overflow.push_back(data.overflow);
            m_vect_itrigger.push_back(data.itrigger);

            for(size_t i=0; i < data.samples.size(); i++)
            {
                m_vect_samples.push_back(data.samples[i]);
            }

            // First record, samples index = 0
            if(m_vect_samples_count.size() == 0) {
                m_vect_samples_index.push_back(0);
            } else {
                auto last_count = m_vect_samples_count[m_vect_samples_count.size() - 1];
                auto last_index = m_vect_samples_index[m_vect_samples_index.size() - 1];
                m_vect_samples_index.push_back(last_index+last_count);
            }
            m_vect_samples_count.push_back(data.samples.size());
        }

        bool isBoundToTree() const { return m_is_bound; }

    private:
        bool m_is_bound = false;
        uint64_t m_count;
        std::vector<uint32_t> m_vect_roc;
        std::vector<uint32_t> m_vect_slot;
        std::vector<uint32_t> m_vect_channel;
        std::vector<bool> m_vect_invalid_samples;
        std::vector<bool> m_vect_overflow;
        std::vector<uint32_t> m_vect_itrigger;
        std::vector<uint64_t> m_vect_samples_count;
        std::vector<uint64_t> m_vect_samples_index;
        std::vector<uint16_t> m_vect_samples;
    };
}
