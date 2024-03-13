//
// Created by romanov on 5/8/2023.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include "RawData.h"
#include "RawDataFactory.h"
#include <JANA/JFactoryT.h>
#include "extensions/spdlog/SpdlogMixin.h"


namespace ml4fpga::gem {

    class RawDataFactory :
            public JFactoryT<RawData>,
            public spdlog::extensions::SpdlogMixin<RawDataFactory>  {
    public:
        RawDataFactory()=default;
        void Init() override;
        void Process(const std::shared_ptr<const JEvent>&) override;
        void Finish() override;


        std::map<int, std::string> m_apv_id_names_map;
    };

} // ml4fpga
