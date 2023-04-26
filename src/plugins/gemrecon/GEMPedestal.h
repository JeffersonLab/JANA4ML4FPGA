#ifndef __GEMPEDESTAL_H__
#define __GEMPEDESTAL_H__

#include <fstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <TH1F.h>
#include <TFile.h>

#include <gem/constants.h>

class GEMRawDecoder;

class GemMapping;

class GEMRawPedestal;

class GEMPedestal {
public:
    const int NCH = gem::ChannelsCount;     // Number of channels, which is 128 everywhere

    GEMPedestal(std::string pedFileName, int nbOfTimeSamples);

    ~GEMPedestal();

    void BookHistos();

    void AccumulateEvent(int evtID, std::map<int, std::vector<int> > &event);

    void ComputePedestal();

    void SavePedestalFile();

    void LoadPedestal();

    std::string GetHistoName(Int_t apvKey, TString dataType, TString dataNb);

    void Delete();

    std::vector<Float_t> GetAPVNoises(int);

    std::vector<Float_t> GetAPVOffsets(int);

private:
    std::set<int> FECs;

    int nNbofAPVs;

    TString fPedFileName;

    std::ifstream file;

    std::vector<TH1F *> vStripOffsetHistos;
    std::vector<TH1F *> vStripNoiseHistos;
    std::vector<TH1F *> vApvPedestalOffset;
    std::vector<TH1F *> vApvPedestalNoise;

    TH1F *hAllStripNoise;
    TH1F *hAllXStripNoise;
    TH1F *hAllYStripNoise;

    GemMapping *mapping;
    GEMRawPedestal *fRawPedestal;
};

#endif
