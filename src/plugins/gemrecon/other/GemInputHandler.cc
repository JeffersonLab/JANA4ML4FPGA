#include "hardcode.h"
#include "GemInputHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>
#include <time.h>
#include <pthread.h>
#include "GemView.h"
#include "GEMOnlineHitDecoder.h"
#include "TH1F.h"

#define HEADER_SIZE 2

#define BANKID 17

//===================================================
GemInputHandler::GemInputHandler(TCanvas *c) {
    c3 = c;
    fSrsStart = 0xda000022;
    fSrsEnd = 0xda0000ff;
    fMapping = GemMapping::GetInstance();
    fEventNumber = -1;
    fLastEvent = 0;
    fMinADCvalue = 50;
    fZeroSupCut = 10;
    fComModeCut = 20;
    fNbOfTimeSamples = 9;
    fStartTimeSample = 2;
    fStopTimeSample = 7;
    fMinClusterSize = 2;
    fMaxClusterSize = 20;
    fMaxClusterMult = 5;
    fIsHitPeakOrSumADCs = "peakADCs";
    fIsCentralOrAllStripsADCs = "centralStripADCs";
    printf("GemInputHandler::GemInputHandler() enter \n");
}


//===================================================
GemInputHandler::~GemInputHandler() {
    Int_t size = fDetectorClusterMap.size();
    for (Int_t a = 0; a < size; a++) {
        map<Int_t, vector<Float_t> > map = fDetectorClusterMap[a];
        Int_t size2 = map.size();
        for (Int_t b = 0; b < size2; b++) map[b].clear();
        map.clear();
    }
    fDetectorClusterMap.clear();

    if (fBuffer != NULL)
        delete[](fBuffer), fBuffer = NULL;

    // force close ET
    //if(etID != NULL) et_forcedclose(etID);
}

//===================================================================
inline int GemInputHandler::parseEventByHeader(PRadEventHeader *header) {

    //cout << "header->tag= " << header->tag << endl;
    switch (header->tag) {
        case PhysicsType1:
        case PhysicsType2:
        case PhysicsGEMType:
        default:
            break; // go on to process
        case PreStartEvent:
        case GoEvent:
        case EndEvent:
            return -1; // not interested event type
    }

    const uint32_t *buffer = (const uint32_t *) header;
    size_t evtSize = header->length;
    size_t dataSize = 0;
    size_t index = 0;
    index += HEADER_SIZE; // skip event header

    while (index < evtSize) {
        PRadEventHeader *evtHeader = (PRadEventHeader *) &buffer[index];
        dataSize = evtHeader->length - 1;
        index += HEADER_SIZE; // header info is read

        // check the header, skip uninterested ones
        switch (evtHeader->type) {
            case EvioBank: // Bank type header for ROC
            case EvioBank_B:
                switch (evtHeader->tag) {
                    case PRadGEMROC_1:
                        continue;
                    case PRadROC_6: // PRIMEXROC6
                    case PRadROC_5: // PRIMEXROC5
                    case PRadROC_4: // PRIMEXROC4
                    case PRadTS: // Not interested in ROC 1, PRIMEXTS2
                    default: // unrecognized ROC
                        // Skip the whole segment
                        break;
                }
                break;
            case UnsignedInt32bit: // uint32 data bank
                switch (evtHeader->tag) {
                    default:
                        break;
                    case EVINFO_BANK: // Bank contains the event information
                        //eventNb = buffer[index];
                        break;
                    case TI_BANK: // Bank 0x4, TI data, not interested
                        // skip the whole segment
                        break;
                    case FASTBUS_BANK: // Bank 0x7, Fastbus data
                        break;
                    case GEMMONITOR_BANK_1: //return 1 if the buffer contain as least one bank for the online monitor
                    case GEMMONITOR_BANK_2:
                    case GEMMONITOR_BANK_3:
                    case GEMMONITOR_BANK_4:
                    case GEMMONITOR_BANK_5:
                    case GEMMONITOR_BANK_6:
                    case GEMMONITOR_BANK_7:
                    case GEMMONITOR_BANK_8:
                        if (((int) evtHeader->num) < 13 && ((int) evtHeader->num) > 4) {
                            return 1;
                        }
                        break;
                }
                break;

            default:
                // Unknown header
                break;
        }
        index += dataSize; // Data are either processed or skipped above
    }
    return 0;
}


