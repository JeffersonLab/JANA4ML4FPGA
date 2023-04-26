#include "GEMOnlineHitDecoder.h"
// #include "GemView.h"
#include <spdlog/fmt/fmt.h>

using namespace std;

static string trim(const string &str, const string &w = " \t\n\r") {
    const auto strBegin = str.find_first_not_of(w);
    if (strBegin == string::npos) return ""; // no content
    const auto strEnd = str.find_last_not_of(w);
    const auto strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
}


//======================================================
GEMOnlineHitDecoder::GEMOnlineHitDecoder(Int_t iEntry, Int_t nbOfTS, Int_t startTS, Int_t stopTS, Int_t zeroSupCut,
                                         Int_t comModeCut, TString isPeakOrSum, TString centralOrAllStrips,
                                         Float_t minADCs, Int_t minClustSize, Int_t maxClustSize, Int_t maxClustMult) {

    NCH = 128;
    fEventNb = iEntry;

    fZeroSupCut = zeroSupCut;
    fComModeCut = comModeCut;
    fMinADCvalue = minADCs;

    fNbOfTimeSamples = nbOfTS;
    fStopTimeSamples = stopTS;
    fStartTimeSamples = startTS;
    //  assert(fStartTimeSamples > 0);
    assert(fStopTimeSamples < fNbOfTimeSamples);
    assert(!(fStopTimeSamples > fNbOfTimeSamples));

    fIsHitMaxOrTotalADCs = isPeakOrSum;
    fStartData = 0;
    fMapping = GemMapping::GetInstance();
    FECs.clear();
    mSrsSingleEvent.clear();
    FECs = fMapping->GetBankIDSet();
    fAPVBaseline = 2500;

    //=======================//
    // cluster information   //
    //=======================//
    fMinClusterSize = minClustSize;
    fMaxClusterSize = maxClustSize;
    fMaxClusterMult = maxClustMult;
    fIsCentralOrAllStripsADCs = centralOrAllStrips;
    fIsGoodClusterEvent = kFALSE;
    fListOfClustersCleanFromPlane.clear();
}

//===================================================================
Bool_t GEMOnlineHitDecoder::IsADCchannelActive() {
    Bool_t isADCchannelActive = kFALSE;
    GemMapping *mapping = GemMapping::GetInstance();
    fActiveADCchannels = mapping->GetActiveADCchannels(fFECID);
    if (find(fActiveADCchannels.begin(), fActiveADCchannels.end(), fADCChannel) != fActiveADCchannels.end()) {
        fAPVKey = mapping->GetAPVNoFromID(fAPVID);
        fAPVHeaderLevel = mapping->GetAPVHeaderLevelFromID(fAPVID);
        isADCchannelActive = kTRUE;
    }
    return isADCchannelActive;
}

//==================================================================================
void GEMOnlineHitDecoder::DeleteClustersInPlaneMap() {
    map<TString, list<GEMCluster *> >::const_iterator itr;
    for (itr = fListOfClustersCleanFromPlane.begin(); itr != fListOfClustersCleanFromPlane.end(); itr++) {
        list<GEMCluster *> listOfCluster = (*itr).second;
        for (auto &&cluster: listOfCluster) {
            delete cluster;
        }
        listOfCluster.clear();
    }
    fListOfClustersCleanFromPlane.clear();
}

//_______________________________________________________________________________
GEMOnlineHitDecoder::~GEMOnlineHitDecoder() {
    //clear hits
    printf("~GEMOnlineHitDecoder()::  delete hits = %d\n", fListOfHits.size());
    map<int, GEMHit *>::iterator it = fListOfHits.begin();
    for (; it != fListOfHits.end(); ++it) {
        delete (it->second);
        //printf("~GEMOnlineHitDecoder()::  delete hit = %p\n",it->second);
    }
    fListOfHits.clear();

    if (fListOfClustersCleanFromPlane.size() > 0) {
        map<TString, list<GEMCluster *> >::iterator itt = fListOfClustersCleanFromPlane.begin();
        for (; itt != fListOfClustersCleanFromPlane.end(); ++itt) {
            list<GEMCluster *>::iterator itc = (itt->second).begin();
            for (; itc != (itt->second).end(); ++itc) delete *itc;
            (itt->second).clear();
        }
    }
    fListOfClustersCleanFromPlane.clear();

    fListOfHitsClean.clear();
    fListOfHitsFromPlane.clear();
    fListOfHitsCleanFromPlane.clear();

    fActiveADCchannels.clear();
    FECs.clear();
    mSrsSingleEvent.clear();
    DeleteClustersInPlaneMap();
}

