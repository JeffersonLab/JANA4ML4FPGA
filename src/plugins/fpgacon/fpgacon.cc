// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include "FpgaDqmProcessor.h"
#include "F125ClusterFactory.h"
#include "FpgaExchangeFactory.h"
#include <extensions/jana/CozyFactoryGeneratorT.h>

extern "C" {
    void InitPlugin(JApplication *app) {

        // Initializes this plugin
        InitJANAPlugin(app);

        // Adds our processor to JANA2 to execute
        app->Add(new FpgaDqmProcessor(app));


        app->Add(new CozyFactoryGeneratorT<ml4fpga::fpgacon::F125ClusterFactory>("clust", app));

        //app->Add(new JFactoryGeneratorT<ml4fpga::fpgacon::FpgaResultFactory>());
    }
}
    