//====================================================
int GemInputHandler::ProcessRawDataFromFile(char *filename, int iEntry) {
    fEventNumber = -1;
    //printf("\n GemInputHandler::ProcessRawDataFromFile() => Event Number %d, file=%s \n", iEntry,filename) ;

    vector<int> vSRSSingleEventData;
    try {
        evioFileChannel chan(filename, "r");
        //printf("\n GemInputHandler::ProcessRawDataFromFile() => Try to open file=%s \n",filename) ;
        chan.open();
        //printf("\n GemInputHandler::ProcessRawDataFromFile() => Opened file=%s \n",filename) ;
        while (chan.read()) {
            fEventNumber++;
            //printf("\n GemInputHandler::ProcessRawDataFromFile() wait evt=%d  current=%d \n",iEntry,fEventNumber) ;
            if (fEventNumber != iEntry) continue;
            vSRSSingleEventData.clear();
            evioDOMTree event(chan);
            evioDOMNodeListP fecEventList = event.getNodeList(isLeaf());
            evioDOMNodeList::iterator iter;
            for (iter = fecEventList->begin(); iter != fecEventList->end(); ++iter) {
                if (((*iter)->tag == BANKID)) {
                    vector<uint32_t> *vec = (*iter)->getVector<uint32_t>();
                    if (vec != NULL && vec->size() != 0) {
                        vSRSSingleEventData.reserve(vSRSSingleEventData.size() + vec->size());
                        vSRSSingleEventData.insert(vSRSSingleEventData.end(), vec->begin(), vec->end());
                    } else printf("GemInputHandler::ProcessEvent() => No FEC founds \n");
                }
            }
            if (vSRSSingleEventData.size() == 0) continue; // if no srs event found, go to next event
            GEMRawDecoder raw_decoder(vSRSSingleEventData);
            raw_decoder.Decode(vSRSSingleEventData);
            fCurrentEvent.clear();
            fCurrentEvent = raw_decoder.GetDecoded();
            break;
        }
        chan.close();
        //printf("\n GemInputHandler::ProcessRawDataFromFile()  chan.close(); file=%s \n",filename) ;

    } catch (evioException e) {
        cerr << endl << e.toString() << endl << endl;
        exit(EXIT_FAILURE);
    }
    //exit(EXIT_SUCCESS);
    return 1;
}

//===========================================================================
int GemInputHandler::ProcessETEvent(TH1F **hist, TH2F **hist2d) {
    printf("\n GemInputHandler::ProcessETEvent() \n");
    if (fEventFound && fBuffer != NULL) {
        GEMRawDecoder raw_decoder(fBuffer, fBufferSize);
        fCurrentEvent.clear();
        fCurrentEvent = raw_decoder.GetDecoded();
        GEMOnlineHitDecoder hit_decoder(0, fNbOfTimeSamples, fStartTimeSample, fStopTimeSample, fZeroSupCut,
                                        fComModeCut, fIsHitPeakOrSumADCs, fIsCentralOrAllStripsADCs, fMinADCvalue,
                                        fMinClusterSize, fMaxClusterSize, fMaxClusterMult);
        hit_decoder.ProcessEvent(fCurrentEvent, fPedestal);
        map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
        map<TString, Int_t>::const_iterator plane_itr;
        for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
            Int_t i = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane((*plane_itr).first)) +
                      (*plane_itr).second;
            hist[i]->Reset("ICESM");
            hist2d[i]->Reset("ICESM");
            hit_decoder.GetHit((*plane_itr).first, hist[i]);
        }
        return 1;
    } else return 0;
}

