#include "PlaneDecodedDataFactory.h"

#include "rawdataparser/Df125WindowRawData.h"

#include <JANA/JEvent.h>

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
    InitLogger(plugin_name + ":PlaneDecode");

    // P A R A M E T E R S
    // Number of SRS time samples:
    app->SetDefaultParameter("daq:srs_window_raw:ntsamples", m_srs_ntsamples, "Number of SRS time samples");
    app->SetDefaultParameter(plugin_name + ":min_adc", m_min_adc, "Min ADC value (For hists?)");
    app->SetDefaultParameter(plugin_name + ":max_adc", m_max_adc, "Max ADC value (For hists?)");

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
void PlaneDecodedDataFactory::Finish() {
//    m_log->trace("DecodedDataFactory finished\n");

}

}