//======================================================
void GEMOnlineHitDecoder::ProcessEvent(map<int, map<int, vector<int> > > srsSingleEvent, GEMPedestal *ped) {

    mSrsSingleEvent = srsSingleEvent;
    printf("\n Enter  GEMHitDecoder::ProcessEvent fListOfHits size =%d\n", fListOfHits.size());

    map<int, map<int, vector<int> > >::iterator it;
    for (it = mSrsSingleEvent.begin(); it != mSrsSingleEvent.end(); ++it) {
        fFECID = it->first;
        map<int, vector<int> > fec_data = it->second;
        map<int, vector<int> >::iterator itt;
        for (itt = fec_data.begin(); itt != fec_data.end(); ++itt) {
            fADCChannel = itt->first;
            fAPVID = fFECID << 4 | fADCChannel;
            if (IsADCchannelActive()) {
                fRawData16bits.clear();
                fRawData16bits = itt->second;
                assert(fRawData16bits.size() > 0);

                fAPVStatus = fMapping->GetAPVstatus(fAPVID);
                string apv_status = fAPVStatus.Data();
                apv_status = trim(apv_status);
                auto apvMap = fMapping->GetDetectorFromAPVIDMap();

                fmt::print("    fMapping->GetDetectorFromAPVIDMap().size {:<10}", apvMap.size());
                for(auto mapItem: apvMap) {
                    int id = mapItem.first;
                    std::string name(mapItem.second);
                    fmt::print("    {:<10} {:<10}", id, name);
                }
                auto detNameTsrt = fMapping->GetDetectorFromAPVIDMap()[fAPVID];
                std::string detectorName(detNameTsrt);
                std::string detectorType(fMapping->GetDetectorTypeFromDetector(detectorName));


                fPedestalNoises.clear();
                fPedestalOffsets.clear();
                fPedestalNoises = ped->GetAPVNoises(fAPVID);
                fPedestalOffsets = ped->GetAPVOffsets(fAPVID);
                fAPVBaseline = std::accumulate(fPedestalOffsets.begin(), fPedestalOffsets.end(), 0.0) / NCH;

                fPedestalNoises_1stSet.clear();
                fPedestalOffsets_1stSet.clear();
                fPedestalNoises_2ndSet.clear();
                fPedestalOffsets_2ndSet.clear();
                if ((detectorType == "PRADGEM") && (apv_status != "normal")) {
                    for (Int_t chNo = 0; chNo < NCH; chNo++) {
                        if (fMapping->PRadStripMapping(fAPVID, chNo) < 16) {
                            fPedestalOffsets_1stSet.push_back(fPedestalOffsets[chNo]);
                            fPedestalNoises_1stSet.push_back(fPedestalNoises[chNo]);
                        } else {
                            fPedestalOffsets_2ndSet.push_back(fPedestalOffsets[chNo]);
                            fPedestalNoises_2ndSet.push_back(fPedestalNoises[chNo]);
                        }
                    }
                }
                // DECODE DATA FROM SPECIAL APVs into Hits
                if ((detectorType == "PRADGEM") && (apv_status != "normal")) APVEventSplitChannelsDecoder();
                else APVEventDecoder();
            }
        }
    }

    //clear mSrsSingleEvent
    for (it = mSrsSingleEvent.begin(); it != mSrsSingleEvent.end(); ++it) {
        map<int, vector<int> > fec_data = it->second;
        map<int, vector<int> >::iterator itt;
        for (itt = fec_data.begin(); itt != fec_data.end(); ++itt) {
            itt->second.clear();
        }
        it->second.clear();
    }
    mSrsSingleEvent.clear();
    GetListOfHitsFromPlanes();
    GetListOfHitsCleanFromPlanes();
    // cluster computing
    ComputeClusters();
}

