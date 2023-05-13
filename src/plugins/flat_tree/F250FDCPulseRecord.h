
#pragma once

#include <cstdint>
#include <vector>
#include <TTree.h>
#include "AlignedArraysIO.h"

namespace flatio {
    struct F250FDCPulseRecord
    {

        uint32_t event_within_block;
        bool qf_pedestal;
        uint32_t pedestal;
        uint32_t integral;
        bool qf_nsa_beyond_ptw;
        bool qf_overflow;
        bool qf_underflow;
        uint32_t nsamples_over_threshold;
        uint32_t course_time;
        uint32_t fine_time;
        uint32_t pulse_peak;
        bool qf_vpeak_beyond_nsa;
        bool qf_vpeak_not_found;
        bool qf_bad_pedestal;
        uint32_t pulse_number;
        uint32_t nsamples_integral;
        uint32_t nsamples_pedestal;
        bool emulated;
        uint32_t integral_emulated;
        uint32_t pedestal_emulated;
        uint32_t time_emulated;
        uint32_t course_time_emulated;
        uint32_t fine_time_emulated;
        uint32_t pulse_peak_emulated;
        uint32_t qf_emulated;
    };
    
    class F250FDCPulseRecordIO: public AlignedArraysIO
    {
    public:
        void bindToTree(TTree *tree) override {
            m_is_bound = true;
            tree->Branch("f250_pulse_count", &m_count, "f250_pulse_count/l");

            tree->Branch("f250_pulse_event_within_block", &m_vect_event_within_block);
            tree->Branch("f250_pulse_qf_pedestal", &m_vect_qf_pedestal);
            tree->Branch("f250_pulse_pedestal", &m_vect_pedestal);
            tree->Branch("f250_pulse_integral", &m_vect_integral);
            tree->Branch("f250_pulse_qf_nsa_beyond_ptw", &m_vect_qf_nsa_beyond_ptw);
            tree->Branch("f250_pulse_qf_overflow", &m_vect_qf_overflow);
            tree->Branch("f250_pulse_qf_underflow", &m_vect_qf_underflow);
            tree->Branch("f250_pulse_nsamples_over_threshold", &m_vect_nsamples_over_threshold);
            tree->Branch("f250_pulse_course_time", &m_vect_course_time);
            tree->Branch("f250_pulse_fine_time", &m_vect_fine_time);
            tree->Branch("f250_pulse_pulse_peak", &m_vect_pulse_peak);
            tree->Branch("f250_pulse_qf_vpeak_beyond_nsa", &m_vect_qf_vpeak_beyond_nsa);
            tree->Branch("f250_pulse_qf_vpeak_not_found", &m_vect_qf_vpeak_not_found);
            tree->Branch("f250_pulse_qf_bad_pedestal", &m_vect_qf_bad_pedestal);
            tree->Branch("f250_pulse_pulse_number", &m_vect_pulse_number);
            tree->Branch("f250_pulse_nsamples_integral", &m_vect_nsamples_integral);
            tree->Branch("f250_pulse_nsamples_pedestal", &m_vect_nsamples_pedestal);
            tree->Branch("f250_pulse_emulated", &m_vect_emulated);
            tree->Branch("f250_pulse_integral_emulated", &m_vect_integral_emulated);
            tree->Branch("f250_pulse_pedestal_emulated", &m_vect_pedestal_emulated);
            tree->Branch("f250_pulse_time_emulated", &m_vect_time_emulated);
            tree->Branch("f250_pulse_course_time_emulated", &m_vect_course_time_emulated);
            tree->Branch("f250_pulse_fine_time_emulated", &m_vect_fine_time_emulated);
            tree->Branch("f250_pulse_pulse_peak_emulated", &m_vect_pulse_peak_emulated);
            tree->Branch("f250_pulse_qf_emulated", &m_vect_qf_emulated);
        }