//====================================================
//=======    SF  
//====================================================

int GemInputHandler::ProcessSingleEventFromBank(TH1F **hist, TH2F **hist2d, evioDOMNodeP bankPtr,
                                                vector<SFclust> &gemclust) {
    fEventNumber = -1;
    printf("\nGemInputHandler::ProcessSingleEventFromBank() => Enter  \n");
    vector<int> vSRSSingleEventData;

    //  vector<uint32_t> *vec = bankPtr->getVector<uint32_t>();

    vSRSSingleEventData.clear();
    //evioDOMTree event(chan);
    //evioDOMNodeListP fecEventList = event.getNodeList( isLeaf() );
    //evioDOMNodeList::iterator iter;

    vector<uint32_t> *vec = bankPtr->getVector<uint32_t>();
    if (vec != NULL && vec->size() != 0) {
        vSRSSingleEventData.reserve(vSRSSingleEventData.size() + vec->size());
        vSRSSingleEventData.insert(vSRSSingleEventData.end(), vec->begin(), vec->end());
    } else printf("GemInputHandler::ProcessEvent() => No FEC founds \n");

    if (vSRSSingleEventData.size() == 0) {
        printf("\nGemInputHandler::ProcessSingleEventFromBank() => No data.  return ...  \n");
        return 1; // if no srs event found, go to next event
    }
    GEMRawDecoder raw_decoder(vSRSSingleEventData);
    raw_decoder.Decode(vSRSSingleEventData);
    fCurrentEvent.clear();
    fCurrentEvent = raw_decoder.GetDecoded();
    GEMOnlineHitDecoder hit_decoder(1, fNbOfTimeSamples, fStartTimeSample, fStopTimeSample, fZeroSupCut, fComModeCut,
                                    fIsHitPeakOrSumADCs, fIsCentralOrAllStripsADCs, fMinADCvalue, fMinClusterSize,
                                    fMaxClusterSize, fMaxClusterMult);
    hit_decoder.ProcessEvent(fCurrentEvent, fPedestal);
    map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
    map<TString, Int_t>::const_iterator plane_itr;
    int pad = 0;
    int ncx = 0, ncy = 0;
    vector<SFclust> clustX, clustY;
    for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
        TString plane = (*plane_itr).first;
        Int_t i = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane((*plane_itr).first)) + (*plane_itr).second;
        Int_t j = fMapping->GetNbOfPlane() + i;
        // all channels hit
        hist[j]->Reset();
        hit_decoder.GetHit((*plane_itr).first, hist[j]);
        // Zero sup hit
        hist[i]->Reset();
        hit_decoder.GetClusterHit((*plane_itr).first, hist[i]);
        hist2d[i]->Reset();
        hit_decoder.GetTimeBinClusterHit((*plane_itr).first, hist2d[i]);

        if (plane == "GEM1X") {
            ncx = hit_decoder.GetClusters((*plane_itr).first, clustX);
        }
        if (plane == "GEM1Y") {
            ncy = hit_decoder.GetClusters((*plane_itr).first, clustY);
        }

        if (c3) {
            c3->cd(++pad);
            hist[j]->Draw();
            c3->cd(++pad);
            hist[i]->Draw();
            c3->cd(++pad);
            hist2d[i]->Draw();
            c3->Modified();
            c3->Update();
        }

    }
    cout << "------------------->>  cluster counter :: nx=" << ncx << " ny=" << ncy << endl;
    if (ncx == ncy) {
        printf(" OK nx=%d ny=%d \n", clustX.size(), clustY.size());
        for (int ic = 0; ic < ncx; ic++) {


            printf("cl=%d  x=%f  E=%f A=%f N=%d  \n", ic, clustX[ic].x, clustX[ic].E, clustX[ic].A, clustX[ic].N);
            printf("cl=%d  y=%f  E=%f A=%f N=%d  \n", ic, clustY[ic].x, clustY[ic].E, clustY[ic].A, clustY[ic].N);

            SFclust cl;
            cl.x = clustX[ic].x;
            cl.y = clustY[ic].x;
            cl.E = clustX[ic].E + clustY[ic].E;
            cl.A = clustX[ic].A + clustY[ic].A;
            gemclust.push_back(cl);

        }
    }

    // if (c3) sleep(1);

    return 1;
}

