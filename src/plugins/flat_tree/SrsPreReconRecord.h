
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct SrsPreReconRecord
    {

        double y;
        double x;
    };
    
    class SrsPreReconRecordIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("srs_prerecon__count", &m_count, "srs_prerecon__count/l");

            tree->Branch("srs_prerecon__y", &m_vect_y);
            tree->Branch("srs_prerecon__x", &m_vect_x);
        }

        void clear() override {
            m_count = 0;

            m_vect_y.clear();
            m_vect_x.clear();
        }
        
        void add(const SrsPreReconRecord& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add SrsPreReconRecord data because SrsPreReconRecordIO is not bound to tree");
            }
            m_count++;

            m_vect_y.push_back(data.y);
            m_vect_x.push_back(data.x);
        }
        
        bool isBoundToTree() const { return m_is_bound; }
        
    private:
        bool m_is_bound = false;
        uint64_t m_count;

        std::vector<double> m_vect_y;
        std::vector<double> m_vect_x;
    };
}
