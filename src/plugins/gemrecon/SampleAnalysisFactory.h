//
// Created by romanov on 3/7/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <extensions/jana/CozyFactory.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extmath/RollingStatistics.h>
#include <JANA/JFactoryT.h>
#include <rawdataparser/DGEMSRSWindowRawData.h>

#include "GemMappingService.h"
#include "SampleAnalysisData.h"
#include "SampleData.h"


namespace ml4fpga::gem {

    class SampleAnalysisFactory : public CozyFactory<EmptyConfig>{
    public:
        SampleAnalysisFactory()=default;

        void CozyInit() override;
        void CozyProcess(uint64_t run_number, uint64_t event_number) override;


        Input<DGEMSRSWindowRawData> m_srs_raw_data {this};
        Output<SampleData> m_output {this};
        Service<GemMappingService> m_mapping_service {this};
        int m_req_ntsamples;                        // Number of time samples set in flag
        GemMapping* m_mapping;
        std::map<int, std::string> m_apv_id_names_map;
        //Output<FpgaTrackFit> m_output_trak_fit {this};

    private:

        size_t m_cfg_fpga_max_hits = 50;   // Max hits to be sent to FPGA
        Parameter<std::string> m_cfg_host {this, "host", "localhost", "Host address to connect to"};
        Parameter<size_t> m_rolling_len {this, "roll_stat_len", 500, "Rolling average and sigma length for each sample"};

        std::map<uint64_t, extmath::RollingStatistics<double>> m_sample_stats;


};

} // gem
// ml4fpga