///====================================================
int GemInputHandler::ProcessSingleEventFromFile(char *filename, int iEntry, TH1F **hist, TH2F **hist2d) {
    fEventNumber = -1;
    printf("\nGemInputHandler::ProcessSingleEventFromFile() => Entries # %d,  \n", iEntry);
    vector<int> vSRSSingleEventData;
    try {
        evioFileChannel chan(filename, "r");
        chan.open();
        while (chan.read()) {
            fEventNumber++;
            if (fEventNumber != iEntry) continue;
            vSRSSingleEventData.clear();
            evioDOMTree event(chan);
            evioDOMNodeListP fecEventList = event.getNodeList(isLeaf());
            evioDOMNodeList::iterator iter;
            for (iter = fecEventList->begin(); iter != fecEventList->end(); ++iter) {
                if (((*iter)->tag == BANKID)) {
                    vector<uint32_t> *vec = (*iter)->getVector<uint32_t>();
                    if (vec != NULL && vec->size() != 0) {
                        vSRSSingleEventData.reserve(vSRSSingleEventData.size() + vec->size());
                        vSRSSingleEventData.insert(vSRSSingleEventData.end(), vec->begin(), vec->end());
                    } else printf("GemInputHandler::ProcessEvent() => No FEC founds \n");
                }
            }
            if (vSRSSingleEventData.size() == 0) continue; // if no srs event found, go to next event
            GEMRawDecoder raw_decoder(vSRSSingleEventData);
            raw_decoder.Decode(vSRSSingleEventData);
            fCurrentEvent.clear();
            fCurrentEvent = raw_decoder.GetDecoded();
            GEMOnlineHitDecoder hit_decoder(iEntry, fNbOfTimeSamples, fStartTimeSample, fStopTimeSample, fZeroSupCut,
                                            fComModeCut, fIsHitPeakOrSumADCs, fIsCentralOrAllStripsADCs, fMinADCvalue,
                                            fMinClusterSize, fMaxClusterSize, fMaxClusterMult);
            hit_decoder.ProcessEvent(fCurrentEvent, fPedestal);
            map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
            map<TString, Int_t>::const_iterator plane_itr;
            for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
                TString plane = (*plane_itr).first;
                Int_t i = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane((*plane_itr).first)) +
                          (*plane_itr).second;
                Int_t j = fMapping->GetNbOfPlane() + i;
                // all channels hit
                hist[j]->Reset();
                hit_decoder.GetHit((*plane_itr).first, hist[j]);
                // Zero sup hit
                hist[i]->Reset();
                hit_decoder.GetClusterHit((*plane_itr).first, hist[i]);
                hist2d[i]->Reset();
                hit_decoder.GetTimeBinClusterHit((*plane_itr).first, hist2d[i]);
            }
            break;
        }
        chan.close();
    } catch (evioException e) {
        cerr << endl << e.toString() << endl << endl;
        exit(EXIT_FAILURE);
    }
    //exit(EXIT_SUCCESS);
    return 1;
}

