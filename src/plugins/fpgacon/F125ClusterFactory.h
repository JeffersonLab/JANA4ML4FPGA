//
// Created by Dmitry Romanov on 3/8/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <TH2.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/jana/CozyFactory.h>
#include <JANA/JFactoryT.h>
#include <rawdataparser/Df125WindowRawData.h>


#include "F125Cluster.h"
#include "F125ClusterContext.h"

namespace ml4fpga::fpgacon {

    class F125ClusterFactory : public CozyFactory<EmptyConfig>
    {
    public:
        F125ClusterFactory() = default;
        void CozyInit() override;
        void CozyBeginRun(uint64_t run_number) override;
        void CozyProcess(uint64_t run_number, uint64_t event_number) override;
        void CozyFinish() override;
    private:
        Input<Df125WindowRawData> m_f125_input {this};
        Output<F125Cluster> m_cluster_output {this};
        Output<F125ClusterContext> m_cluster_context_output {this};

        Parameter<bool> m_cfg_use_clustering {this, "use_clustering", true, "Maximum num vertices that can be found"};
        TH2F * hevt;
        TH2F * hevtc;
        TH2F * hevti;
        TH2F * hevtf;
        // ParameterRef<bool> m_reassignTracksAfterFirstFit {this, "reassignTracksAfterFirstFit",
        //                        config().m_reassignTracksAfterFirstFit,
        //                        "Whether or not to reassign tracks after first fit"};
    };

}

