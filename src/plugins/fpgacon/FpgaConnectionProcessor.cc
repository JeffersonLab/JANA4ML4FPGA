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

#define USE_TRK
#define MAX_PRINT 1
#define USE_FPGA 1
#define USE_TCP 1
#define USE_FIT 1


// -- GEMTRD mapping --
int GetGEMChan(int ch, int slot) {
	int cardNumber = ch / 24;
	int cardChannel = ch - cardNumber * 24;
	int invCardChannel = 23 - cardChannel;
	if (slot < 6 || (slot == 6 && ch < 24)) {
		return invCardChannel + cardNumber * 24 + (slot - 3) * 72.;
	}
	return -1;
}


//------------------
// OccupancyAnalysis (Constructor)
//------------------
FpgaConnectionProcessor::FpgaConnectionProcessor(JApplication* app) : JEventProcessor(app) {
}

//------------------
// Init
//------------------
void FpgaConnectionProcessor::Init() {
	std::string plugin_name = GetPluginName();

	// Get JANA application
	auto app = GetApplication();

	// Ask service locator a file to write histograms to
	auto root_file_service = app->GetService<RootFile_service>();

	// Get TDirectory for histograms root file
	auto globalRootLock = app->GetService<JGlobalRootLock>();
	globalRootLock->acquire_write_lock();
	auto file = root_file_service->GetHistFile();
	globalRootLock->release_lock();

	// Create a directory for this plugin. And subdirectories for series of histograms
	m_dir_main = file->mkdir(plugin_name.c_str());

	// Get Log level from user parameter or default
	InitLogger(plugin_name);

	logger()->info("This plugin name is: " + GetPluginName());
	logger()->info("FpgaConnectionProcessor initialization is done");
}