//====================================================
int GemInputHandler::ProcessSearchEventFromFile(char *filename, TH1F **hist, TH2F **hist2d) {
    fLastEvent = fEventNumber;
    fEventNumber = -1;
    Bool_t eventFound = kFALSE;
    printf("\nGemInputHandler::ProcessSearchEventFromFile() => Event Loop Start # %d \n", fLastEvent);

    vector<int> vSRSSingleEventData;
    try {
        evioFileChannel chan(filename, "r");
        chan.open();
        while (chan.read()) {
            fEventNumber++;
            if (fEventNumber < fLastEvent) continue;
            if (!eventFound) {
                vSRSSingleEventData.clear();
                evioDOMTree event(chan);
                evioDOMNodeListP fecEventList = event.getNodeList(isLeaf());
                evioDOMNodeList::iterator iter;
                for (iter = fecEventList->begin(); iter != fecEventList->end(); ++iter) {
                    if (((*iter)->tag == BANKID)) {
                        vector<uint32_t> *vec = (*iter)->getVector<uint32_t>();
                        if (vec != NULL && vec->size() != 0) {
                            vSRSSingleEventData.reserve(vSRSSingleEventData.size() + vec->size());
                            vSRSSingleEventData.insert(vSRSSingleEventData.end(), vec->begin(), vec->end());
                        } else printf("GemInputHandler::ProcessEvent() => No FEC founds \n");
                    }
                }
                if (vSRSSingleEventData.size() == 0) continue; // if no srs event found, go to next event
                GEMRawDecoder raw_decoder(vSRSSingleEventData);
                raw_decoder.Decode(vSRSSingleEventData);
                fCurrentEvent.clear();
                fCurrentEvent = raw_decoder.GetDecoded();
                GEMOnlineHitDecoder hit_decoder(fEventNumber, fNbOfTimeSamples, fStartTimeSample, fStopTimeSample,
                                                fZeroSupCut, fComModeCut, fIsHitPeakOrSumADCs,
                                                fIsCentralOrAllStripsADCs, fMinADCvalue, fMinClusterSize,
                                                fMaxClusterSize, fMaxClusterMult);
                hit_decoder.ProcessEvent(fCurrentEvent, fPedestal);
                if (hit_decoder.IsGoodEventFound()) {
                    printf("GemInputHandler::ProcessSearchEventFromFile() => Event of Interest # %d \n", fEventNumber);
                    eventFound = kTRUE;
                    fLastEvent = fEventNumber;
                    map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
                    map<TString, Int_t>::const_iterator plane_itr;
                    for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
                        TString plane = (*plane_itr).first;
                        Int_t i = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane((*plane_itr).first)) +
                                  (*plane_itr).second;
                        Int_t j = fMapping->GetNbOfPlane() + i;
                        // all channels hit
                        hist[j]->Reset();
                        hit_decoder.GetHit((*plane_itr).first, hist[j]);
                        // Zero sup hit
                        hist[i]->Reset();
                        hit_decoder.GetClusterHit((*plane_itr).first, hist[i]);
                        hist2d[i]->Reset();
                        hit_decoder.GetTimeBinClusterHit((*plane_itr).first, hist2d[i]);
                    }
                    fEventNumber++;
                    eventFound = kFALSE;
                    break;
                }
            }
            //      break;
        }
        chan.close();
    } catch (evioException e) {
        cerr << endl << e.toString() << endl << endl;
        exit(EXIT_FAILURE);
    }
    //exit(EXIT_SUCCESS);
    return 1;
}

