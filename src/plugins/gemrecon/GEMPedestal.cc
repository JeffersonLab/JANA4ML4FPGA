#include "GEMPedestal.h"
#include "GEMRawDecoder.h"
#include "GemMapping.h"
#include "GEMRawPedestal.h"
#include "GemConfiguration.h"
#include <stdio.h>
#include <TCanvas.h>
#include <iostream>
#include <filesystem>
#include <gem/constants.h>

using namespace std;

//=========================================================================================================================
GEMPedestal::GEMPedestal(std::string pedFileName, int nbOfTimeSamples) {
    printf("   GEMPedestal::GEMPedestal() ==> Start init \n");
    printf("   GEMPedestal::GEMPedestal() ==> nbOfTimeSamples %i \n", nbOfTimeSamples);
    printf("   GEMPedestal::GEMPedestal() ==> pedestal file %s\n", fPedFileName.Data());

    fPedFileName = pedFileName;
    fRawPedestal = new GEMRawPedestal(nbOfTimeSamples);
    mapping = GemMapping::GetInstance();
    nNbofAPVs = mapping->GetNbOfAPVs();
    FECs.clear();
    FECs = mapping->GetBankIDSet();

    printf("   GEMPedestal::GEMPedestal() ==> NCH %i \n", gem::ChannelsCount);
    printf("   GEMPedestal::GEMPedestal() ==> nNbofAPVs %i \n", nNbofAPVs);
    printf("   GEMPedestal::GEMPedestal() ==> FECs.size() %i \n", FECs.size());
    printf("   GEMPedestal::GEMPedestal() ==> End init \n");

}

//=========================================================================================================================
GEMPedestal::~GEMPedestal() {
    FECs.clear();
}

//=========================================================================================================================
void GEMPedestal::Delete() {
    int N = vStripOffsetHistos.size();
    for (int i = 0; i < N; i++) {
        delete vStripOffsetHistos[i];
        delete vStripNoiseHistos[i];
    }
    int M = vApvPedestalOffset.size();
    for (int i = 0; i < M; i++) {
        delete vApvPedestalOffset[i];
        delete vApvPedestalNoise[i];
    }
}

//=========================================================================================================================
void GEMPedestal::BookHistos() {
    printf("   GEMPedestal::BookHistos() ==> Enter \n");
    //book histograms for each strip
    for (int chNo = 0; chNo < NCH; chNo++) {
        for (int apvKey = 0; apvKey < nNbofAPVs; apvKey++) {
            stringstream out;
            stringstream apv;
            apv << apvKey;
            out << chNo;
            TString chNoStr = out.str();
            TString apvStr = apv.str();
            TString noise = "hNoise_" + apvStr + "_" + chNoStr;
            TString offset = "hOffset_" + apvStr + "_" + chNoStr;
            vStripNoiseHistos.push_back(new TH1F(noise, noise, 8097, -4048, 4048));
            vStripOffsetHistos.push_back(new TH1F(offset, offset, 8097, -4048, 4048));
        }
    }

    // book histograms for each APV
    for (int apvKey = 0; apvKey < nNbofAPVs; apvKey++) { ;
        stringstream out;
        out << apvKey;
        TString outStr = out.str();
        std::string hist_name = GetHistoName(apvKey, "offset", "");
        vApvPedestalOffset.push_back(new TH1F(hist_name.c_str(), hist_name.c_str(), 128, -0.5, 127.5));
        hist_name = GetHistoName(apvKey, "noise", "");
        vApvPedestalNoise.push_back(new TH1F(hist_name.c_str(), hist_name.c_str(), 128, -0.5, 127.5));
    }

    // book histograms for overall distribution
    hAllStripNoise = new TH1F("hAllStripNoise", "Overall Noise Distribution", 100, 0, 10);
    hAllXStripNoise = new TH1F("hAllXStripNoise", "Overall X Direction Noise Distribution", 100, 0, 10);
    hAllYStripNoise = new TH1F("hAllYStripNoise", "Overall Y Direction Noise Distribution", 100, 0, 10);
    printf("   GEMPedestal::BookHistos() ==> Exit \n");
}

//=========================================================================================================================
std::string GEMPedestal::GetHistoName(Int_t apvKey, TString dataType, TString dataNb) {
    Int_t apvID = mapping->GetAPVIDFromAPVNo(apvKey);
    TString apvName = mapping->GetAPVFromID(apvID);
    TString histoName = dataType + dataNb + "_" + apvName;
    return histoName.Data();
}


