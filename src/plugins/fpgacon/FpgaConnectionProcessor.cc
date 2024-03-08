#include "FpgaConnectionProcessor.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include <services/root_output/RootFile_service.h>

#include <TSocket.h>
#include <TMarker.h>
#include <TCanvas.h>
#include <TH2.h>
#include <TF1.h>
#include <rawdataparser/Df125WindowRawData.h>

#include "F125Cluster.h"
#include "services/dqm/DataQualityMonitorService.h"


//------------------
// Init
//------------------
void FpgaConnectionProcessor::Init() {
	std::string plugin_name = GetPluginName();

	// Get JANA application
	auto app = GetApplication();

	// Get Log level from user parameter or default
	InitLogger(plugin_name+"_proc");

	// Ask service locator a file to write histograms to
	m_dqm_service = app->GetService<DataQualityMonitorService>();

	logger()->info("This plugin name is: " + GetPluginName());
	logger()->info(JTypeInfo::demangle<FpgaConnectionProcessor>() + " initialization is done");
}


//------------------
// Process
//------------------
// This function is called every event
void FpgaConnectionProcessor::Process(const std::shared_ptr<const JEvent>&event) {

	if(event->GetEventNumber()<3) return;

	auto clusters = event->Get<ml4fpga::fpgacon::F125Cluster>();
	for(auto& cluster: clusters) {
		logger()->info(" id: {}", cluster->id);

	}


}


//------------------
// Finish
//------------------
void FpgaConnectionProcessor::Finish() {
	//    m_log->trace("FpgaConnectionProcessor finished\n");
}

