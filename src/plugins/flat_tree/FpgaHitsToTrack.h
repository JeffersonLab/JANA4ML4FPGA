
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct FpgaHitsToTrack
    {

        uint32_t hit_id;
        uint32_t track_id;
    };
    
    class FpgaHitsToTrackIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("fpga_hit_track_count", &m_count, "fpga_hit_track_count/l");

            tree->Branch("fpga_hit_track_hit_id", &m_vect_hit_id);
            tree->Branch("fpga_hit_track_track_id", &m_vect_track_id);
        }

        void clear() override {
            m_count = 0;

            m_vect_hit_id.clear();
            m_vect_track_id.clear();
        }
        
        void add(const FpgaHitsToTrack& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add FpgaHitsToTrack data because FpgaHitsToTrackIO is not bound to tree");
            }
            m_count++;

            m_vect_hit_id.push_back(data.hit_id);
            m_vect_track_id.push_back(data.track_id);
        }
        
        bool isBoundToTree() const { return m_is_bound; }
        
    private:
        bool m_is_bound = false;
        uint64_t m_count;

        std::vector<uint32_t> m_vect_hit_id;
        std::vector<uint32_t> m_vect_track_id;
    };
}
