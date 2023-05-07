//
// Created by romanov on 5/7/2023.
//
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct GemSimpleCluster
    {
        double x;
        double y;
        double energy;
        double adc;
    };

    class GemSimpleClusterIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("gem_scluster_count", &m_count, "gem_scluster_count/l");
            tree->Branch("gem_scluster_x", &m_vect_x);
            tree->Branch("gem_scluster_y", &m_vect_y);
            tree->Branch("gem_scluster_energy", &m_vect_energy);
            tree->Branch("gem_scluster_adc", &m_vect_adc);
        }

        void clear() override {
            m_count = 0;
            m_vect_x.clear();
            m_vect_y.clear();
            m_vect_energy.clear();
            m_vect_adc.clear();
        }

        void add(const GemSimpleCluster& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add GemSimpleCluster data because GemSimpleClusterIO is not bound to tree");
            }
            m_count++;
            m_vect_x.push_back(data.x);
            m_vect_y.push_back(data.y);
            m_vect_energy.push_back(data.energy);
            m_vect_adc.push_back(data.adc);
        }

        bool isBoundToTree() const { return m_is_bound; }

    private:
        bool m_is_bound = false;
        uint64_t m_count;
        std::vector<double> m_vect_x;
        std::vector<double> m_vect_y;
        std::vector<double> m_vect_energy;
        std::vector<double> m_vect_adc;
    };
}