//=====================================================
void GEMOnlineHitDecoder::APVEventDecoder() {
    //  printf(" Enter  GEMHitDecoder::APVEventDecoder()\n") ;

    Int_t idata = 0, firstdata = 0, lastdata = 0;
    Int_t size = fRawData16bits.size();
    int commonMode = 0;
    vector<Float_t> rawDataTS, rawDataZS;
    vector<Float_t> dataTest, commonModeOffsets;
    rawDataTS.clear();

    // COMPUTE APV25 COMMON MODE CORRECTION
    //if (fIsNewFECdataFlag) {
    if (0) {
        for (idata = 0; idata < size; idata++) {
            if (fRawData16bits[idata] < fAPVHeaderLevel) {
                idata++;
                if (fRawData16bits[idata] < fAPVHeaderLevel) {
                    idata++;
                    if (fRawData16bits[idata] < fAPVHeaderLevel) {
                        idata += 10;
                        fStartData = idata;
                        idata = size;
                    }
                }
            }
        }
        //fIsNewFECdataFlag = kFALSE ;
    }
    //xinzhan check raw data

    //===========================================================================================================
    // BLOCK WHERE COMMON MODE CORRECTION FOR ALL APV25 TIME BINS IS COMPUTED
    //===========================================================================================================
    firstdata = fStartData;
    lastdata = firstdata + NCH;

    for (Int_t timebin = 0; timebin < fNbOfTimeSamples; timebin++) {
        // printf("GEMHitDecoder::APVEventDecoder()  timebin=%d lastdata=%d ,size=%d \n",timebin,lastdata,size);
        // EXTRACT APV25 DATA FOR A GIVEN TIME BIN
        if (lastdata > size) {
            cout << "###ERROR:  GEMHitDecoder: APV Raw Data out of Range" << endl;
            cout << "           please adjust your header level in mapping file" << endl;
            cout << "           increase recommended..." << endl;
        }

        rawDataTS.insert(rawDataTS.end(), &fRawData16bits[firstdata], &fRawData16bits[lastdata]);
        assert(rawDataTS.size() == NCH);
        dataTest.insert(dataTest.end(), &fRawData16bits[firstdata], &fRawData16bits[lastdata]);
        assert(dataTest.size() == NCH);

        // PERFORM APV25 PEDESTAL OFFSET CORRECTION  FOR A GIVEN TIME BIN
        std::transform(rawDataTS.begin(), rawDataTS.end(), fPedestalOffsets.begin(), rawDataTS.begin(), std::minus<Float_t>());
        std::transform(dataTest.begin(), dataTest.end(), fPedestalOffsets.begin(), dataTest.begin(), std::minus<Float_t>());

        map<Float_t, Int_t> rawdatamap;
        for (int j = 0; j < NCH; j++) {
            rawdatamap[dataTest[j]] = j;
        }
        // Select only 100 channels with lowest adc
        std::sort(dataTest.begin(), dataTest.end());
        assert(fComModeCut < 28);
        for (int i = 0; i < fComModeCut; i++) {
            //     if(fAPVID == 0) printf("\n Enter  GEMHitDecoder::APVEventDecoder()=>BF: data[%d]=%f \n",timebin,dataTest[i]) ;
            dataTest[i] = -fPedestalOffsets[rawdatamap[dataTest[i]]] + fAPVBaseline;
            //     if(fAPVID == 0) printf(" Enter  GEMHitDecoder::APVEventDecoder()=>AF: data[%d]=%f \n",  timebin,dataTest[i]) ;
        }
        assert(dataTest.size() == NCH);
        rawdatamap.clear();

        // COMPUTE COMMON MODE FOR A GIVEN APV AND TIME BIN
        commonMode = std::accumulate(dataTest.begin(), dataTest.end(), 0.0) / NCH;
        commonModeOffsets.push_back(commonMode);
        //    if(fAPVID == 0) printf(" Enter  GEMHitDecoder::APVEventDecoder(), timebin = %d, commonMode = %d \n",  timebin, commonMode) ;

        // PERFORM COMMON MODE CORRECTION FOR A GIVEN TIME BIN
        std::transform(rawDataTS.begin(), rawDataTS.end(), rawDataTS.begin(),
                       std::bind2nd(std::minus<Float_t>(), commonMode));

        //  ADC SUM OVER ALL TIME BINS USE AS THE TEST CRITERIA FOR ZERO SUPPRESSION
        if (timebin == 0) rawDataZS.resize(rawDataTS.size());
        std::transform(rawDataZS.begin(), rawDataZS.end(), rawDataTS.begin(), rawDataZS.begin(), std::plus<Float_t>());

        // PROCEED TO NEXT TIME BIN
        firstdata = lastdata;
        lastdata = firstdata + NCH;

        // CLEAR EVERYTHING
        rawDataTS.clear();
        dataTest.clear();
    }

    // ADC AVERAGE OVER ALL TIME BINS USE AS THE TEST CRITERIA FOR ZERO SUPPRESSION
    std::transform(rawDataZS.begin(), rawDataZS.end(), rawDataZS.begin(),
                   std::bind2nd(std::divides<Float_t>(), (Float_t) fNbOfTimeSamples));

    //=============================================================================================================================
    // BLOCK WHERE ZERO SUPPRESSION IS COMPUTED
    //=============================================================================================================================
    firstdata = fStartData;
    lastdata = firstdata + NCH;

    Bool_t isCommonModeTooLarge = kFALSE;
    for (Int_t timebin = 0; timebin < fNbOfTimeSamples; timebin++) {
        if (fabs(commonModeOffsets[timebin]) > 200) {
            isCommonModeTooLarge = kTRUE;
            break;
        }
    }
    //if (isCommonModeTooLarge) return;

    for (Int_t timebin = 0; timebin < fNbOfTimeSamples; timebin++) {
        // EXTRACT APV25 DATA FOR A GIVEN TIME BIN
        rawDataTS.insert(rawDataTS.end(), &fRawData16bits[firstdata], &fRawData16bits[lastdata]);
        for (Int_t chNo = 0; chNo < NCH; chNo++) {
            Int_t hitID = (fAPVKey << 8) | chNo;
            Float_t data = -(rawDataTS[chNo] - fPedestalOffsets[chNo] - commonModeOffsets[timebin]);
            Float_t avgdata = -rawDataZS[chNo];

            //   if (fAPVID % 2 == 1) data = 1.2 * data ;

            // NO ZERO SUPPRESSION
            if (!fListOfHits[hitID]) {
                GEMHit *hit = new GEMHit(hitID, fAPVID, chNo, 0, fIsHitMaxOrTotalADCs, fNbOfTimeSamples,
                                         fStopTimeSamples, fStartTimeSamples);
                fListOfHits[hitID] = hit;
            }
            fListOfHits[hitID]->AddTimeBinADCs(timebin, data, fPedestalNoises[chNo]);
            TString planename = fListOfHits[hitID]->GetPlane();

            // ZERO SUPPRESSION
            if (avgdata > (fZeroSupCut * fPedestalNoises[chNo])) {
                if (!fListOfHitsClean[hitID]) {
                    GEMHit *hit = new GEMHit(hitID, fAPVID, chNo, fZeroSupCut, fIsHitMaxOrTotalADCs, fNbOfTimeSamples,
                                             fStopTimeSamples, fStartTimeSamples);
                    fListOfHitsClean[hitID] = hit;
                }
                fListOfHitsClean[hitID]->AddTimeBinADCs(timebin, data, fPedestalNoises[chNo]);
            }
        }
        firstdata = lastdata;
        lastdata = firstdata + NCH;
        rawDataTS.clear();
    }
    commonModeOffsets.clear();
}

