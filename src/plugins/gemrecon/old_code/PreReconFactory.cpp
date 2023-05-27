#include "PreReconFactory.h"

#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "rawdataparser/Df125WindowRawData.h"
#include "rawdataparser/Df125FDCPulse.h"
#include "GEMOnlineHitDecoder.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include "services/root_output/RootFile_service.h"
#include <filesystem>
#include "plugins/gemrecon/Pedestal.h"
#include "plugins/gemrecon/RawData.h"

namespace ml4fpga::gem {



//-------------------------------------
// Init
//-------------------------------------
void PreReconFactory::Init() {

    std::string plugin_name = GetPluginName();

    // Get JANA application
    auto app = GetApplication();


    // Get Log level from user parameter or default
    InitLogger(plugin_name + ":PreRecoF");


    // P A R A M E T E R S
    // Number of SRS time samples:
    app->SetDefaultParameter("daq:srs_window_raw:ntsamples", m_srs_ntsamples, "Number of SRS time samples");
    app->SetDefaultParameter( "gemrecon:min_adc", m_min_adc, "Min ADC value (For histos?)");
    app->SetDefaultParameter( "gemrecon:max_adc", m_max_adc, "Max ADC value (For histos?)");


    //  D O N E
    logger()->info("initialization done");

    // TODO Mapping NoOneKnowsWhyItWORKS

}



//------------------
// Process
//------------------
// This function is called every event
void PreReconFactory::Process(const std::shared_ptr<const JEvent> &event) {
    m_log->debug("new event");
    try {
        auto decoded_data = const_cast<DecodedData*>(event->GetSingle<DecodedData>());
        auto mapping = GemMapping::GetInstance();

        auto result = new PreReconData();
        result->num_time_samples = m_srs_ntsamples;

        auto plane_map = mapping->GetAPVIDListFromPlaneMap();
        for(const auto pair: plane_map) {
            auto plane_name = pair.first;
            auto apv_list = pair.second;

            for(auto apv_id: apv_list) {
                auto apv_data = decoded_data->apv_data[apv_id];
                for(auto timebin: apv_data.data){
                    for(auto ch_data: timebin) {
                        if (plane_name == "URWELLX") {
                            result->samples_x.push_back(ch_data);
                        }
                        if (plane_name == "URWELLY") {
                            result->samples_y.push_back(ch_data);
                        }
                    }
                }
            }
        }


        Insert(result);
    }
    catch (std::exception &exp) {
        m_log->error("Error during process");
        m_log->error("Exception what()='{}', type='{}'", exp.what(), typeid(exp).name());
    }
}


//------------------
// Finish
//------------------
void PreReconFactory::Finish() {
//    m_log->trace("PreRecoFactory finished\n");

}

}
