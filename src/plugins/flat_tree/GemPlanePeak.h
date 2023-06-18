
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct GemPlanePeak
    {

        uint32_t plane_id;
        uint32_t channel;
        uint32_t apv_id;
        double height;
        double width;
        double area;
    };
    
    class GemPlanePeakIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("gem_peak_count", &m_count, "gem_peak_count/l");

            tree->Branch("gem_peak_plane_id", &m_vect_plane_id);
            tree->Branch("gem_peak_channel", &m_vect_channel);
            tree->Branch("gem_peak_apv_id", &m_vect_apv_id);
            tree->Branch("gem_peak_height", &m_vect_height);
            tree->Branch("gem_peak_width", &m_vect_width);
            tree->Branch("gem_peak_area", &m_vect_area);
        }

        void clear() override {
            m_count = 0;

            m_vect_plane_id.clear();
            m_vect_channel.clear();
            m_vect_apv_id.clear();
            m_vect_height.clear();
            m_vect_width.clear();
            m_vect_area.clear();
        }
        
        void add(const GemPlanePeak& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add GemPlanePeak data because GemPlanePeakIO is not bound to tree");
            }
            m_count++;

            m_vect_plane_id.push_back(data.plane_id);
            m_vect_channel.push_back(data.channel);
            m_vect_apv_id.push_back(data.apv_id);
            m_vect_height.push_back(data.height);
            m_vect_width.push_back(data.width);
            m_vect_area.push_back(data.area);
        }
        
        bool isBoundToTree() const { return m_is_bound; }
        
    private:
        bool m_is_bound = false;
        uint64_t m_count;

        std::vector<uint32_t> m_vect_plane_id;
        std::vector<uint32_t> m_vect_channel;
        std::vector<uint32_t> m_vect_apv_id;
        std::vector<double> m_vect_height;
        std::vector<double> m_vect_width;
        std::vector<double> m_vect_area;
    };
}