//=====================================================
void GEMOnlineHitDecoder::APVEventSplitChannelsDecoder() {
    //printf(" Enter  GEMHitDecoder::APVEventDecoderSpecialChannel()\n") ;

    Int_t idata = 0, firstdata = 0, lastdata = 0;

    Int_t size = fRawData16bits.size();
    Int_t size1 = fPedestalNoises_1stSet.size();
    Int_t size2 = fPedestalNoises_2ndSet.size();

    Float_t commonMode = 0;

    vector<Float_t> rawDataTS, rawDataTS_1stSet, rawDataTS_2ndSet;
    vector<Float_t> rawDataZS_1stSet, rawDataZS_2ndSet;
    vector<Float_t> threshold_1stSet, threshold_2ndSet;
    vector<Float_t> dataTest_1stSet, dataTest_2ndSet;
    vector<Float_t> commonModeOffsets_1stSet, commonModeOffsets_2ndSet;

    rawDataTS.clear();
    rawDataTS_1stSet.clear();
    rawDataTS_2ndSet.clear();

    rawDataZS_1stSet.clear();//resize(rawDataTS_1stSet.size()) ;
    rawDataZS_2ndSet.clear();//resize(rawDataTS_2ndSet.size()) ;
    threshold_1stSet.clear();
    threshold_2ndSet.clear();
    dataTest_1stSet.clear();
    dataTest_2ndSet.clear();
    commonModeOffsets_1stSet.clear();
    commonModeOffsets_2ndSet.clear();

    //  fNbOfTimeSamples = 0 ;

    // COMPUTE APV25 COMMON MODE CORRECTION
    //if (fIsNewFECdataFlag) {
    //if(1){
    for (idata = 0; idata < size; idata++) {
        if (fRawData16bits[idata] < fAPVHeaderLevel) {
            idata++;
            if (fRawData16bits[idata] < fAPVHeaderLevel) {
                idata++;
                if (fRawData16bits[idata] < fAPVHeaderLevel) {
                    idata += 10;
                    fStartData = idata;
                    idata = size;
                    //	    fNbOfTimeSamples++ ;
                }
            }
        }
    }
    //fIsNewFECdataFlag = kFALSE ;
    //}

    //===========================================================================================================
    // BLOCK WHERE COMMON MODE CORRECTION FOR ALL APV25 TIME BINS IS COMPUTED
    //===========================================================================================================
    firstdata = fStartData;
    lastdata = firstdata + NCH;

    for (Int_t timebin = 0; timebin < fNbOfTimeSamples; timebin++) {

        threshold_1stSet.resize(size1);
        threshold_2ndSet.resize(size2);
        dataTest_1stSet.resize(size1);
        dataTest_2ndSet.resize(size2);
        rawDataZS_1stSet.resize(size1);
        rawDataZS_2ndSet.resize(size2);

        //=======================================================
        // EXTRACT APV25 DATA FOR A GIVEN TIME BIN
        //=======================================================
        if (lastdata > size) {
            cout << "###Warning:  GEMHitDecoder: APV Raw Data out of Range" << endl;
            cout << "           please adjust your header level in mapping file" << endl;
            cout << "           increase recommended..." << endl;
            //GemView::TheClient->SendMessage("2.GEM.APV raw data out of range");
        }

        //assert( lastdata < size);
        rawDataTS.insert(rawDataTS.end(), &fRawData16bits[firstdata], &fRawData16bits[lastdata]);
        assert(rawDataTS.size() == 128);

        //split Channel data
        for (Int_t chNo = 0; chNo < NCH; chNo++) {
            if (fMapping->PRadStripMapping(fAPVID, chNo) < 16)
                rawDataTS_1stSet.push_back(rawDataTS[chNo]);
            else
                rawDataTS_2ndSet.push_back(rawDataTS[chNo]);
        }
        assert(rawDataTS_1stSet.size() == 16);
        assert(rawDataTS_2ndSet.size() == 112);

        //===============================================================
        //                     1ST SET OF CHANNELS
        //===============================================================

        // PERFORM APV25 PEDESTAL OFFSET CORRECTION  FOR A GIVEN TIME BIN
        std::transform(rawDataTS_1stSet.begin(), rawDataTS_1stSet.end(), fPedestalOffsets_1stSet.begin(),
                       rawDataTS_1stSet.begin(), std::minus<int>());

        // COMPUTE THRESOLD (= 20 PEDESTAL NOISE) FOR COMMON MODE DATA SELECTION
        threshold_1stSet.clear();
        threshold_1stSet.resize(rawDataTS_1stSet.size());
        std::transform(fPedestalNoises_1stSet.begin(), fPedestalNoises_1stSet.end(), threshold_1stSet.begin(),
                       std::bind2nd(std::multiplies<Float_t>(), 50.0));

        dataTest_1stSet.clear();
        dataTest_1stSet.insert(dataTest_1stSet.begin(), rawDataTS_1stSet.begin(), rawDataTS_1stSet.end());
        assert(rawDataTS_1stSet.size() == 16);
        int nbHits_1stSet = 0;
        for (int i = 0; i < 16; i++) {
            if ((dataTest_1stSet[i] + threshold_1stSet[i]) < 0) {
                dataTest_1stSet[i] = 0;
                nbHits_1stSet++;
            }
        }
        assert(nbHits_1stSet <= 16);

        // COMPUTE COMMON MODE FOR A GIVEN APV AND TIME BIN
        commonMode = std::accumulate(dataTest_1stSet.begin(), dataTest_1stSet.end(), 0.0) / (16 - nbHits_1stSet);
        commonModeOffsets_1stSet.push_back(commonMode);

        // PERFORM COMMON MODE CORRECTION FOR A GIVEN TIME BIN
        std::transform(rawDataTS_1stSet.begin(), rawDataTS_1stSet.end(), rawDataTS_1stSet.begin(),
                       std::bind2nd(std::minus<Float_t>(), commonMode));

        //  ADC SUM OVER ALL TIME BINS USE AS THE TEST CRITERIA FOR ZERO SUPPRESSION
        if (timebin == 0) rawDataZS_1stSet.resize(rawDataTS_1stSet.size());
        std::transform(rawDataZS_1stSet.begin(), rawDataZS_1stSet.end(), rawDataTS_1stSet.begin(),
                       rawDataZS_1stSet.begin(), std::plus<Float_t>());

        //======================================================================
        //                    2ND SET OF CHANNELS
        //======================================================================

        // PERFORM APV25 PEDESTAL OFFSET CORRECTION  FOR A GIVEN TIME BIN
        std::transform(rawDataTS_2ndSet.begin(), rawDataTS_2ndSet.end(), fPedestalOffsets_2ndSet.begin(),
                       rawDataTS_2ndSet.begin(), std::minus<Float_t>());

        // COMPUTE THRESOLD (= 20 PEDESTAL NOISE) FOR COMMON MODE DATA SELECTION
        threshold_2ndSet.clear();
        threshold_2ndSet.resize(rawDataTS_2ndSet.size());
        std::transform(fPedestalNoises_2ndSet.begin(), fPedestalNoises_2ndSet.end(), threshold_2ndSet.begin(),
                       std::bind2nd(std::multiplies<Float_t>(), 20.0));

        // TEST IF APV CHANNEL IS A HIT OR PEDESTAL => ROUGH TEST TO ELIMINATE HIT CHANNELS IN FOR THE COMMON MODE
        dataTest_2ndSet.clear();
        dataTest_2ndSet.insert(dataTest_2ndSet.begin(), rawDataTS_2ndSet.begin(), rawDataTS_2ndSet.end());

        int nbHits_2ndSet = 0;
        assert(size2 == 112);
        for (int i = 0; i < size2; i++) {
            if ((dataTest_2ndSet[i] + threshold_2ndSet[i]) < 0) {
                dataTest_2ndSet[i] = 0;
                nbHits_2ndSet++;
            }
        }
        assert(nbHits_2ndSet <= size2);

        // COMPUTE COMMON MODE AND PERFORM COMMON MODE CORRECTION FOR A GIVEN APV AND TIME BIN FOR 2ND SET OF CHANNELS
        commonMode = std::accumulate(dataTest_2ndSet.begin(), dataTest_2ndSet.end(), 0.0) / (112 - nbHits_2ndSet);
        commonModeOffsets_2ndSet.push_back(commonMode);
        std::transform(rawDataTS_2ndSet.begin(), rawDataTS_2ndSet.end(), rawDataTS_2ndSet.begin(),
                       std::bind2nd(std::minus<Float_t>(), commonMode));

        //  ADC SUM OVER ALL TIME BINS USE AS THE TEST CRITERIA FOR ZERO SUPPRESSION
        if (timebin == 0) {
            rawDataZS_2ndSet.clear();
            rawDataZS_2ndSet.resize(rawDataTS_2ndSet.size());
        }
        std::transform(rawDataZS_2ndSet.begin(), rawDataZS_2ndSet.end(), rawDataTS_2ndSet.begin(),
                       rawDataZS_2ndSet.begin(), std::plus<Float_t>());

        //===============================================================================
        //        CLEAR EVERYTHING
        //===============================================================================
        rawDataTS.clear();
        rawDataTS_1stSet.clear();
        rawDataTS_2ndSet.clear();
        threshold_1stSet.clear();
        threshold_2ndSet.clear();
        dataTest_1stSet.clear();
        dataTest_2ndSet.clear();

        // PROCEED TO NEXT TIME BIN
        firstdata = lastdata + 12;
        lastdata = firstdata + NCH;
    }

    // ADC AVERAGE OVER ALL TIME BINS USE AS THE TEST CRITERIA FOR ZERO SUPPRESSION
    std::transform(rawDataZS_1stSet.begin(), rawDataZS_1stSet.end(), rawDataZS_1stSet.begin(),
                   std::bind2nd(std::divides<Float_t>(), (Float_t) fNbOfTimeSamples));
    std::transform(rawDataZS_2ndSet.begin(), rawDataZS_2ndSet.end(), rawDataZS_2ndSet.begin(),
                   std::bind2nd(std::divides<Float_t>(), (Float_t) fNbOfTimeSamples));

    //=============================================================================================================================
    // BLOCK WHERE ZERO SUPPRESSION IS COMPUTED
    //=============================================================================================================================
    firstdata = fStartData;
    lastdata = firstdata + NCH;

    for (Int_t timebin = 0; timebin < fNbOfTimeSamples; timebin++) {

        // EXTRACT APV25 DATA FOR A GIVEN TIME BIN
        rawDataTS.insert(rawDataTS.end(), &fRawData16bits[firstdata], &fRawData16bits[lastdata]);
        int i1st = 0, i2nd = 0;

        for (Int_t chNo = 0; chNo < NCH; chNo++) {
            Int_t hitID = (fAPVKey << 8) | chNo;
            Float_t data = 0;
            Float_t avgdata = 0;
            if (fMapping->PRadStripMapping(fAPVID, chNo) < 16) {
                data = -(rawDataTS[chNo] - fPedestalOffsets[chNo] - commonModeOffsets_1stSet[timebin]);
                avgdata = -rawDataZS_1stSet[i1st];
                i1st++;
            } else {
                data = -(rawDataTS[chNo] - fPedestalOffsets[chNo] - commonModeOffsets_2ndSet[timebin]);
                avgdata = -rawDataZS_2ndSet[i2nd];
                i2nd++;
            }

            // NO ZERO SUPPRESSION
            if (!fListOfHits[hitID]) {
                GEMHit *hit = new GEMHit(hitID, fAPVID, chNo, 0, fIsHitMaxOrTotalADCs, fNbOfTimeSamples,
                                         fStopTimeSamples, fStartTimeSamples);
                fListOfHits[hitID] = hit;
            }

            //    if (fAPVID % 2 == 1) data = 1.2 * data ;

            fListOfHits[hitID]->AddTimeBinADCs(timebin, data, fPedestalNoises[chNo]);

            // ZERO SUPPRESSION
            if (avgdata > (fZeroSupCut * fPedestalNoises[chNo])) {
                if (!fListOfHitsClean[hitID]) {
                    GEMHit *hit = new GEMHit(hitID, fAPVID, chNo, fZeroSupCut, fIsHitMaxOrTotalADCs, fNbOfTimeSamples,
                                             fStopTimeSamples, fStartTimeSamples);
                    fListOfHitsClean[hitID] = hit;
                }
                fListOfHitsClean[hitID]->AddTimeBinADCs(timebin, data, fPedestalNoises[chNo]);
            }
        }
        firstdata = lastdata + 12;
        lastdata = firstdata + NCH;
        rawDataTS.clear();
    }

    rawDataTS.clear();
    rawDataTS_1stSet.clear();
    rawDataTS_2ndSet.clear();
    threshold_1stSet.clear();
    threshold_2ndSet.clear();
    dataTest_1stSet.clear();
    dataTest_2ndSet.clear();

    rawDataTS.clear();
    commonModeOffsets_1stSet.clear();
    commonModeOffsets_2ndSet.clear();
}

