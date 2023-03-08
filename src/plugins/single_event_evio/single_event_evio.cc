//
// Created by Xinxin Mei, xme@jlab.org on 2/23/23.
//
#include <JANA/JApplication.h>

#include "SingleEvioEventFileSource.h"

extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

//    app->Add(new CDAQEVIOFileSource(TEST_FILEPATH, app));  // test
    app->Add(new JEventSourceGeneratorT<SingleEvioEventFileSource>);
}
}
