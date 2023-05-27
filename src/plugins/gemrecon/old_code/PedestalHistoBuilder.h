// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <sstream>
#include "plugins/gemrecon/Constants.h"

#include <TH1F.h>

class PedestalHistoBuilder {
public:

//
////=========================================================================================================================
//    void BookHistos() {
//        using namespace std;
//        printf("   GEMPedestal::BookHistos() ==> Enter \n");
//        //book histograms for each strip
//        for (int chNo = 0; chNo < gem::ChannelsCount; chNo++) {
//            for (int apvKey = 0; apvKey < nNbofAPVs; apvKey++) {
//                stringstream out;
//                stringstream apv;
//                apv << apvKey;
//                out << chNo;
//                TString chNoStr = out.str();
//                TString apvStr = apv.str();
//                TString noise = "hNoise_" + apvStr + "_" + chNoStr;
//                TString offset = "hOffset_" + apvStr + "_" + chNoStr;
//                vStripNoiseHistos.push_back(new TH1F(noise, noise, 8097, -4048, 4048));
//                vStripOffsetHistos.push_back(new TH1F(offset, offset, 8097, -4048, 4048));
//            }
//        }
//
//        // book histograms for each APV
//        for (int apvKey = 0; apvKey < nNbofAPVs; apvKey++) { ;
//            stringstream out;
//            out << apvKey;
//            TString outStr = out.str();
//            vApvPedestalOffset.push_back(new TH1F(GetHistoName(apvKey, "offset", ""), GetHistoName(apvKey, "offset", ""), 128, -0.5, 127.5));
//            vApvPedestalNoise.push_back(new TH1F(GetHistoName(apvKey, "noise", ""), GetHistoName(apvKey, "noise", ""), 128, -0.5, 127.5));
//        }
//
//        // book histograms for overall distribution
//        hAllStripNoise = new TH1F("hAllStripNoise", "Overall Noise Distribution", 100, 0, 10);
//        hAllXStripNoise = new TH1F("hAllXStripNoise", "Overall X Direction Noise Distribution", 100, 0, 10);
//        hAllYStripNoise = new TH1F("hAllYStripNoise", "Overall Y Direction Noise Distribution", 100, 0, 10);
//        printf("   GEMPedestal::BookHistos() ==> Exit \n");
//    }
//private:
//
//    TH1F *hAllStripNoise;
//    TH1F *hAllXStripNoise;
//    TH1F *hAllYStripNoise;
};