//======================================================
void GEMOnlineHitDecoder::GetListOfHitsFromPlanes() {
    map<Int_t, GEMHit *>::const_iterator hit_itr;
    for (hit_itr = fListOfHits.begin(); hit_itr != fListOfHits.end(); ++hit_itr) {
        GEMHit *hit = (*hit_itr).second;
        TString planename = hit->GetPlane();
        fListOfHitsFromPlane[planename].push_back(hit);
    }
}

//======================================================
void GEMOnlineHitDecoder::GetListOfHitsCleanFromPlanes() {
    map<Int_t, GEMHit *>::const_iterator hit_itr;
    for (hit_itr = fListOfHitsClean.begin(); hit_itr != fListOfHitsClean.end(); ++hit_itr) {
        GEMHit *hit = (*hit_itr).second;
        TString planename = hit->GetPlane();
        fListOfHitsCleanFromPlane[planename].push_back(hit);
    }
}

//======================================================
void GEMOnlineHitDecoder::GetHit(TString plane, TH1F *theHist) {
    // printf(" Enter  GEMHitDecoder::GetHit()\n") ;
    list<GEMHit *> hitList = fListOfHitsFromPlane[plane];
    list<GEMHit *>::iterator hit_it;
    for (hit_it = hitList.begin(); hit_it != hitList.end(); ++hit_it) {
        theHist->Fill((*hit_it)->GetStripNo(), (*hit_it)->GetHitADCs());
    }
    hitList.clear();
}