//=========================================================================================================================
void GEMPedestal::AccumulateEvent(int evtID, map<int, vector<int> > &mAPVRawTSs) {
    if (evtID % 250 == 0) printf("   GEMPedestal::AccumulateEvent() ==> process event Id = %d\n", evtID);
    fRawPedestal->ComputeEventPedestal(mAPVRawTSs);
    for (auto &i: mAPVRawTSs) {
        int apvid = i.first;
        int apvKey = mapping->GetAPVNoFromID(apvid);
        for (int chNo = 0; chNo < NCH; chNo++) {
            //cout << "AccumulateEvent() " <<  fRawPedestal->GetStripNoise(apvid, chNo) << endl;
            vStripNoiseHistos[apvKey * NCH + chNo]->Fill(fRawPedestal->GetStripNoise(apvid, chNo));
            vStripOffsetHistos[apvKey * NCH + chNo]->Fill(fRawPedestal->GetStripOffset(apvid, chNo));
        }
    }
}

//=========================================================================================================================
void GEMPedestal::ComputePedestal() {
    for (int apvKey = 0; apvKey < nNbofAPVs; apvKey++) {
        for (int chNo = 0; chNo < NCH; chNo++) {
            Float_t offset = vStripOffsetHistos[apvKey * NCH + chNo]->GetMean();
            Float_t noise = vStripNoiseHistos[apvKey * NCH + chNo]->GetRMS();

            vApvPedestalOffset[apvKey]->SetBinContent(chNo, offset);
            vApvPedestalNoise[apvKey]->SetBinContent(chNo, noise);

            hAllStripNoise->Fill(noise);
            Int_t fAPVID = mapping->GetAPVIDFromAPVNo(apvKey);
            TString plane = mapping->GetPlaneFromAPVID(fAPVID);
            //std::cout << " palne = " << plane << " noise " << noise <<std::endl;
            if (plane.Contains("X")) hAllXStripNoise->Fill(noise);
            else if (plane.Contains("Y")) hAllYStripNoise->Fill(noise);
            else cout << "GEMPedestal::ComputePedestal: Error: Unrecongnized plane name..." << endl;
        }
    }
}

//=========================================================================================================================
void GEMPedestal::SavePedestalFile() {
    printf("   GEMPedestal::SavePedestalFile() ==> save pedestal data in root file %s\n", fPedFileName.Data());
    TFile *file = new TFile(fPedFileName.Data(), "recreate");
    cout << "SavePedestalFile()  0 " << endl;
    ComputePedestal();
    cout << "SavePedestalFile()  1 " << endl;
    for (int apvKey = 0; apvKey < nNbofAPVs; apvKey++) {
        cout << "SavePedestalFile()  loop " << apvKey << endl;
        vApvPedestalOffset[apvKey]->Write();
        vApvPedestalNoise[apvKey]->Write();
    }
    hAllStripNoise->Write();
    hAllXStripNoise->Write();
    hAllYStripNoise->Write();
    file->Write();
    file->Close();
}

//=========================================================================================================================
void GEMPedestal::LoadPedestal() {
    printf("   GEMPedestal::LoadPedestal() ==> Loading pedestal file %s\n", fPedFileName.Data());

    if (!std::filesystem::exists(fPedFileName.Data())) {
        throw std::runtime_error(std::string("Pedestal file not found: ") + fPedFileName.Data());
    }
    TFile file(fPedFileName.Data(), "READ");
    if (file.IsZombie()) {
        std::string message = std::string("Pedestal file IsZombie") + fPedFileName.Data();
        throw std::runtime_error(std::string("Pedestal file not found: ") + fPedFileName.Data());
    }
    Int_t nAPVs = mapping->GetNbOfAPVs();
    for (int i = 0; i < nAPVs; i++) {
        stringstream out;
        out << i;
        TString outStr = out.str();
        vApvPedestalOffset.push_back((TH1F *) file.Get(GetHistoName(i, "offset", "").c_str()));
        vApvPedestalOffset[i]->SetDirectory(0);
        vApvPedestalNoise.push_back((TH1F *) file.Get(GetHistoName(i, "noise", "").c_str()));
        vApvPedestalNoise[i]->SetDirectory(0);
        printf("   GEMPedestal::LoadPedestal() ==> Got data for APV %i NBins offset: %i NBins noise: %i \n", i, vApvPedestalOffset[i]->GetNbinsX(), vApvPedestalNoise[i]->GetNbinsY());
    }
    // Cannot close file while pedestal histograms are still being used...
    file.Close();
}

//=========================================================================================================================
vector<Float_t> GEMPedestal::GetAPVNoises(Int_t apvid) {
    vector<Float_t> noises;
    Int_t apvNo = mapping->GetAPVNoFromID(apvid);
    for (int i = 0; i < NCH; i++) noises.push_back(vApvPedestalNoise[apvNo]->GetBinContent(i));
    return noises;
}

//=========================================================================================================================
vector<Float_t> GEMPedestal::GetAPVOffsets(Int_t apvid) {
    vector<Float_t> offsets;
    Int_t apvNo = mapping->GetAPVNoFromID(apvid);
    for (int i = 0; i < NCH; i++) offsets.push_back(vApvPedestalOffset[apvNo]->GetBinContent(i));
    return offsets;
}
