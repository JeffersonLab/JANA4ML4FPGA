#pragma once

#include <JANA/JMultifactory.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JFactoryT.h>
#include <TH2F.h>
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "GemReconDqmProcessor.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "SFclust.h"
#include "ApvDecodingResult.h"
#include "Constants.h"
#include "extmath/PeakFinder.h"
#include "GemMapping.h"
#include "PlanePeak.h"

namespace ml4fpga::gem {



    class ClusterFactory: public JFactoryT<SFclust>, public spdlog::extensions::SpdlogMixin<ClusterFactory>  {
    public:
        explicit ClusterFactory();
        void Init() override;
        void Process(const std::shared_ptr<const JEvent>&) override;
        void Finish() override;
    private:


        /// Directory to store histograms to


        GemMapping *fMapping;               // GEM mapping


        void InitHistForZeroSup();

        void TraceDumpSrsData(std::vector<const DGEMSRSWindowRawData *> srs_data, size_t print_rows=10);
        void TraceDumpMapping(std::vector<const DGEMSRSWindowRawData *> srs_data, size_t print_rows=10);
        void FillTrdHistogram(uint64_t event_number, TDirectory *hists_dir, std::vector<const DGEMSRSWindowRawData *> srs_data, int max_x = 250, int max_y = 300);
        GemApvDecodingResult DecodeApv(int apv_id, std::vector<std::vector<int>> raw_data);

        unsigned int m_srs_ntsamples = 9;
        int m_min_adc = 50;         /// Min ADC value for histogram plotting
        int m_max_adc = 5000;       /// Max ADC value for histogram plotting
        std::string fIsHitPeakOrSumADCs = "peakADCs";
        std::string fIsCentralOrAllStripsADCs = "centralStripADCs";
        std::string m_plane_name_x = "URWELLX";
        std::string m_plane_name_y = "URWELLY";
        double m_plane_size_x;
        double m_plane_size_y;
        int m_plane_apv_num_x;
        int m_plane_apv_num_y;
    };
} // namespace ml4fpga::gem


