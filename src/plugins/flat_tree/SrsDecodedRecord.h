
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct SrsDecodedRecord
    {

        uint32_t apv_id;
        std::string plane_name;
        std::string detector;
        std::vector<uint16_t> samples;
    };
    
    class SrsDecodedRecordIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("srs_decoded_count", &m_count, "srs_decoded_count/l");

            tree->Branch("srs_decoded_apv_id", &m_vect_apv_id);
            tree->Branch("srs_decoded_plane_name", &m_vect_plane_name);
            tree->Branch("srs_decoded_detector", &m_vect_detector);
            tree->Branch("srs_decoded_samples_index", &m_vect_samples_index);
            tree->Branch("srs_decoded_samples_count", &m_vect_samples_count);
            tree->Branch("srs_decoded_samples", &m_vect_samples);
        }

        void clear() override {
            m_count = 0;

            m_vect_apv_id.clear();
            m_vect_plane_name.clear();
            m_vect_detector.clear();
            m_vect_samples.clear();
            m_vect_samples_index.clear();
            m_vect_samples_count.clear();
        }
        
        void add(const SrsDecodedRecord& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add SrsDecodedRecord data because SrsDecodedRecordIO is not bound to tree");
            }
            m_count++;

            m_vect_apv_id.push_back(data.apv_id);
            m_vect_plane_name.push_back(data.plane_name);
            m_vect_detector.push_back(data.detector);
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

        std::vector<uint32_t> m_vect_apv_id;
        std::vector<std::string> m_vect_plane_name;
        std::vector<std::string> m_vect_detector;
        std::vector<uint16_t> m_vect_samples_count;
        std::vector<uint16_t> m_vect_samples_index;
        std::vector<uint16_t> m_vect_samples;
    };
}
