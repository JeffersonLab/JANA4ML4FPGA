// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "ClusterFactory.h"
#include "PedestalFactory.h"
#include "RawDataFactory.h"
#include "GemReconDqmProcessor.h"
#include "DecodedDataFactory.h"
#include <JANA/JFactoryGenerator.h>


extern "C" {
    void InitPlugin(JApplication *app) {
        // Initializes this plugin
        InitJANAPlugin(app);

        // Adds our processor to JANA2 to execute
        app->Add(new ml4fpga::gem::GemReconDqmProcessor(app));
        app->Add(new ml4fpga::gem::ClusterFactoryGenerator(app));
        app->Add(new JFactoryGeneratorT<ml4fpga::gem::PedestalFactory>());
        app->Add(new JFactoryGeneratorT<ml4fpga::gem::RawDataFactory>());
        app->Add(new JFactoryGeneratorT<ml4fpga::gem::DecodedDataFactory>());
    }
}
    
