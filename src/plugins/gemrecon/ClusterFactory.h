#pragma once

#include <JANA/JMultifactory.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JFactoryT.h>
#include "plugins/gemrecon/old_code/GEMCluster.h"
#include <TH2F.h>
#include "plugins/gemrecon/old_code/GEMPedestal.h"
#include "plugins/gemrecon/old_code/GemConfiguration.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "GemReconDqmProcessor.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "SFclust.h"


namespace ml4fpga::gem {

    class GemApvDecodingResult {
    public:
        std::vector<float> PedestalOffsets;
        std::vector<float> PedestalNoises;
        std::vector<std::vector<int>> RawData;
        std::vector<double> CommonModeOffsets;
        std::vector<double> RawDataAverage;
    };

    class ClusterFactory: public JFactoryT<SFclust>, public spdlog::extensions::SpdlogMixin<ClusterFactory>  {
    public:
        explicit ClusterFactory();
        void Init() override;
        void Process(const std::shared_ptr<const JEvent>&) override;
        void Finish() override;
    private:


        /// Directory to store histograms to
        TDirectory *m_dir_main{};
        TH1F* m_histo_1d;
        TH2F* m_trd_integral_h2d;
        TDirectory *m_dir_event_hists;
        GemMapping *fMapping;               // GEM mapping

        TH1F **f1DSingleEventHist, **fADCHist, **fHitHist, **fClusterHist, **fClusterInfoHist, **fChargeRatioHist;
        TH2F **fTimeBinPosHist, **fADCTimeBinPosHist, **f2DPlotsHist, **f2DSingleEventHist, **fChargeSharingHist;
        void InitHistForZeroSup();

        void TraceDumpSrsData(std::vector<const DGEMSRSWindowRawData *> srs_data, size_t print_rows=10);
        void TraceDumpMapping(std::vector<const DGEMSRSWindowRawData *> srs_data, size_t print_rows=10);
        void FillTrdHistogram(uint64_t event_number, TDirectory *hists_dir, std::vector<const DGEMSRSWindowRawData *> srs_data, int max_x = 250, int max_y = 300);
        GemApvDecodingResult DecodeApv(int apv_id, std::vector<std::vector<int>> raw_data);

        unsigned int m_srs_ntsamples = 9;
        int m_min_adc = 50;         /// Min ADC value for histogram plotting
        int m_max_adc = 5000;       /// Max ADC value for histogram plotting
        int fZeroSupCut = 10;
        int m_max_cluster_size = 20;
        int m_min_cluster_size = 2;
        int fStartTimeSample = 0;
        int fComModeCut = 20;
        int fMaxClusterMult = 5;
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