//======================================================
void GEMOnlineHitDecoder::FillHitHistos(TString plane, TH1F *posHist, TH1F *adcUniHist) {
    list<GEMHit *> hitList = fListOfHitsCleanFromPlane[plane];
    list<GEMHit *>::iterator hit_it;
    for (hit_it = hitList.begin(); hit_it != hitList.end(); ++hit_it) {
        if (!(*hit_it)->IsCleanHit()) continue;
        Float_t pos = (*hit_it)->GetStripPosition();
        Float_t adc = (*hit_it)->GetHitADCs();
        if (adc == 0) continue;
        AverageADCgain(posHist, adcUniHist, pos, adc);
    }
    for (auto &&hit: hitList) {
        delete hit;
    }
    hitList.clear();
}


//=========================================================================================================================
void GEMOnlineHitDecoder::AverageADCgain(TH1F *posHist, TH1F *adcUniHist, Float_t pos, Float_t adc) {
    adcUniHist->Multiply(posHist);
    posHist->Fill(pos);
    adcUniHist->Fill(pos, adc);
    adcUniHist->Divide(posHist);
}

/*****************************************************************************
 * **                    Compute Clusters Information                     ** *
 *****************************************************************************/

//============================================================================================
static Bool_t CompareStripNo(TObject *obj1, TObject *obj2) {
    Bool_t compare;
    if (((GEMHit *) obj1)->GetStripNo() < ((GEMHit *) obj2)->GetStripNo()) compare = kTRUE;
    else compare = kFALSE;
    return compare;
}

//============================================================================================
static Bool_t CompareClusterADCs(TObject *obj1, TObject *obj2) {
    Bool_t compare;
    if (((GEMCluster *) obj1)->GetClusterADCs() > ((GEMCluster *) obj2)->GetClusterADCs()) compare = kTRUE;
    else compare = kFALSE;
    return compare;
}

//============================================================================================
void GEMOnlineHitDecoder::ComputeClusters() {
    // printf("==GEMOnlineHitDecoder::ComputeClusters() \n") ;
    map<TString, list<GEMHit *> >::const_iterator hitsFromPlane_itr;
    for (hitsFromPlane_itr = fListOfHitsCleanFromPlane.begin();
         hitsFromPlane_itr != fListOfHitsCleanFromPlane.end(); ++hitsFromPlane_itr) {
        TString plane = (*hitsFromPlane_itr).first;
        list<GEMHit *> hitsFromPlane = (*hitsFromPlane_itr).second;
        hitsFromPlane.sort(CompareStripNo);
        Int_t listSize = hitsFromPlane.size();
        if (listSize < fMinClusterSize) {
            fIsGoodClusterEvent = kFALSE;
            continue;
        }
        Int_t previousStrip = -2;
        Int_t clusterNo = -1;
        map<Int_t, GEMCluster *> clustersMap;
        list<GEMHit *>::const_iterator hit_itr;
        for (hit_itr = hitsFromPlane.begin(); hit_itr != hitsFromPlane.end(); hit_itr++) {
            GEMHit *hit = *hit_itr;
            Int_t currentStrip = hit->GetStripNo();
            if (currentStrip < 0) continue; // for special 0~16 strips, just remove them.
            if (currentStrip != (previousStrip + 1)) clusterNo++;
            if (!clustersMap[clusterNo]) {
                clustersMap[clusterNo] = new GEMCluster(fEventNb, clusterNo, fMinClusterSize, fMaxClusterSize,
                                                        fIsCentralOrAllStripsADCs, fMinADCvalue);
                clustersMap[clusterNo]->SetPlane(hit->GetPlane());
                clustersMap[clusterNo]->SetStopTimeSamples(fStopTimeSamples);
                clustersMap[clusterNo]->SetStartTimeSamples(fStartTimeSamples);
            }
            clustersMap[clusterNo]->AddHit(hit);
            previousStrip = currentStrip;
        }

        map<Int_t, GEMCluster *>::const_iterator cluster_itr;
        for (cluster_itr = clustersMap.begin(); cluster_itr != clustersMap.end(); cluster_itr++) {
            GEMCluster *cluster = (*cluster_itr).second;
            cluster->ComputeCluster();
            if (!cluster->IsGoodCluster()) {
                delete cluster;
                continue;
            }
            fListOfClustersCleanFromPlane[plane].push_back(cluster);
        }
        fListOfClustersCleanFromPlane[plane].sort(CompareClusterADCs);
        cout << "fListOfClustersCleanFromPlane " << plane << " size = " << fListOfClustersCleanFromPlane[plane].size()
             << endl;
        hitsFromPlane.clear();
        clustersMap.clear();
    }
}

//==================================================================================
void GEMOnlineHitDecoder::GetClusterHit(TString plane, TH1F *theHist) {
    //  printf(" Enter  GEMHitDecoder::GetClusterHit(), plane = %s \n", plane.Data()) ;
    list<GEMCluster *> listOfClusters = fListOfClustersCleanFromPlane[plane];
    Int_t nbOfAnalysedClusters = 1;
    list<GEMCluster *>::iterator cluster_it;
    for (cluster_it = listOfClusters.begin(); cluster_it != listOfClusters.end(); ++cluster_it) {
        if (nbOfAnalysedClusters > fMaxClusterMult) continue;
        Int_t nbOfHits = (*cluster_it)->GetNbOfHits();
        for (Int_t i = 0; i < nbOfHits; i++) {
            theHist->Fill((*cluster_it)->GetHit(i)->GetStripNo(), (*cluster_it)->GetHit(i)->GetHitADCs());
        }
        nbOfAnalysedClusters++;
    }
    listOfClusters.clear();
}
//==================================================================================

