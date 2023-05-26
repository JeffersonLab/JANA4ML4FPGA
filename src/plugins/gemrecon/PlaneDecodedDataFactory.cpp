#include "PlaneDecodedDataFactory.h"

#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "rawdataparser/Df125WindowRawData.h"
#include "rawdataparser/Df125FDCPulse.h"
#include "plugins/gemrecon/old_code/GEMOnlineHitDecoder.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include "services/root_output/RootFile_service.h"
#include <filesystem>
#include "Pedestal.h"
#include "RawData.h"

namespace ml4fpga::gem {



//-------------------------------------
// Init
//-------------------------------------
void PlaneDecodedDataFactory::Init() {

    std::string plugin_name = GetPluginName();

    // Get JANA application
    auto app = GetApplication();

    // Get Log level from user parameter or default
    InitLogger(plugin_name + ":" + JTypeInfo::demangle<GemReconDqmProcessor>());

    // P A R A M E T E R S
    // Number of SRS time samples:
    app->SetDefaultParameter("daq:srs_window_raw:ntsamples", m_srs_ntsamples, "Number of SRS time samples");
    app->SetDefaultParameter( plugin_name + ":min_adc", m_min_adc, "Min ADC value (For histos?)");
    app->SetDefaultParameter( plugin_name + ":max_adc", m_max_adc, "Max ADC value (For histos?)");

    //  D O N E
    logger()->info("This plugin name is: " + GetPluginName());
    logger()->info("DecodedDataFactory initialization is done");

    m_mapping = GemMapping::GetInstance();
}



//------------------
// Process
//------------------
// This function is called every event
void PlaneDecodedDataFactory::Process(const std::shared_ptr<const JEvent> &event) {
    m_log->debug("new event");
    try {
        auto apv_decoded_data = const_cast<ApvDecodedData*>(event->GetSingle<ApvDecodedData>());

        auto result = new PlaneDecodedData();

        // Now go per plane
        auto plane_map = m_mapping->GetAPVIDListFromPlaneMap();
        for (auto pair: plane_map) {
            auto name = pair.first;
            auto det_name = m_mapping->GetDetectorFromPlane(name);
            auto apv_list = pair.second;
            m_log->info("  Plane: {:<10} from detector {:<10} has {} APVs:", name, det_name, apv_list.size());

            // Make a merge for all APVs
            AdcDecodedData merged_data;
            size_t i=0;
            for (auto apv_id: apv_list) {
                if(i==0) {
                    merged_data = apv_decoded_data->apv_data[apv_id];
                } else {
                    merged_data = mergeAdcDecodedData(merged_data, apv_decoded_data->apv_data[apv_id]);
                }
                i++;
            }
            result->plane_data[name] = merged_data;
        }


    }
    catch (std::exception &exp) {
        m_log->error("Error during process");
        m_log->error("Exception what()='{}', type='{}'", exp.what(), typeid(exp).name());
    }
}


//------------------
// Finish
//------------------
void PlaneDecodedDataFactory::Finish() {
//    m_log->trace("DecodedDataFactory finished\n");

}

}
