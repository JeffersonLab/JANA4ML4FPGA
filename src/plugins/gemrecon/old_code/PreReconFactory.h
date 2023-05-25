#pragma once

#include <JANA/JMultifactory.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JFactoryT.h>
#include "GEMCluster.h"
#include <TH2F.h>
#include "GEMPedestal.h"
#include "GemConfiguration.h"
#include "plugins/gemrecon/GemMapping.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "plugins/gemrecon/GemReconDqmProcessor.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "plugins/gemrecon/SFclust.h"
#include "PreReconData.h"


namespace ml4fpga::gem {




class PreReconFactory: public JFactoryT<PreReconData>, public spdlog::extensions::SpdlogMixin<PreReconFactory>  {
    public:
        PreReconFactory() = default;
        void Init() override;
        void Process(const std::shared_ptr<const JEvent>&) override;
        void Finish() override;
    private:

        unsigned int m_srs_ntsamples = 9;
        int m_min_adc = 50;         /// Min ADC value for histogram plotting
        int m_max_adc = 5000;       /// Max ADC value for histogram plotting
        std::string fIsHitPeakOrSumADCs = "peakADCs";
        std::string fIsCentralOrAllStripsADCs = "centralStripADCs";
};



} // namespace ml4fpga::gem


