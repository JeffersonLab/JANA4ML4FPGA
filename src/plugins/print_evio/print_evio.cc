// Copyright 2023, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "PrintEvioProcessor.h"


extern "C" {
    void InitPlugin(JApplication *app) {

        // Initializes this plugin
        InitJANAPlugin(app);

        // Adds our processor to JANA2 to execute
        app->Add(new PrintEvioProcessor(app));
    }
}
    