//------------------
// Process
//------------------
// This function is called every event
void FpgaConnectionProcessor::Process(const std::shared_ptr<const JEvent>&event) {
	m_log->trace("FpgaConnectionProcessor event");

	std::vector<const Df125WindowRawData *> f125_values = event->Get<Df125WindowRawData>();


	//+++++++++++++++++++++++++++++ TCP +++++++++++++++++++

#if  (USE_FPGA==1)  //--  tcp send

	// Open connection to server
	TSocket* sock = new TSocket("localhost", 20250);

	// Wait till we get the start message
	char str[32];
	//sock->Recv(str, 32);

	// server tells us who we are
	//int idx = !strcmp(str, "go 0") ? 0 : 1;

	//printf("recv: %d %s \n",idx,str);
	Float_t messlen = 0;
	Float_t cmesslen = 0;

#define BUFSIZE 128000
	static unsigned int BUFFER[BUFSIZE];
	float* FBUFFER = (float *)BUFFER;
	int LENEVENT;
	int nmod = 1, hdr, itrg = 0;
	unsigned int EVT_Type_BOR = (0x0 & 0x3) << 22; //--  BOR=0x0
	unsigned int EVT_Type_EOR = (0x1 & 0x3) << 22; //--  EOR=0x1
	unsigned int EVT_Type_DATA = (0x2 & 0x3) << 22; //--  DATA=0x2
	static int HEADER[10];
	int modID = 4;
#endif
	//++++++++++++++++++++++++++++++++++++++++++++++++++++


	//---  clustering histograms  ---

	int nx0 = 100;
	int ny0 = 250;

	auto hevt = new TH2F("hevt", " Event display; z pos,mm; y pos,mm ", nx0, 0., +30., ny0, -50., +50.);
	hevt->SetStats(false);
	hevt->SetMaximum(10.);
	// HistList->Add(hevt);		// TODO (commented)

	auto hevtc = new TH2F("hevtc", " Clustering ; FADC bins; GEM strips", nx0, -0.5, nx0 - 0.5, ny0, -0.5, ny0 - 0.5);
	hevtc->SetStats(false);
	hevtc->SetMinimum(0.07);
	hevtc->SetMaximum(40.);
	// HistList->Add(hevtc);

	auto hevti = new TH2F("hevti", " ML-FPGA response; z pos,mm; y pos,mm ", nx0, 0., +30., ny0, -50., +50.);
	hevti->SetStats(false);
	hevti->SetMaximum(10.);
	//HistList->Add(hevti);		// TODO (commented)

	auto hevtf = new TH2F("hevtf", " Clusters for FPGA ; z pos,mm; y pos,mm ", nx0, 0., +30., ny0, -50., +50.);
	hevtf->SetStats(false);
	hevtf->SetMaximum(10.);
	// HistList->Add(hevtf);	// TODO (commented)

	//-------------------------------------------------------------------------
	//-----------------  canvas 1 FPGA Display ----------
	char c2Title[256];
	sprintf(c2Title, "Event_Display_Run=%lu", event->GetEventNumber());
	auto* c2 = new TCanvas("FPGA", c2Title, 100, 100, 1000, 1300);
	c2->Divide(1, 3);
	c2->cd(1);


	//==================================================================================================
	//                    Process Fa125  RAW data
	//==================================================================================================

#define USE_125_RAW

	if (event->GetEventNumber() < MAX_PRINT) {
		printf("------------------ Fadc125  wraw_count = %llu ---------\n", f125_values.size());
	}

	hevt->Reset();
	hevtc->Reset();
	hevti->Reset();
	hevtf->Reset();
	bool electron = false;

	int Ch_out;


	for (size_t i = 0; i < f125_values.size(); i++) {
		// --- fadc125 channels loop

		auto f125_data = f125_values[i];

		//  if (jentry<MAX_PRINT) printf("F125:RAW: i=%lld  sl=%d, ch=%d, idx=%d, cnt=%d \n"
		//			   ,i,f125_wraw_slot->at(i),f125_wraw_channel->at(i)
		//			   ,f125_wraw_samples_index->at(i),f125_wraw_samples_count->at(i));
		int fADCSlot = f125_data->slot;
		int fADCChan = f125_data->channel;
		int gemChan = GetGEMChan(fADCChan, fADCSlot);


		int amax = 0;
		int tmax = 0;
		if (gemChan < 0) continue;
		double DEDX_THR = 120;
		int TimeWindowStart = 95;
		const int NCAL = 7;

		for (int si = 0; si < f125_data->samples.size(); si++) {
			//printf("f125Loop:: %d fadc_window=%d\n",si,fadc_window);
			int time = si;
			int adc = f125_data->samples[si]; // printf(" sample=%d adc=%d \n",si,adc);
			if (adc > 4090) printf("!!!!!!!!!!!!!!!!!!!!!! ADC 125 overflow: %d \n", adc);
			if (adc > amax) {
				amax = adc;
				tmax = si;
			}
			// if (fADCChan<NCAL) {
			// 	//hCal_adc[fadc_chan]->Fill(amax);
			// 	//CalSum+=amax;
			// } else { // Cherenkov
			// 	if (fadc_chan==13) { hCher_u_adc->Fill(amax);   hCher_u_time->Fill(tmax); Ch_u=amax; }
			// 	if (fadc_chan==14) { hCher_din_adc->Fill(amax);  hCher_din_time->Fill(tmax); Ch_in=amax; }
			//
			// }


			if (adc > DEDX_THR) {
				// double adc_fill=adc;
				// if (electron_ch)  {
				//   f125_el_raw->Fill(time,gemChan,adc);
				// }else  {
				//   f125_pi_raw->Fill(time,gemChan,adc);
				// }
				//
				// time-=TimeWindowStart;
				// if ( 0 > time || time > 100 ) continue; // --- drop early and late hits ---
				//
				// //hevtc->Fill(time-100,gemChan,adc/100.);
				// hevtc->SetBinContent(100-time,gemChan,adc/100.);
				//
				// double xp = gemChan/250.*100-50.;
				// double zp = (time)/(100.)*30;
				// double ap = adc/100.;
				// //hevt->Fill(zp,xp,ap);
				// hevt->SetBinContent(100-time,gemChan,adc/100.);
			}
		} // --  end of samples loop
		if (fADCChan == 15) {
			if (amax > 300) electron = true;
			// hCher_dout_adc->Fill(amax);
			// hCher_dout_time->Fill(tmax);
			Ch_out = amax;
		}
	} // -- end of fadc125 channels loop


	//    if (!(jentry%NPRT)) {
#if (USE_FPGA==0)
    int nx = f125_el_raw->GetNbinsX();
    int ny = f125_el_raw->GetNbinsY();
    double pedestal=100.;
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("+++                           RAW TRD DATA                                                         +++\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    for (int ii=0; ii<nx; ii++) {
      for (int jj=0; jj<ny; jj++) {
	if (electron_ch) {
	  double cc = f125_el_raw->GetBinContent(ii, jj);
	  //printf("EL: %d %d cc=%f \n",ii,jj,cc);
	  //if (cc == 0.) f125_el_raw->SetBinContent(ii,jj,pedestal);
	} else  {
	  double cc = f125_pi_raw->GetBinContent(ii, jj);
	  //printf("PI: %d %d cc=%f \n",ii,jj,cc);
	  //if (cc == 0.) f125_pi_raw->SetBinContent(ii,jj,pedestal);
	}
      }
    }
#endif
	//=================================================================================================================
#if (USE_FPGA>0)
	// -------------------------------   hist dist clustering         ------------------------
#define MAX_CLUST 500
	float clust_Xpos[MAX_CLUST];
	float clust_Ypos[MAX_CLUST];
	float clust_Zpos[MAX_CLUST];
	float clust_dEdx[MAX_CLUST];
	float clust_Size[MAX_CLUST];
	float clust_Width[MAX_CLUST][3]; // y1, y2, dy ; strips
	float clust_Length[MAX_CLUST][3]; // x1, x2, dx ; time

	float hits_Xpos[500];
	float hits_Ypos[500];
	float hits_Zpos[500];
	float hits_dEdx[500];

	for (int k = 0; k < MAX_CLUST; k++) {
		clust_Xpos[k] = 0;
		clust_Ypos[k] = 0;
		clust_Zpos[k] = 0;
		clust_dEdx[k] = 0;
		clust_Size[k] = 0;
		clust_Width[k][0] = 999999;
		clust_Width[k][1] = -999999;
		clust_Width[k][2] = 0;
		clust_Length[k][0] = 999999;
		clust_Length[k][1] = -999999;
		clust_Length[k][2] = 0;
	}
	float CL_DIST = 2.7; // mm
	int nclust = 0;

	//hevti->Reset();
	//for (int i=0; i<EVENT_SIZE; i++) {

	TH2F* hp = hevt; // -- hevt and hevtc should be same bin size
	TH2F* hpc = hevtc;

	int nx = hp->GetNbinsX();
	int ny = hp->GetNbinsY();
	double xmi = hp->GetXaxis()->GetBinLowEdge(1);
	double xma = hp->GetXaxis()->GetBinUpEdge(nx);
	double ymi = hp->GetYaxis()->GetBinLowEdge(1);
	double yma = hp->GetYaxis()->GetBinUpEdge(ny);
	double binx = (xma - xmi) / nx;
	double biny = (yma - ymi) / ny;
	printf("nx=%d,ny=%d,xmi=%f,xma=%f,ymi=%f,yma=%f\n", nx, ny, xmi, xma, ymi, yma);

	double THR2 = 1.2;
	for (int ix = 0; ix < nx; ix++) {
		//-------------------- clustering loop ------------------------------------
		for (int iy = 0; iy < ny; iy++) {
			double c1 = hpc->GetBinContent(ix, iy); // hpc->SetBinContent(ix,iy,5.);         // energy
			double x1 = double(ix) / double(nx) * (xma - xmi) + xmi - binx / 2.; // drift time
			double y1 = double(iy) / double(ny) * (yma - ymi) + ymi - biny / 2.; // X strip

			if (c1 < THR2) continue;
			if (nclust == 0) {
				clust_Xpos[nclust] = y1;
				clust_Ypos[nclust] = 0;
				clust_Zpos[nclust] = x1;
				clust_dEdx[nclust] = c1;
				clust_Size[nclust] = 1;
				clust_Width[nclust][0] = y1;
				clust_Width[nclust][1] = y1;
				clust_Width[nclust][2] = 0;
				clust_Length[nclust][0] = x1;
				clust_Length[nclust][1] = x1;
				clust_Length[nclust][2] = 0;
				nclust++;
				continue;
			}

			int added = 0;
			for (int k = 0; k < nclust; k++) {
				double dist = sqrt(pow((y1 - clust_Xpos[k]), 2.) + pow((x1 - clust_Zpos[k]), 2.));
				//--- dist hit to clusters
				if (dist < CL_DIST) {
					clust_Xpos[k] = (y1 * c1 + clust_Xpos[k] * clust_dEdx[k]) / (c1 + clust_dEdx[k]); //--  new X pos
					clust_Zpos[k] = (x1 * c1 + clust_Zpos[k] * clust_dEdx[k]) / (c1 + clust_dEdx[k]); //--  new Z pos
					clust_dEdx[k] = c1 + clust_dEdx[k]; // new dEdx
					clust_Size[k] = 1 + clust_Size[k]; // clust size in pixels
					//if (k==9) printf("L:1: k=%d y1=%f min=%f max=%f \n",k,y1, clust_Width[k][0], clust_Width[k][1] );
					if (y1 < clust_Width[k][0]) clust_Width[k][0] = y1;
					if (y1 > clust_Width[k][1]) clust_Width[k][1] = y1;
					clust_Width[k][2] = clust_Width[k][1] - clust_Width[k][0];
					if (x1 < clust_Length[k][0]) clust_Length[k][0] = x1;
					if (x1 > clust_Length[k][1]) clust_Length[k][1] = x1;
					clust_Length[k][2] = clust_Length[k][1] - clust_Length[k][0];
					//if (k==9) printf("L:2: k=%d y1=%f min=%f max=%f \n",k,y1, clust_Width[k][0], clust_Width[k][1] );
					// Var(X_n) = Var(X_{n-1}) + \frac{(X_n - \bar{X_{n-1}})^2}{n}
					hpc->SetBinContent(ix, iy, k + 1.);
					added = 1;
					break;
				}
			}
			if (added == 0) {
				if (nclust + 1 >= MAX_CLUST) continue;
				clust_Xpos[nclust] = y1;
				clust_Ypos[nclust] = 0;
				clust_Zpos[nclust] = x1;
				clust_dEdx[nclust] = c1;
				clust_Size[nclust] = 1;
				clust_Width[nclust][0] = y1;
				clust_Width[nclust][1] = y1;
				clust_Width[nclust][2] = 0;
				clust_Length[nclust][0] = x1;
				clust_Length[nclust][1] = x1;
				clust_Length[nclust][2] = 0;
				nclust++;
			}
		}
	} //----------------------------------- end  clustering loop -----------------------------------------------

	int MinClustSize = 10;
	double MinClustWidth = 0.001;
	double MinClustLength = 0.01;
	double zStart = 5.; // mm
	double zEnd = 29.; // mm
	//int ihit=0;
	int ii = 0;
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("                Xpos   Ypos   Zpos       E    Width  Length   Size \n");
	//       0 Clust( 1):   43.8    0.0    3.9      5.3    0.0    0.0      4.0
	for (int k = 0; k < nclust; k++) {
		//hevt->Fill(clust_Zpos[k],clust_Xpos[k],clust_dEdx[k]);
		//hevti->Fill(hits_Zpos[k],hits_Xpos[k],dEdx[k]);
		//printf("%d Clust: X,Y,Z,E = %f %f %f %f \n",k,hits_Xpos[k],hits_Ypos[k],hits_Zpos[k],hits_dEdx[k]);
		printf("%2d Clust(%2d): %6.1f %6.1f %6.1f %8.1f %6.2f %6.2f %8.1f  ", k, k + 1, clust_Xpos[k], clust_Ypos[k],
		       clust_Zpos[k], clust_dEdx[k], clust_Width[k][2], clust_Length[k][2], clust_Size[k]);
		//printf("                                  %6.1f %6.1f %6.1f %6.1f \n",clust_Width[k][0],clust_Width[k][1], clust_Length[k][0], clust_Length[k][1]);

		//-------------  Cluster Filter -----------------

		if (clust_Size[k] >= MinClustSize && zStart < clust_Zpos[k] && clust_Zpos[k] < zEnd && clust_Width[k][2] >
		    MinClustWidth) {
			hits_Xpos[ii] = clust_Xpos[k];
			hits_Ypos[ii] = clust_Ypos[k];
			hits_Zpos[ii] = clust_Zpos[k];
			hits_dEdx[ii] = clust_dEdx[k];
			ii++;
			printf("\n");
		}
		else {
			printf(" <--- skip \n");
		}
	}
	int nhits = ii;
	// -----------------------         end hist dist clustering          ----------------------------------------

	//=================================== Draw HITS and CLUST  ============================================

	char hevtTitle[80];
	sprintf(hevtTitle, " Display Event: %lld   z pos,mm; y pos,mm ", event->GetEventNumber());
	hevt->SetTitle(hevtTitle);
	printf("hits_SIZE=%d  Clust size = %d \n", nhits, nclust);
	if (event->GetEventNumber() < 1000) {
		c2->cd(1);
		hevt->Draw("colz");
		c2->cd(2);
		hevtf->Draw("text");
		c2->cd(3);
		hevti->Draw("colz");
		//c0->cd(10);   hevti->Draw("colz");
		c2->Modified();
		c2->Update();
		c2->cd(1);
		gPad->Modified();
		gPad->Update();
		int COLMAP[] = {1, 2, 3, 4, 6, 5};
		int pmt, pmt0 = 20; // PM type
		for (int i = 0; i < nclust; i++) {
			//printf("i=%d trk=%d |  %8.2f,%8.2f\n",i, tracks[i], Xcl[i], Zcl[i]);
			TMarker m = TMarker(clust_Zpos[i], clust_Xpos[i], pmt);
			int tcol = 2; //min(tracks[i],6);
			if (clust_Size[i] < MinClustSize) pmt = 22;
			else pmt = pmt0;
			int mcol = COLMAP[tcol - 1];
			m.SetMarkerColor(mcol);
			m.SetMarkerStyle(pmt);
			//m.SetMarkerSize(0.5+clust_Size[i]/100);
			m.SetMarkerSize(0.7 + clust_dEdx[i] / 300);
			m.DrawClone();
			gPad->Modified();
			gPad->Update();
		}
		c2->Modified();
		c2->Update();
	}

	//----------------------------------------------------------
	//---                 Send to FPGA                      ----
	//----------------------------------------------------------

#if (USE_TCP==1)
	//-----------------  send DATA  ----------------
	//printf(" send DATA  \n");
	int DC_NROC = 4;

	int LEN_HITS = nhits; //-- floats X,Y,Z,E
	if (LEN_HITS > 50) LEN_HITS = 50; //--- max hits to fpga
	int k = 4; //-- start data 4 words header;
	printf("-- BUFFER:: \n");
	for (int n = 0; n < LEN_HITS; n++) {
		FBUFFER[k++] = hits_Xpos[n];
		printf("%2d,0x%08x,f:%f ", (k - 1), BUFFER[k - 1], FBUFFER[k - 1]);
		//FBUFFER[k++]= hits_Ypos[n];
		//printf("%2d,0x%08x,f:%f ",(k-1),BUFFER[k-1],FBUFFER[k-1]);
		FBUFFER[k++] = hits_Zpos[n];
		printf("%2d,0x%08x,f:%f ", (k - 1), BUFFER[k - 1], FBUFFER[k - 1]);
		//FBUFFER[k++]= hits_dEdx[n];
		//printf("%2d,0x%08x,f:%f ",(k-1),BUFFER[k-1],FBUFFER[k-1]);
		printf("=====>  Clust data: X,Y,Z,E=%f %f %f %f \n", hits_Xpos[n], hits_Ypos[n], hits_Zpos[n], hits_dEdx[n]);
	}

	printf("Filled bufer size=%d , data = %d hits =%d \n", k, k - 4, (k - 4) / 2);

	int PAD = k % 2;

	itrg++;
	for (int i = 0; i < nmod; i++) {
		LENEVENT = k + PAD;

		hdr = (i & 0xF) << 24;

		BUFFER[0] = DC_NROC; // DC_NROC if send to EVB, or
		BUFFER[0] |= EVT_Type_DATA;
		BUFFER[1] = itrg;
		BUFFER[2] = LENEVENT; //--
		BUFFER[3] = PAD; //--

		BUFFER[0] &= ~(0xff << 24); // --  clear ModID
		unsigned int MODIDx = ((modID + i) & 0xff) << 24; //-- ModID=modID+i
		BUFFER[0] |= MODIDx;


		int evtModID = (BUFFER[0] >> 24) & 0xff;
		unsigned int evtTrigID = BUFFER[1];
		int evtSize = BUFFER[2];
		int evtPad = BUFFER[3];

		if (itrg < 200) {
			printf("==> SEND:: Trg=%d(%d,%d) Mod=%d(%d) siz=%d(%d), evtPad=%d\n"
			       , evtTrigID, itrg, BUFFER[1], evtModID, i, evtSize, LENEVENT, evtPad);
		}

		//rc=tcp_event_snd(BUFFER,LENEVENT,nmod,i,hdr,itrg);
		//if (rc<0) { printf(" ERROR send \n"); sleep(1); }
		//--------------------------------------------------
		//int tcp_event_snd( unsigned int *DATA, int lenDATA,int n,int k, unsigned int evtHDR, unsigned int TriggerID )

		unsigned int* DATA = BUFFER;
		int lenDATA = LENEVENT;
		int n = nmod;
		int k = i;
		unsigned int evtHDR = hdr;
		unsigned int TriggerID = itrg;

		HEADER[0] = 0x5; //---  buffered for evb
		HEADER[1] = 0xAABBCCDD;
		HEADER[2] = lenDATA;
		HEADER[3] = evtHDR;
		HEADER[4] = TriggerID;
		HEADER[5] = n;
		HEADER[6] = k;
		HEADER[7] = k;

		sock->SendRaw((char *)HEADER, sizeof(HEADER), kDefault);
		sock->SendRaw((char *)DATA, lenDATA * 4, kDefault);


		printf("read GNN out, wait for FPGA data ... \n"); //=======================================================

#define MAX_NODES 100
		unsigned int NDATA[MAX_NODES + 10];
		int RHEADER[10];
		int COLMAP[] = {1, 2, 3, 4, 6, 5};

		sock->RecvRaw((char *)RHEADER, sizeof(RHEADER), kDefault);
		int lenNODES = RHEADER[2];
		printf("RHEADER::");
		for (int ih = 0; ih < 5; ih++) printf(" %d (0x%x) \n", RHEADER[ih], RHEADER[ih]);
		printf(" LenDATA=%d \n", RHEADER[2]);

		sock->RecvRaw((char *)NDATA, lenNODES * 4, kDefault);

		PAD = NDATA[3];
		int nnodes = lenNODES - 4 - PAD; //-- 4 is size of header
		printf("nodes return: %d (nclust=%d), TRKS: \n", nnodes, nclust);


		unsigned int TDATA[2048];
		float* FTDATA = (float *)TDATA;
#if (USE_FIT==1)
		printf("read FIT out, wait for FPGA data ... \n"); //=======================================================
		//RHEADER[10];

		sock->RecvRaw((char *)RHEADER, sizeof(RHEADER), kDefault);
		int lenFITS = RHEADER[2];
		printf("RHEADER::");
		for (int ih = 0; ih < 5; ih++) printf(" %d \n", RHEADER[ih]);
		printf(" LenDATA=%d \n", RHEADER[2]);
		sock->RecvRaw((char *)TDATA, lenFITS * 4, kDefault);

		PAD = TDATA[3];

		for (int i = 0; i < lenFITS; i++) {
			printf("tracks fit return: i=%d  data=0x%x (%f)  \n", i, TDATA[i], FTDATA[i]);
		}


		int nfits = lenFITS - 4 - PAD;
		printf("tracks fit return: %d  PAD=%d , TDATA[3]=%d \n", nfits, PAD, TDATA[3]);

#else
	int nfits=nnodes;
	printf("tracks fit return: %d  \n", nfits);
#endif  // --- end  if USE_FIT  ---


#endif  // --- end  if USE_TCP  ---

		if (nfits > 0) {
			//=============== Draw FPGA Clust =============

			for (int nd = 0; nd < std::min(nnodes, nhits); nd++) {
				int trknum = NDATA[nd + 4];
				printf(" %u, ", trknum);
				c2->cd(3);
				gPad->Modified();
				gPad->Update();
				if (trknum > 0) {
					//DrawPolyMarker (1, &hits_Xpos[nd], &hits_Zpos[n], Option_t *option="")
					//TMarker* m = new TMarker(hits_Xpos[nd],hits_Zpos[n],24);  // memory leak !!!!!
					TMarker m = TMarker(hits_Zpos[nd], hits_Xpos[nd], 24);
					int tcol = std::min(trknum, 6);
					int mcol = COLMAP[tcol - 1];
					m.SetMarkerColor(mcol);
					m.SetMarkerStyle(20);
					m.SetMarkerSize(1.5);
					m.DrawClone();
					gPad->Modified();
					gPad->Update();
					printf("=====>  Draw:%d X,Y,Z,E=%f %f %f %f \n", nd, hits_Xpos[nd], hits_Ypos[nd], hits_Zpos[nd],
					       hits_dEdx[nd]);
				}
			}
			c2->Modified();
			c2->Update();
			printf("\n");


#if (USE_FIT==1)
			//=============== Draw Tracks lines =============

			int ntracks = nfits / 3;
			int cs = nfits % 3;
			if (cs != 0) {
				printf("==========>>>>   Error FIT results : %d %d %d \n", nfits, ntracks, cs);
				break;
			};

			int cnt = 4; // word counter in data buffer
			c2->cd(3);
			gPad->Modified();
			gPad->Update();
			for (int i = 0; i < ntracks; i++) {
				int trknum = TDATA[cnt++];
				float aa = FTDATA[cnt++];
				float bb = FTDATA[cnt++];
				printf(" Fit Track=%d aa=%f bb=%f \n", trknum, aa, bb);

				TF1 ftrk("ftrk", "[0]*x+[1]", zStart, zEnd);
				ftrk.SetParameter(0, aa);
				ftrk.SetParameter(1, bb);
				ftrk.DrawClone("same");
				gPad->Modified();
				gPad->Update();
			}
			c2->Modified();
			c2->Update();
#endif  // --- end  if USE_FIT  ---
		}
		else {
			printf(" No tracks to draw \n");
		}

		//=============== Draw All Clust ================
		//----------------------  To FPGA ------------------
		c2->cd(2);
		gPad->Modified();
		gPad->Update();
		printf(" Draw clusters  \n");
		for (int k = 0; k < nhits; k++) {
			TMarker mh = TMarker(hits_Zpos[k], hits_Xpos[k], 24);
			int mhcol = 1;
			mh.SetMarkerColor(mhcol);
			mh.SetMarkerSize(1.5);
			mh.DrawClone();
			gPad->Modified();
			gPad->Update();
		}
		c2->Modified();
		c2->Update();
		//----------------------  from FPGA ------------------
		c2->cd(3);
		gPad->Modified();
		gPad->Update();
		printf(" Draw clusters  \n");
		for (int k = 0; k < nhits; k++) {
			TMarker mh = TMarker(hits_Zpos[k], hits_Xpos[k], 24);
			int mhcol = 1;
			mh.SetMarkerColor(mhcol);
			mh.SetMarkerSize(1.5);
			//c2->cd(2); mh.DrawClone();  c2->Modified(); c2->Update();
			mh.DrawClone();
			gPad->Modified();
			gPad->Update();
		}
		c2->Modified();
		c2->Update();
	} // -- end mod  ---
	printf(" all done, click low right pad ...  \n");
	//c2->cd(2); gPad->WaitPrimitive();
	//if (jentry<35) sleep(5); else sleep(1);


	//-------------------------------------------------------

#endif   // (USE_FPGA>0)


	//=======================  End Fa125 RAW  process Loop  =====================================================
}


//------------------
// Finish
//------------------
void FpgaConnectionProcessor::Finish() {
	//    m_log->trace("FpgaConnectionProcessor finished\n");
}