int GEMOnlineHitDecoder::GetClusters(TString plane, vector<SFclust> &clust) {
    printf(" Enter  GEMHitDecoder::GetClusters(), plane = %s \n", plane.Data());

    list<GEMCluster *> listOfClusters = fListOfClustersCleanFromPlane[plane];
    Int_t nbOfAnalysedClusters = 0;
    SFclust cl;
    list<GEMCluster *>::iterator cluster_it;
    for (cluster_it = listOfClusters.begin(); cluster_it != listOfClusters.end(); ++cluster_it) {
        if (nbOfAnalysedClusters > fMaxClusterMult) continue;
        Int_t nbOfHits = (*cluster_it)->GetNbOfHits();
        double pos = 0, E = 0, A = 0;
        int N = nbOfHits;
        for (Int_t i = 0; i < nbOfHits; i++) {
            //theHist -> Fill( (* cluster_it)->GetHit(i)->GetStripNo(), (* cluster_it)->GetHit(i)->GetHitADCs());
            pos += (*cluster_it)->GetHit(i)->GetStripNo() * (*cluster_it)->GetHit(i)->GetHitADCs();
            E += (*cluster_it)->GetHit(i)->GetHitADCs();
            if (A < (*cluster_it)->GetHit(i)->GetHitADCs()) A = (*cluster_it)->GetHit(i)->GetHitADCs();
        }
        pos /= E;
        cl.x = pos;
        cl.y = 0;
        cl.A = A;
        cl.E = E;
        cl.N = N;
        clust.push_back(cl);
        nbOfAnalysedClusters++;
    }

    listOfClusters.clear();
    return nbOfAnalysedClusters;
}

//======================================================
void GEMOnlineHitDecoder::GetTimeBinClusterHit(TString plane, TH2F *the2dHist) {
    //  printf(" Enter  GEMHitDecoder::GetTimeBinClusterHit()\n") ;
    list<GEMCluster *> listOfClusters = fListOfClustersCleanFromPlane[plane];
    list<GEMCluster *>::iterator cluster_it;
    Int_t nbOfAnalysedClusters = 1;
    for (cluster_it = listOfClusters.begin(); cluster_it != listOfClusters.end(); ++cluster_it) {
        if (nbOfAnalysedClusters > fMaxClusterMult) continue;
        Int_t nbOfHits = (*cluster_it)->GetNbOfHits();
        for (Int_t i = 0; i < nbOfHits; i++) {
            map<Int_t, Float_t> timeBinADCs = (*cluster_it)->GetHit(i)->GetTimeBinADCs();
            map<Int_t, Float_t>::iterator timebin_it;
            for (timebin_it = timeBinADCs.begin(); timebin_it != timeBinADCs.end(); ++timebin_it) {
                the2dHist->Fill((25 * (*timebin_it).first), (*cluster_it)->GetHit(i)->GetStripNo(),
                                (*timebin_it).second);
            }
        }
        nbOfAnalysedClusters++;
    }
    listOfClusters.clear();
}

//======================================================
void
GEMOnlineHitDecoder::FillADCvsDriftTimeAndPositionForLargestCluster(TString plane, TH2F *the2dHist, TH2F *adcHist) {
    //  printf(" Enter  GEMHitDecoder::FillADCvsDriftTimeAndPositionForLargestCluster()\n") ;
    list<GEMCluster *> listOfClusters = fListOfClustersCleanFromPlane[plane];
    Int_t size = listOfClusters.size();
    if (size < 1) return;
    GEMCluster *cluster = listOfClusters.front();
    Int_t timebin = cluster->GetClusterPeakTimeBin();
    Int_t strip = cluster->GetClusterCentralStripNb();
    Float_t adc = cluster->GetClusterADCs();
    the2dHist->Fill(25 * timebin, strip);
    adcHist->Fill(25 * timebin, strip, adc);
    //  printf(" Enter  GEMHitDecoder::FillADCvsDriftTimeAndPositionForLargestCluster() adc = %f \n\n", adc) ;
    listOfClusters.clear();
}

//======================================================
void GEMOnlineHitDecoder::FillADCvsDriftTimeAndPositionForAllClusters(TString plane, TH2F *the2dHist, TH2F *adcHist) {
    //  printf(" Enter  GEMHitDecoder::FillADCvsDriftTimeAndPositionForAllClusters()\n") ;
    list<GEMCluster *> listOfClusters = fListOfClustersCleanFromPlane[plane];
    list<GEMCluster *>::iterator cluster_it;
    for (cluster_it = listOfClusters.begin(); cluster_it != listOfClusters.end(); ++cluster_it) {
        Int_t timebin = (*cluster_it)->GetClusterPeakTimeBin();
        Int_t strip = (*cluster_it)->GetClusterCentralStripNb();
        Float_t adc = (*cluster_it)->GetClusterADCs();
        //    printf(" Enter  GEMHitDecoder::FillADCvsDriftTimeAndPositionForAllClusters() adc = %f \n\n", adc) ;
        the2dHist->Fill(25 * timebin, strip);
        adcHist->Fill(25 * timebin, strip, adc);
    }
    listOfClusters.clear();
}

