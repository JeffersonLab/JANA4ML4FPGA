
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct F125FDCPulseRecord
    {

        uint32_t roc;
        uint32_t slot;
        uint32_t channel;
        uint32_t npk;
        uint32_t le_time;
        uint32_t time_quality_bit;
        uint32_t overflow_count;
        uint32_t pedestal;
        uint32_t integral;
        uint32_t peak_amp;
        uint32_t peak_time;
        uint32_t word1;
        uint32_t word2;
        uint32_t nsamples_pedestal;
        uint32_t nsamples_integral;
        bool emulated;
        uint32_t le_time_emulated;
        uint32_t time_quality_bit_emulated;
        uint32_t overflow_count_emulated;
        uint32_t pedestal_emulated;
        uint32_t integral_emulated;
        uint32_t peak_amp_emulated;
        uint32_t peak_time_emulated;
    };
    
    class F125FDCPulseRecordIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("f125_pulse_count", &m_count, "f125_pulse_count/l");

            tree->Branch("f125_pulse_roc", &m_vect_roc);
            tree->Branch("f125_pulse_slot", &m_vect_slot);
            tree->Branch("f125_pulse_channel", &m_vect_channel);
            tree->Branch("f125_pulse_npk", &m_vect_npk);
            tree->Branch("f125_pulse_le_time", &m_vect_le_time);
            tree->Branch("f125_pulse_time_quality_bit", &m_vect_time_quality_bit);
            tree->Branch("f125_pulse_overflow_count", &m_vect_overflow_count);
            tree->Branch("f125_pulse_pedestal", &m_vect_pedestal);
            tree->Branch("f125_pulse_integral", &m_vect_integral);
            tree->Branch("f125_pulse_peak_amp", &m_vect_peak_amp);
            tree->Branch("f125_pulse_peak_time", &m_vect_peak_time);
            tree->Branch("f125_pulse_word1", &m_vect_word1);
            tree->Branch("f125_pulse_word2", &m_vect_word2);
            tree->Branch("f125_pulse_nsamples_pedestal", &m_vect_nsamples_pedestal);
            tree->Branch("f125_pulse_nsamples_integral", &m_vect_nsamples_integral);
            tree->Branch("f125_pulse_emulated", &m_vect_emulated);
            tree->Branch("f125_pulse_le_time_emulated", &m_vect_le_time_emulated);
            tree->Branch("f125_pulse_time_quality_bit_emulated", &m_vect_time_quality_bit_emulated);
            tree->Branch("f125_pulse_overflow_count_emulated", &m_vect_overflow_count_emulated);
            tree->Branch("f125_pulse_pedestal_emulated", &m_vect_pedestal_emulated);
            tree->Branch("f125_pulse_integral_emulated", &m_vect_integral_emulated);
            tree->Branch("f125_pulse_peak_amp_emulated", &m_vect_peak_amp_emulated);
            tree->Branch("f125_pulse_peak_time_emulated", &m_vect_peak_time_emulated);
        }

        void clear() override {
            m_count = 0;

            m_vect_roc.clear();
            m_vect_slot.clear();
            m_vect_channel.clear();
            m_vect_npk.clear();
            m_vect_le_time.clear();
            m_vect_time_quality_bit.clear();
            m_vect_overflow_count.clear();
            m_vect_pedestal.clear();
            m_vect_integral.clear();
            m_vect_peak_amp.clear();
            m_vect_peak_time.clear();
            m_vect_word1.clear();
            m_vect_word2.clear();
            m_vect_nsamples_pedestal.clear();
            m_vect_nsamples_integral.clear();
            m_vect_emulated.clear();
            m_vect_le_time_emulated.clear();
            m_vect_time_quality_bit_emulated.clear();
            m_vect_overflow_count_emulated.clear();
            m_vect_pedestal_emulated.clear();
            m_vect_integral_emulated.clear();
            m_vect_peak_amp_emulated.clear();
            m_vect_peak_time_emulated.clear();
        }
        
        void add(const F125FDCPulseRecord& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add F125FDCPulseRecord data because F125FDCPulseRecordIO is not bound to tree");
            }
            m_count++;

            m_vect_roc.push_back(data.roc);
            m_vect_slot.push_back(data.slot);
            m_vect_channel.push_back(data.channel);
            m_vect_npk.push_back(data.npk);
            m_vect_le_time.push_back(data.le_time);
            m_vect_time_quality_bit.push_back(data.time_quality_bit);
            m_vect_overflow_count.push_back(data.overflow_count);
            m_vect_pedestal.push_back(data.pedestal);
            m_vect_integral.push_back(data.integral);
            m_vect_peak_amp.push_back(data.peak_amp);
            m_vect_peak_time.push_back(data.peak_time);
            m_vect_word1.push_back(data.word1);
            m_vect_word2.push_back(data.word2);
            m_vect_nsamples_pedestal.push_back(data.nsamples_pedestal);
            m_vect_nsamples_integral.push_back(data.nsamples_integral);
            m_vect_emulated.push_back(data.emulated);
            m_vect_le_time_emulated.push_back(data.le_time_emulated);
            m_vect_time_quality_bit_emulated.push_back(data.time_quality_bit_emulated);
            m_vect_overflow_count_emulated.push_back(data.overflow_count_emulated);
            m_vect_pedestal_emulated.push_back(data.pedestal_emulated);
            m_vect_integral_emulated.push_back(data.integral_emulated);
            m_vect_peak_amp_emulated.push_back(data.peak_amp_emulated);
            m_vect_peak_time_emulated.push_back(data.peak_time_emulated);
        }
        
        bool isBoundToTree() const { return m_is_bound; }
        
    private:
        bool m_is_bound = false;
        uint64_t m_count;

        std::vector<uint32_t> m_vect_roc;
        std::vector<uint32_t> m_vect_slot;
        std::vector<uint32_t> m_vect_channel;
        std::vector<uint32_t> m_vect_npk;
        std::vector<uint32_t> m_vect_le_time;
        std::vector<uint32_t> m_vect_time_quality_bit;
        std::vector<uint32_t> m_vect_overflow_count;
        std::vector<uint32_t> m_vect_pedestal;
        std::vector<uint32_t> m_vect_integral;
        std::vector<uint32_t> m_vect_peak_amp;
        std::vector<uint32_t> m_vect_peak_time;
        std::vector<uint32_t> m_vect_word1;
        std::vector<uint32_t> m_vect_word2;
        std::vector<uint32_t> m_vect_nsamples_pedestal;
        std::vector<uint32_t> m_vect_nsamples_integral;
        std::vector<bool> m_vect_emulated;
        std::vector<uint32_t> m_vect_le_time_emulated;
        std::vector<uint32_t> m_vect_time_quality_bit_emulated;
        std::vector<uint32_t> m_vect_overflow_count_emulated;
        std::vector<uint32_t> m_vect_pedestal_emulated;
        std::vector<uint32_t> m_vect_integral_emulated;
        std::vector<uint32_t> m_vect_peak_amp_emulated;
        std::vector<uint32_t> m_vect_peak_time_emulated;
    };
}