//===========================================================================
int GemInputHandler::ProcessMultiEventsFromFile(TString fileName, unsigned int minEntry, unsigned int maxEntry,
                                                TH1F **adcDistHist, TH1F **hitHist, TH1F **clusterHist,
                                                TH1F **clusterInfoHist, TH2F **pos2DHist, TH2F **chargeSharingHist,
                                                TH1F **chargeRatioHist, TH2F **timeBinPosHist,
                                                TH2F **adcTimeBinPosHist) {

    //  printf("\nGemInputHandler::ProcessMultiEventsFromFile() => Event Loop Start # max=%d \n", maxEntry) ;

    if (fZeroSupCut < 1) return 0;

    int efficiencyCount = 0;
    //  fOfflineProgress = 0.;
    unsigned int fEventNumber = 0;
    vector<int> vSRSSingleEventData;

    map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
    map<TString, Int_t>::const_iterator plane_itr;

    try {
        evioFileChannel chan(fileName.Data(), "r");
        chan.open();

        while (fEventNumber < maxEntry && chan.read()) {
            fEventNumber++;
            if (fEventNumber < minEntry) continue;

            /**
            fOfflineProgress = 100.*(float)(fEventNumber-minEntry)/(float(maxEntry - minEntry));
            theBar->setValue(fOfflineProgress);
            theBar->update();
            */

            if ((fEventNumber - minEntry) % ((maxEntry - minEntry) / 10) == 0)
                printf("GemInputHandler::ProcessMultiEventsFromFile() =>  %d events have been processed \n",
                       fEventNumber);

            vSRSSingleEventData.clear();
            evioDOMTree event(chan);
            evioDOMNodeListP fecEventList = event.getNodeList(isLeaf());

            evioDOMNodeList::iterator iter;
            for (iter = fecEventList->begin(); iter != fecEventList->end(); ++iter) {
                //printf("GemInputHandler::ProcessMultiEventsFromFile() fecEventList tag = %d  \n", (*iter)->tag) ;
                if (((*iter)->tag == BANKID)) {
                    vector<uint32_t> *vec = (*iter)->getVector<uint32_t>();
                    if (vec != NULL && vec->size() != 0) {
                        vSRSSingleEventData.reserve(vSRSSingleEventData.size() + vec->size());
                        vSRSSingleEventData.insert(vSRSSingleEventData.end(), vec->begin(), vec->end());
                    } else printf("GemInputHandler::ProcessMultiEventsFromFile() => No FEC founds \n");
                }
            }
            //cout << "vSRSSingleEventData.size()=  " << vSRSSingleEventData.size() << endl;
            if (vSRSSingleEventData.size() == 0) continue;

            GEMRawDecoder raw_decoder(vSRSSingleEventData);
            raw_decoder.Decode(vSRSSingleEventData);
            fCurrentEvent.clear();
            fCurrentEvent = raw_decoder.GetDecoded();

            GEMOnlineHitDecoder hit_decoder(fEventNumber, fNbOfTimeSamples, fStartTimeSample, fStopTimeSample,
                                            fZeroSupCut, fComModeCut, fIsHitPeakOrSumADCs, fIsCentralOrAllStripsADCs,
                                            fMinADCvalue, fMinClusterSize, fMaxClusterSize, fMaxClusterMult);
            hit_decoder.ProcessEvent(fCurrentEvent, fPedestal);

            map<TString, Int_t> listOfPlanes = fMapping->GetPlaneIDFromPlaneMap();
            map<TString, Int_t>::const_iterator plane_itr;
            for (plane_itr = listOfPlanes.begin(); plane_itr != listOfPlanes.end(); ++plane_itr) {
                TString plane = ((*plane_itr).first);
                Int_t i = 2 * fMapping->GetDetectorID(fMapping->GetDetectorFromPlane((*plane_itr).first)) +
                          (*plane_itr).second;
                Int_t j = fMapping->GetNbOfPlane() + i;
                // Hit informations
                hit_decoder.FillHitHistos(plane, hitHist[i], hitHist[j]);
                hit_decoder.FillADCvsDriftTimeAndPositionForLargestCluster(plane, timeBinPosHist[j],
                                                                           adcTimeBinPosHist[j]);
                hit_decoder.FillADCvsDriftTimeAndPositionForAllClusters(plane, timeBinPosHist[i], adcTimeBinPosHist[i]);
                // Cluster informations
                hit_decoder.FillClusterHistos(plane, clusterHist[i], clusterHist[j], adcDistHist[i], clusterInfoHist[j],
                                              clusterInfoHist[i]);
            }
            map<Int_t, TString> listOfDetectors = fMapping->GetDetectorFromIDMap();
            map<Int_t, TString>::const_iterator det_itr;
            for (det_itr = listOfDetectors.begin(); det_itr != listOfDetectors.end(); ++det_itr) {
                int i = (*det_itr).first;
                TString det = (*det_itr).second;
                hit_decoder.Fill2DClusterHistos(det, pos2DHist[3 * i], pos2DHist[3 * i + 1], pos2DHist[3 * i + 2],
                                                chargeSharingHist[i], chargeRatioHist[i]);
            }
        }
        chan.close();
    } catch (evioException e) {
        cerr << endl << e.toString() << endl << endl;
        exit(EXIT_FAILURE);
    }
    //  exit(EXIT_SUCCESS);
    cout << efficiencyCount << endl;
    return 1;
}

