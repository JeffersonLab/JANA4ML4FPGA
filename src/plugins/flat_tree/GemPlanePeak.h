
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct GemPlanePeak
    {

        uint32_t plane_id;
        std::string plane_name;
        uint32_t index;
        uint32_t apv_id;
        uint32_t time_id;
        double height;
        double width;
        double area;
        double real_pos;
    };
    
    class GemPlanePeakIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("gem_peak_count", &m_count, "gem_peak_count/l");

            tree->Branch("gem_peak_plane_id", &m_vect_plane_id);
            tree->Branch("gem_peak_plane_name", &m_vect_plane_name);
            tree->Branch("gem_peak_index", &m_vect_index);
            tree->Branch("gem_peak_apv_id", &m_vect_apv_id);
            tree->Branch("gem_peak_time_id", &m_vect_time_id);
            tree->Branch("gem_peak_height", &m_vect_height);
            tree->Branch("gem_peak_width", &m_vect_width);
            tree->Branch("gem_peak_area", &m_vect_area);
            tree->Branch("gem_peak_real_pos", &m_vect_real_pos);
        }

        void clear() override {
            m_count = 0;

            m_vect_plane_id.clear();
            m_vect_plane_name.clear();
            m_vect_index.clear();
            m_vect_apv_id.clear();
            m_vect_time_id.clear();
            m_vect_height.clear();
            m_vect_width.clear();
            m_vect_area.clear();
            m_vect_real_pos.clear();
        }
        
        void add(const GemPlanePeak& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add GemPlanePeak data because GemPlanePeakIO is not bound to tree");
            }
            m_count++;

            m_vect_plane_id.push_back(data.plane_id);
            m_vect_plane_name.push_back(data.plane_name);
            m_vect_index.push_back(data.index);
            m_vect_apv_id.push_back(data.apv_id);
            m_vect_time_id.push_back(data.time_id);
            m_vect_height.push_back(data.height);
            m_vect_width.push_back(data.width);
            m_vect_area.push_back(data.area);
            m_vect_real_pos.push_back(data.real_pos);
        }
        
        bool isBoundToTree() const { return m_is_bound; }
        
    private:
        bool m_is_bound = false;
        uint64_t m_count;

        std::vector<uint32_t> m_vect_plane_id;
        std::vector<std::string> m_vect_plane_name;
        std::vector<uint32_t> m_vect_index;
        std::vector<uint32_t> m_vect_apv_id;
        std::vector<uint32_t> m_vect_time_id;
        std::vector<double> m_vect_height;
        std::vector<double> m_vect_width;
        std::vector<double> m_vect_area;
        std::vector<double> m_vect_real_pos;
    };
}
