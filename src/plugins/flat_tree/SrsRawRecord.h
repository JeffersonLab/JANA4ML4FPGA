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
            tree->Branch("srs_raw_samples_index", &m_vect_samples_index);
            tree->Branch("srs_raw_samples_count", &m_vect_samples_count);
            tree->Branch("srs_raw_samples", &m_vect_samples);
        }

        void clear() override {
            m_count = 0;
            m_samples_index = 0;
            m_samples_count = 0;
            m_vect_roc.clear();
            m_vect_slot.clear();
            m_vect_channel.clear();
            m_vect_apv_id.clear();
            m_vect_channel_apv.clear();
            m_vect_best_sample.clear();
            m_vect_samples.clear();
            m_vect_samples_count.clear();
            m_vect_samples_index.clear();
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
            for(unsigned short sample : data.samples)
            {
                m_vect_samples.push_back(sample);
            }
            // m_samples_size now contains previous record sample size
            // m_samples_size=0 for the first sample
            // (!) so we have to first increase m_samples_index and THEN set current m_samples_size
            m_samples_index += m_samples_count;
            m_samples_count = data.samples.size();
            m_vect_samples_count.push_back(m_samples_count);
            m_vect_samples_index.push_back(m_samples_index);

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
        uint64_t m_samples_index;
        uint64_t m_samples_count;
        std::vector<uint16_t> m_vect_samples;
        std::vector<uint64_t> m_vect_samples_count;
        std::vector<uint64_t> m_vect_samples_index;
    };
}
