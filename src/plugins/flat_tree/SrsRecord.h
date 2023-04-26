
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct SrsRecord
    {
        uint32_t roc;
        uint32_t slot;
        uint32_t channel;
        uint32_t apv_id;
        uint32_t channel_apv;
        uint16_t best_sample;
        std::vector<int> samples;
    };
    
    class SrsRecordIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("srs_count", &m_count, "srs_count/l");
            tree->Branch("srs_roc", &m_vect_roc);
            tree->Branch("srs_slot", &m_vect_slot);
            tree->Branch("srs_channel", &m_vect_channel);
            tree->Branch("srs_apv_id", &m_vect_apv_id);
            tree->Branch("srs_channel_apv", &m_vect_channel_apv);
            tree->Branch("srs_best_sample", &m_vect_best_sample);
            tree->Branch("srs_samples", &m_vect_samples);
        }

        void clear() override {
            m_count = 0;
            m_vect_roc.clear();
            m_vect_slot.clear();
            m_vect_channel.clear();
            m_vect_apv_id.clear();
            m_vect_channel_apv.clear();
            m_vect_best_sample.clear();
            m_vect_samples.clear();
        }
        
        void add(const SrsRecord& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add SrsRecord data because SrsRecordIO is not bound to tree");
            }
            m_count++;
            m_vect_roc.push_back(data.roc);
            m_vect_slot.push_back(data.slot);
            m_vect_channel.push_back(data.channel);
            m_vect_apv_id.push_back(data.apv_id);
            m_vect_channel_apv.push_back(data.channel_apv);
            m_vect_best_sample.push_back(data.best_sample);
            m_vect_samples.push_back(data.samples);
        }
        
        bool isBoundToTree() const { return m_is_bound; }
        
    private:
        bool m_is_bound = false;
        uint64_t m_count;
        std::vector<uint32_t> m_vect_roc;
        std::vector<uint32_t> m_vect_slot;
        std::vector<uint32_t> m_vect_channel;
        std::vector<uint32_t> m_vect_apv_id;
        std::vector<uint32_t> m_vect_channel_apv;
        std::vector<uint16_t> m_vect_best_sample;
        std::vector<std::vector<int>> m_vect_samples;
    };
}
