//
// Created by Xinxin Mei, xme@jlab.org on 2/23/23.
//
#include <JANA/JApplication.h>

#include "CDAQEVIOFileSource.h"

// put a real path for simplicity
/*
 * [hdtrdops@gluon200 ~]$ ls /gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_00
hd_rawdata_002539_000.evio  hd_rawdata_002539_003.evio  hd_rawdata_002539_006.evio
hd_rawdata_002539_001.evio  hd_rawdata_002539_004.evio  hd_rawdata_002539_007.evio
hd_rawdata_002539_002.evio  hd_rawdata_002539_005.evio
 */
#define TEST_FILEPATH "/gluonraid3/data4/rawdata/trd/DATA/hd_rawdata_002539_002.evio"


extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

//    app->Add(new CDAQEVIOFileSource(TEST_FILEPATH, app));  // test
    app->Add(new JEventSourceGeneratorT<CDAQEVIOFileSource>);
}
}