        void clear() override {
            m_count = 0;

            m_vect_event_within_block.clear();
            m_vect_qf_pedestal.clear();
            m_vect_pedestal.clear();
            m_vect_integral.clear();
            m_vect_qf_nsa_beyond_ptw.clear();
            m_vect_qf_overflow.clear();
            m_vect_qf_underflow.clear();
            m_vect_nsamples_over_threshold.clear();
            m_vect_course_time.clear();
            m_vect_fine_time.clear();
            m_vect_pulse_peak.clear();
            m_vect_qf_vpeak_beyond_nsa.clear();
            m_vect_qf_vpeak_not_found.clear();
            m_vect_qf_bad_pedestal.clear();
            m_vect_pulse_number.clear();
            m_vect_nsamples_integral.clear();
            m_vect_nsamples_pedestal.clear();
            m_vect_emulated.clear();
            m_vect_integral_emulated.clear();
            m_vect_pedestal_emulated.clear();
            m_vect_time_emulated.clear();
            m_vect_course_time_emulated.clear();
            m_vect_fine_time_emulated.clear();
            m_vect_pulse_peak_emulated.clear();
            m_vect_qf_emulated.clear();
        }
        
        void add(const F250FDCPulseRecord& data) {
            if(!m_is_bound) {
                throw std::logic_error("Can't add F250FDCPulseRecord data because F250FDCPulseRecordIO is not bound to tree");
            }
            m_count++;

            m_vect_event_within_block.push_back(data.event_within_block);
            m_vect_qf_pedestal.push_back(data.qf_pedestal);
            m_vect_pedestal.push_back(data.pedestal);
            m_vect_integral.push_back(data.integral);
            m_vect_qf_nsa_beyond_ptw.push_back(data.qf_nsa_beyond_ptw);
            m_vect_qf_overflow.push_back(data.qf_overflow);
            m_vect_qf_underflow.push_back(data.qf_underflow);
            m_vect_nsamples_over_threshold.push_back(data.nsamples_over_threshold);
            m_vect_course_time.push_back(data.course_time);
            m_vect_fine_time.push_back(data.fine_time);
            m_vect_pulse_peak.push_back(data.pulse_peak);
            m_vect_qf_vpeak_beyond_nsa.push_back(data.qf_vpeak_beyond_nsa);
            m_vect_qf_vpeak_not_found.push_back(data.qf_vpeak_not_found);
            m_vect_qf_bad_pedestal.push_back(data.qf_bad_pedestal);
            m_vect_pulse_number.push_back(data.pulse_number);
            m_vect_nsamples_integral.push_back(data.nsamples_integral);
            m_vect_nsamples_pedestal.push_back(data.nsamples_pedestal);
            m_vect_emulated.push_back(data.emulated);
            m_vect_integral_emulated.push_back(data.integral_emulated);
            m_vect_pedestal_emulated.push_back(data.pedestal_emulated);
            m_vect_time_emulated.push_back(data.time_emulated);
            m_vect_course_time_emulated.push_back(data.course_time_emulated);
            m_vect_fine_time_emulated.push_back(data.fine_time_emulated);
            m_vect_pulse_peak_emulated.push_back(data.pulse_peak_emulated);
            m_vect_qf_emulated.push_back(data.qf_emulated);
        }
        
        bool isBoundToTree() const { return m_is_bound; }
        
    private:
        bool m_is_bound = false;
        uint64_t m_count;

        std::vector<uint32_t> m_vect_event_within_block;
        std::vector<bool> m_vect_qf_pedestal;
        std::vector<uint32_t> m_vect_pedestal;
        std::vector<uint32_t> m_vect_integral;
        std::vector<bool> m_vect_qf_nsa_beyond_ptw;
        std::vector<bool> m_vect_qf_overflow;
        std::vector<bool> m_vect_qf_underflow;
        std::vector<uint32_t> m_vect_nsamples_over_threshold;
        std::vector<uint32_t> m_vect_course_time;
        std::vector<uint32_t> m_vect_fine_time;
        std::vector<uint32_t> m_vect_pulse_peak;
        std::vector<bool> m_vect_qf_vpeak_beyond_nsa;
        std::vector<bool> m_vect_qf_vpeak_not_found;
        std::vector<bool> m_vect_qf_bad_pedestal;
        std::vector<uint32_t> m_vect_pulse_number;
        std::vector<uint32_t> m_vect_nsamples_integral;
        std::vector<uint32_t> m_vect_nsamples_pedestal;
        std::vector<bool> m_vect_emulated;
        std::vector<uint32_t> m_vect_integral_emulated;
        std::vector<uint32_t> m_vect_pedestal_emulated;
        std::vector<uint32_t> m_vect_time_emulated;
        std::vector<uint32_t> m_vect_course_time_emulated;
        std::vector<uint32_t> m_vect_fine_time_emulated;
        std::vector<uint32_t> m_vect_pulse_peak_emulated;
        std::vector<uint32_t> m_vect_qf_emulated;
    };
}
