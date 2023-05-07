#pragma once

#include <JANA/JMultifactory.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JFactoryT.h>
#include "GEMCluster.h"
#include <TH2F.h>
#include "GEMPedestal.h"
#include "GemConfiguration.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "GEMReconTestProcessor.h"
#include <extensions/spdlog/SpdlogExtensions.h>
#include "SFclust.h"


namespace ml4fpga::gem {


class ClusterFactory: public JFactoryT<SFclust>, public spdlog::extensions::SpdlogMixin<ClusterFactory>  {
    public:
        explicit ClusterFactory(JApplication* app);
        void Init() override;
        void Process(const std::shared_ptr<const JEvent>&) override;
        void Finish() override;
    private:
        JApplication* m_app;

        /// Directory to store histograms to
        TDirectory *m_dir_main{};
        TH1F* m_histo_1d;
        TH2F* m_trd_integral_h2d;
        TDirectory *m_dir_event_hists;
        GEMPedestal *fPedestal;
        GemMapping *fMapping;
        GemConfiguration fConfig;
        TH1F **f1DSingleEventHist, **fADCHist, **fHitHist, **fClusterHist, **fClusterInfoHist, **fChargeRatioHist;
        TH2F **fTimeBinPosHist, **fADCTimeBinPosHist, **f2DPlotsHist, **f2DSingleEventHist, **fChargeSharingHist;
        void InitHistForZeroSup();

        void TraceDumpSrsData(std::vector<const DGEMSRSWindowRawData *> srs_data, size_t print_rows=10);
        void TraceDumpMapping(std::vector<const DGEMSRSWindowRawData *> srs_data, size_t print_rows=10);
        void FillTrdHistogram(uint64_t event_number, TDirectory *hists_dir, std::vector<const DGEMSRSWindowRawData *> srs_data, int max_x = 250, int max_y = 300);
        GemApvDecodingResult DecodeApv(int apv_id, std::vector<std::vector<int>> raw_data);
    };



/// JFactoryGeneratorT works for both JFactories and JMultifactories

    class ClusterFactoryGenerator : public JFactoryGenerator {

        std::string m_tag;
        bool m_tag_specified;

    public:

        explicit ClusterFactoryGenerator(JApplication *app) :
                m_app(app),
                m_tag_specified(false) {};

        explicit ClusterFactoryGenerator(JApplication *app, std::string tag) :

            m_app(app),
            m_tag(std::move(tag)),
            m_tag_specified(true) {};

        void GenerateFactories(JFactorySet *factory_set) override {
            auto* factory = new ClusterFactory(m_app);

            if (m_tag_specified) {
                // If user specified a tag via the generator (even the empty tag!), use that.
                // Otherwise, use whatever tag the factory may have set for itself.
                factory->SetTag(m_tag);
            }
            factory->SetFactoryName(JTypeInfo::demangle<ClusterFactory>());
            factory->SetPluginName(GetPluginName());
            factory_set->Add(factory);
        }

        JApplication *m_app;
    };


} // namespace ml4fpga::gem


