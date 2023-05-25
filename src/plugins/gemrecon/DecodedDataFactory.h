#pragma once

#include <JANA/JMultifactory.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JFactoryT.h>
#include <JANA/JMultifactory.h>
#include "plugins/gemrecon/old_code/GEMCluster.h"
#include <TH2F.h>
#include "plugins/gemrecon/old_code/GEMPedestal.h"
#include "plugins/gemrecon/old_code/GemConfiguration.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "GemReconDqmProcessor.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "SFclust.h"
#include "DecodedData.h"


namespace ml4fpga::gem {

    class DecodedDataFactory: public JFactoryT<DecodedData>, public spdlog::extensions::SpdlogMixin<DecodedDataFactory>  {
        public:
            DecodedDataFactory() = default;
            void Init() override;
            void Process(const std::shared_ptr<const JEvent>&) override;
            void Finish() override;
        private:

            SingleApvDecodedData DecodeApv(int apv_id, std::vector<std::vector<double>> raw_data,
                                           std::vector<double> offsets,
                                           std::vector<double> noises);

            unsigned int m_srs_ntsamples = 9;
            int m_min_adc = 50;         /// Min ADC value for histogram plotting
            int m_max_adc = 5000;       /// Max ADC value for histogram plotting
            std::string fIsHitPeakOrSumADCs = "peakADCs";
            std::string fIsCentralOrAllStripsADCs = "centralStripADCs";
    };
} // namespace ml4fpga::gem