//======================================================
void GEMOnlineHitDecoder::FillClusterHistos(TString plane, TH1F *posHist, TH1F *adcUniHist, TH1F *adcDistHist,
                                            TH1F *clusterSizeHist, TH1F *clusterMultHist) {
    list<GEMCluster *> clusterList = fListOfClustersCleanFromPlane[plane];
    int clustMult = clusterList.size();
    if (clustMult > 0) clusterMultHist->Fill(clustMult);
    //  printf(" Enter  GEMOnlineHitDecoder::FillClusterHistos() => plane = %s, clustMult=%d \n", plane.Data(), clustMult) ;
    Int_t nbOfAnalysedClusters = 1;
    list<GEMCluster *>::iterator cluster_it;
    for (cluster_it = clusterList.begin(); cluster_it != clusterList.end(); ++cluster_it) {
        if (nbOfAnalysedClusters > fMaxClusterMult) continue;
        AverageADCgain(posHist, adcUniHist, (*cluster_it)->GetClusterPosition(), (*cluster_it)->GetClusterADCs());
        clusterSizeHist->Fill((*cluster_it)->GetNbOfHits());
        adcDistHist->Fill((*cluster_it)->GetClusterADCs());
        nbOfAnalysedClusters++;
    }
    clusterList.clear();
}

//=========================================================================================================================
void GEMOnlineHitDecoder::FillPos2D(TH2F *posHist, TH2F *adcUniHist, TH2F *timeSampleHist, Float_t xpos, Float_t ypos,
                                    Float_t adc, Float_t timing) {
    //  printf(" Enter  GEMOnlineHitDecoder::FillPos2D \n") ;
    posHist->Fill(xpos, ypos);
    adcUniHist->Fill(xpos, ypos, adc);
    timeSampleHist->Fill(xpos, ypos, timing);
}

//====================================================================================
void GEMOnlineHitDecoder::Fill2DClusterHistos(TString detector, TH2F *pos2DHist, TH2F *adc2DHist, TH2F *timeSampleHist,
                                              TH2F *chargeSharingHist, TH1F *chargeRatioHist) {
    // printf(" Enter  GEMOnlineHitDecoder::Fill2DClusterHistos() \n") ;

    list<GEMCluster *> listOfClustersX = fListOfClustersCleanFromPlane[fMapping->GetPlaneListFromDetector(detector.Data()).front()];
    list<GEMCluster *> listOfClustersY = fListOfClustersCleanFromPlane[fMapping->GetPlaneListFromDetector(detector.Data()).back()];

    Int_t clusterMultiplicityX = listOfClustersX.size();
    Int_t clusterMultiplicityY = listOfClustersY.size();
    if ((clusterMultiplicityY == 0) || (clusterMultiplicityX == 0)) return;
    Int_t clusterMult = (clusterMultiplicityX < clusterMultiplicityY) ? clusterMultiplicityX : clusterMultiplicityY;
    if (clusterMult > fMaxClusterMult) clusterMult = fMaxClusterMult;

    TList *clusterListX, *clusterListY;
    clusterListX = new TList;
    clusterListY = new TList;

    list<GEMCluster *>::const_iterator clusterX_itr;
    list<GEMCluster *>::const_iterator clusterY_itr;
    for (clusterX_itr = listOfClustersX.begin(); clusterX_itr != listOfClustersX.end(); ++clusterX_itr)
        clusterListX->Add(*clusterX_itr);
    for (clusterY_itr = listOfClustersY.begin(); clusterY_itr != listOfClustersY.end(); ++clusterY_itr)
        clusterListY->Add(*clusterY_itr);

    for (Int_t k = 0; k < clusterMult; k++) {
        Float_t xpos = ((GEMCluster *) clusterListX->At(k))->GetClusterPosition();
        Float_t ypos = ((GEMCluster *) clusterListY->At(k))->GetClusterPosition();
        Float_t adcCount1 = ((GEMCluster *) clusterListX->At(k))->GetClusterADCs();
        Float_t adcCount2 = ((GEMCluster *) clusterListY->At(k))->GetClusterADCs();
        Float_t timing1 = ((GEMCluster *) clusterListX->At(k))->GetClusterPeakTimeBin();
        Float_t timing2 = ((GEMCluster *) clusterListY->At(k))->GetClusterPeakTimeBin();
        xpos = fMapping->GetPlaneOrientation(fMapping->GetPlaneListFromDetector(detector.Data()).front()) * xpos;
        ypos = fMapping->GetPlaneOrientation(fMapping->GetPlaneListFromDetector(detector.Data()).back()) * ypos;

        Float_t clusterPos1 = xpos;
        Float_t clusterPos2 = ypos;

        if (fMapping->GetReadoutBoardFromDetector(detector.Data()) == "UV_ANGLE") {
            Float_t trapezoidDetLength = fMapping->GetUVangleReadoutMap(detector.Data())[0];
            Float_t trapezoidDetInnerRadius = fMapping->GetUVangleReadoutMap(detector.Data())[1];
            Float_t trapezoidDetOuterRadius = fMapping->GetUVangleReadoutMap(detector.Data())[2];
            Float_t uvAngleCosineDirection =
                    (trapezoidDetOuterRadius - trapezoidDetInnerRadius) / (2 * trapezoidDetLength);
            xpos = 0.5 * (trapezoidDetLength + ((clusterPos1 - clusterPos2) / uvAngleCosineDirection));
            ypos = 0.5 * (clusterPos1 + clusterPos2);
        }

        if ((adcCount1 == 0) || (adcCount2 == 0)) {
            fListOfClustersCleanFromPlane[(fMapping->GetPlaneListFromDetector(detector.Data())).front()].clear();
            fListOfClustersCleanFromPlane[(fMapping->GetPlaneListFromDetector(detector.Data())).back()].clear();
            continue;
        }

        Float_t adc = 0.5 * (adcCount1 + adcCount2);
        Float_t timing = 0.5 * (timing1 + timing2);

        FillPos2D(pos2DHist, adc2DHist, timeSampleHist, xpos, ypos, adc, timing);
        chargeSharingHist->Fill(adcCount1, adcCount2);
        chargeRatioHist->Fill(adcCount1 / adcCount2);
    }

    clusterListX->Clear();
    clusterListY->Clear();
    delete clusterListX;
    delete clusterListY;

    listOfClustersX.clear();
    listOfClustersY.clear();
}