//===========================================================================
int GemInputHandler::ProcessPedestals(TString dataFileName, unsigned int minEntry, unsigned int maxEntry) {
    //  fOfflineProgress = 0.;
    fEventNumber = 0;
    fLastEvent = 0;
    map<int, map<int, vector<int> > > theCurrentEvent;
    vector<int> vSRSSingleEventData;
    try {
        evioFileChannel chan(dataFileName.Data(), "r");
        chan.open();
        while (fEventNumber < ((int) maxEntry) && chan.read()) {
            if (fEventNumber < (int) minEntry) continue;
            /**
            fOfflineProgress = 100.*(float)(fEventNumber - minEntry)/(float(maxEntry  - minEntry));
            theBar->setValue(fOfflineProgress);
            theBar->update();
            */
            if ((fEventNumber - minEntry) % ((maxEntry - minEntry) / 10) == 0)
                printf("GemInputHandler::ProcessPedestals() =>  %d events have been processed \n", fEventNumber);

            vSRSSingleEventData.clear();
            evioDOMTree event(chan);
            evioDOMNodeListP fecEventList = event.getNodeList(isLeaf());
            evioDOMNodeList::iterator iter;
            for (iter = fecEventList->begin(); iter != fecEventList->end(); ++iter) {
                if (((*iter)->tag == BANKID)) {
                    vector<uint32_t> *vec = (*iter)->getVector<uint32_t>();
                    if (vec != NULL && vec->size() != 0) {
                        vSRSSingleEventData.reserve(vSRSSingleEventData.size() + vec->size());
                        vSRSSingleEventData.insert(vSRSSingleEventData.end(), vec->begin(), vec->end());
                    } else printf("GemInputHandler::ProcessPedestals() => No FEC founds \n");
                }
            }
            if (vSRSSingleEventData.size() == 0) continue; // if no srs event found, go to next event

            GEMRawDecoder raw_decoder(vSRSSingleEventData);
            raw_decoder.DecodeFEC(vSRSSingleEventData);
            map<int, vector<int> > fecEvent = raw_decoder.GetFECDecoded();
            if (newEvent()) {
                fPedestal->AccumulateEvent(fEventNumber, fGEMEvent);
                fGEMEvent.clear();
                fLastEvent = fEventNumber;
                fGEMEvent.insert(fecEvent.begin(), fecEvent.end());
            } else {
                fGEMEvent.insert(fecEvent.begin(), fecEvent.end());
                fLastEvent = fEventNumber;
            }

            fEventNumber++;
        }
        chan.close();
    } catch (evioException e) {
        cerr << endl << e.toString() << endl << endl;
        exit(EXIT_FAILURE);
    }
    fPedestal->SavePedestalFile();
    exit(EXIT_SUCCESS);
    return 1;
}

//===========================================================================
int GemInputHandler::newEvent() {
    if ((fEventNumber - (int) fLastEvent) == 1) return 1;
    else return 0;
}
