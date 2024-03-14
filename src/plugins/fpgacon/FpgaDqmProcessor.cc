#include "FpgaDqmProcessor.h"

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
#include <TROOT.h>
#include <rawdataparser/Df125WindowRawData.h>

#include "F125Cluster.h"
#include "F125ClusterContext.h"
#include "services/dqm/DataQualityMonitorService.h"
#include "FpgaHitsToTrack.h"
#include "FpgaTrackFit.h"


//------------------
// Init
//------------------
void FpgaDqmProcessor::Init() {
	std::string plugin_name = GetPluginName();

	// Get JANA application
	auto app = GetApplication();

	// Get Log level from user parameter or default
	InitLogger(plugin_name+"_proc");

	// Ask service locator a file to write histograms to
	m_dqm_service = app->GetService<DataQualityMonitorService>();

	logger()->info("This plugin name is: " + GetPluginName());
	logger()->info(JTypeInfo::demangle<FpgaDqmProcessor>() + " initialization is done");
}


//------------------
// Process
//------------------
// This function is called every event
void FpgaDqmProcessor::Process(const std::shared_ptr<const JEvent>&event) {

	// We use this m_total_event_num because when there are several files of the same accelerator-run
	// we have the same event numbers and have memory leaks with histograms having the same names
	uint64_t m_total_event_num = 0;

	if(event->GetEventNumber()<3) return;



	try {
		bool should_fill_event = m_dqm_service->ShouldProcessEvent(event->GetEventNumber());

		if(!should_fill_event) {
			return;
		}

	}
	catch (std::exception &exp) {
		m_log->error("Error during process");
		m_log->error("Exception what()='{}', type='{}'", exp.what(), typeid(exp).name());
		throw new JException(exp.what());
	}



	auto context = event->GetSingle<ml4fpga::fpgacon::F125ClusterContext>();

	auto hists_dir = m_dqm_service->GetPerEventSubDir(m_total_event_num, "fpgacon");

	auto hevt = static_cast<TH2*>(context->hevt->Clone("hevt-n"));
	hevt->SetDirectory(hists_dir);

	auto hevtc = static_cast<TH2*>(context->hevtc->Clone("hevtc-n"));
	hevtc->SetDirectory(hists_dir);
	hists_dir->cd();

	// Fill peaks on the instagram
	gROOT->SetBatch(kTRUE);
	uint64_t event_number = m_total_event_num;
	//-----------------  canvas 1 FPGA Display ----------
	std::string title = fmt::format("Clustering event {}", m_total_event_num);
	TCanvas *clust_canvas = new TCanvas("clust_canvas",title.c_str(),100,100,1000,1300);
	clust_canvas->cd();
	hevt->Draw("colz");
	int COLMAP[]={1,2,3,4,6,5};

	logger()->trace("{:>10} {:>10} {:>10} {:>10}", "id", "pos_x", "pos_z", "dedx");
	auto clusters = event->Get<ml4fpga::fpgacon::F125Cluster>();
	for(auto& cluster: clusters) {
		logger()->trace("{:>10} {:>10.2f} {:>10.2f} {:>10.2f}", cluster->id, cluster->pos_x, cluster->pos_z, cluster->dedx);

		// Put marker on the prlot
		int mstyle = cluster->size < m_cfg_min_clust_size ? 22 : 20;		// Cluster marker style
		TMarker m = TMarker(cluster->pos_z, cluster->pos_x, mstyle);

		// Cluster marker color
		int tcol=2;
		int mcolor = COLMAP[tcol-1];
		m.SetMarkerColor(mcolor);

		// Size
		m.SetMarkerSize(0.7 + cluster->dedx / 300);	// Parametrize?

		// Update everything
		m.Draw();
		gPad->Modified();
		gPad->Update();
	}
	//canvas->Modified();
	clust_canvas->Update();
	clust_canvas->Write();
	delete clust_canvas;


	// Fit canvas

	 title = fmt::format("Clustering event {}", m_total_event_num);
	TCanvas *tfit_canvas = new TCanvas("clust_canvas",title.c_str(),100,100,1000,1300);
	tfit_canvas->cd();
	hevt->Draw("colz");

	double zStart =  5.; // mm
	double zEnd   = 29.; // mm


	logger()->trace("{:>10} {:>10} {:>10} {:>10}", "id", "pos_x", "pos_z", "dedx");
	auto ht_assoc = event->Get<ml4fpga::fpgacon::FpgaHitsToTrack>();
	auto track_fits = event->Get<ml4fpga::fpgacon::FpgaTrackFit>();
	for(auto tfit: track_fits) {
		TF1 ftrk("ftrk", "[0]*x+[1]", zStart, zEnd);
		ftrk.SetParameter(0, tfit->slope);
		ftrk.SetParameter(1, tfit->intersect);
		ftrk.DrawClone("same");
	}

	if(ht_assoc.size() == clusters.size()) {
		for(int i=0; i < clusters.size(); i++) {
			auto& cluster = clusters[i];
			logger()->trace("{:>10} {:>10.2f} {:>10.2f} {:>10.2f}", cluster->id, cluster->pos_x, cluster->pos_z, cluster->dedx);

			// Put marker on the prlot
			int mstyle = cluster->size < m_cfg_min_clust_size ? 22 : 20;		// Cluster marker style
			TMarker m = TMarker(cluster->pos_z, cluster->pos_x, mstyle);

			// Cluster marker color
			int tcol=std::min(ht_assoc[i]->track_index,6);
			int mcolor = COLMAP[tcol-1];
			m.SetMarkerColor(mcolor);

			// Size
			m.SetMarkerSize(0.7 + cluster->dedx / 300);	// Parametrize?

			// Update everything
			m.Draw();
			gPad->Modified();
			gPad->Update();
		}
	}
	//canvas->Modified();
	clust_canvas->Update();
	clust_canvas->Write();
	delete clust_canvas;


	auto hits_assoc = event->Get<ml4fpga::fpgacon::FpgaHitsToTrack>();
}


//------------------
// Finish
//------------------
void FpgaDqmProcessor::Finish() {
	//    m_log->trace("FpgaConnectionProcessor finished\n");
}

