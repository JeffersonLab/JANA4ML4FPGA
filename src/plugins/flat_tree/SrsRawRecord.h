
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct SrsRawRecord
    {

        uint32_t roc;
        uint32_t slot;
        uint32_t channel;
        uint32_t apv_id;
        uint32_t channel_apv;
        uint16_t best_sample;
        std::vector<uint16_t> samples;
    };
    
    class SrsRawRecordIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("srs_raw_count", &m_count, "srs_raw_count/l");

            tree->Branch("srs_raw_roc", &m_vect_roc);
            tree->Branch("srs_raw_slot", &m_vect_slot);
            tree->Branch("srs_raw_channel", &m_vect_channel);
            tree->Branch("srs_raw_apv_id", &m_vect_apv_id);
            tree->Branch("srs_raw_channel_apv", &m_vect_channel_apv);
            tree->Branch("srs_raw_best_sample", &m_vect_best_sample);
            tree->Branch("srs_raw_samples_index", &m_vect_samples_index);
            tree->Branch("srs_raw_samples_count", &m_vect_samples_count);
            tree->Branch("srs_raw_samples", &m_vect_samples);
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
            m_vect_samples_index.clear();
            m_vect_samples_count.clear();
        }
        
        void add(const SrsRawRecord& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add SrsRawRecord data because SrsRawRecordIO is not bound to tree");
            }
            m_count++;

            m_vect_roc.push_back(data.roc);
            m_vect_slot.push_back(data.slot);
            m_vect_channel.push_back(data.channel);
            m_vect_apv_id.push_back(data.apv_id);
            m_vect_channel_apv.push_back(data.channel_apv);
            m_vect_best_sample.push_back(data.best_sample);
            for(auto item: data.samples) m_vect_samples.push_back(item);
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
        std::vector<uint32_t> m_vect_apv_id;
        std::vector<uint32_t> m_vect_channel_apv;
        std::vector<uint16_t> m_vect_best_sample;
        std::vector<uint16_t> m_vect_samples_count;
        std::vector<uint16_t> m_vect_samples_index;
        std::vector<uint16_t> m_vect_samples;
    };
}
