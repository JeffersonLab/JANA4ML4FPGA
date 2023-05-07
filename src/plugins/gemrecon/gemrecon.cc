// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "GEMReconTestProcessor.h"
#include "ClusterFactory.h"

#include <JANA/JFactoryGenerator.h>


extern "C" {
    void InitPlugin(JApplication *app) {
        // Initializes this plugin
        InitJANAPlugin(app);

        // Adds our processor to JANA2 to execute
        // app->Add(new GEMReconTestProcessor(app));
        app->Add(new ml4fpga::gem::ClusterFactoryGenerator(app));
    }
}
    
