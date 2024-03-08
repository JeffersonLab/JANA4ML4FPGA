//
// Created by Dmitry Romanov on 3/8/2024.
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "FpgaResultFactory.h"

#include <rawdataparser/Df125WindowRawData.h>
#include <JANA/JEvent.h>

#include <TSocket.h>
#include <TMarker.h>
#include <TCanvas.h>
#include <TH2.h>
#include <TF1.h>
#include <rawdataparser/Df125WindowRawData.h>

#define USE_TRK
#define MAX_PRINT 1

#define USE_TCP 1
#define USE_FIT 1
#define MAX_NODES 100
#define BUFSIZE 128000

#define USE_125_RAW




namespace ml4fpga::fpgacon {
    void FpgaResultFactory::Init() {
    	std::string plugin_name = GetPluginName();

    	// Get JANA application
    	auto app = GetApplication();
    	InitLogger(plugin_name);


    	app->SetDefaultParameter(plugin_name + ":use_fpga", m_cfg_use_tcp, "Send messages to FPGA via TCP");


    }

    void FpgaResultFactory::Process(const std::shared_ptr<const JEvent> &event) {


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


    		unsigned int* DATA = BUFFER;
    		int lenDATA = LENEVENT;
    		int n = nmod;
    		int k = i;
    		unsigned int evtHDR = (i & 0xF) << 24;
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
    }


	//=======================  End Fa125 RAW  process Loop  =====================================================

*/
    }

    void FpgaResultFactory::Finish() {
        JFactoryT<FpgaResult>::Finish();
    }
}
