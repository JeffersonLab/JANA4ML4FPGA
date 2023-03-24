/************************************************
 *
 * Xinzhan Bai 
 * xb4zp@virginia.edu
 ************************************************/

#include <arpa/inet.h>
#include <assert.h>
#include <utility>
#include "GEMRawDecoder.h"
#include "GemMapping.h"
#include <fstream>
#include <iostream>
#include <utility>
#include <TH1F.h>

#define APV_MIN_LENGTH 64

using namespace std;

GEMRawDecoder::GEMRawDecoder() {
  mapping = GemMapping::GetInstance();
  mAPVRawSingleEvent.clear();
  mAPVRawHisto.clear();
  vActiveAdcChannels.clear();
}

GEMRawDecoder::GEMRawDecoder(unsigned int * buffer, int n) { 
  mapping = GemMapping::GetInstance();
  mAPVRawSingleEvent.clear();
  mAPVRawHisto.clear();
  vActiveAdcChannels.clear();
  Decode(buffer, n);
}

GEMRawDecoder::GEMRawDecoder(vector<int>  buffer) {
  printf("GEMRawDecoder::GEMRawDecoder(vector<int>  buffer) vec size = %d \n",buffer.size());
  mapping = GemMapping::GetInstance();
  mAPVRawSingleEvent.clear();
  mAPVRawHisto.clear();
  vActiveAdcChannels.clear();
  Decode(buffer);
}

GEMRawDecoder::~GEMRawDecoder() {
  Clear();
}

void GEMRawDecoder::Clear() {
  //clear maps
  clearMaps();
  clearFECMaps();
  //clear APV raw histos
  clearHistos();
  clearFECHistos();
  vActiveAdcChannels.clear();
}

void GEMRawDecoder::clearHistos() {
  //clear APV raw histos
  map<int, map<int, TH1F*> >::iterator it_apv;
  for(it_apv=mAPVRawHisto.begin(); it_apv!=mAPVRawHisto.end(); ++it_apv) {
    map<int, TH1F*> temp = it_apv->second;
    map<int, TH1F*>::iterator itt_apv;

    for(itt_apv=temp.begin(); itt_apv!=temp.end(); ++itt_apv) {
      TH1F *h = itt_apv->second;
      h->Delete();
    }
    temp.clear();
  }
  mAPVRawHisto.clear();
}

void GEMRawDecoder::clearFECHistos() {
  for(auto &i: mFecApvHisto) i.second->Delete();
  mFecApvHisto.clear();
}

void GEMRawDecoder::clearMaps() {
  map<int, map<int, vector<int> > >::iterator it;
  for(it=mAPVRawSingleEvent.begin(); it!=mAPVRawSingleEvent.end(); ++it) {
    map<int, vector<int> > temp = it->second;
    map<int, vector<int> >::iterator it1;
    for(it1=temp.begin(); it1 != temp.end(); ++it1) it1->second.clear();
    it->second.clear();
  }
  mAPVRawSingleEvent.clear();
}

void GEMRawDecoder::clearFECMaps() {
  for(auto &i: mFecApvEvent) i.second.clear();
  mFecApvEvent.clear();
}

void GEMRawDecoder::SwitchEndianess(unsigned int *buf, int &n) {
  int size = n;
  for(int i =0; i<size; ++i) buf[i] = ntohl(buf[i]); 
}

void GEMRawDecoder::Decode( vector<int> buffer) {
  fBufferSize = buffer.size();
  unsigned int *buf;
  buf = new unsigned int[fBufferSize];
  for(int i=0;i< fBufferSize;i++) buf[i] = (unsigned int) buffer[i];
  Decode(buf, fBufferSize);
  delete[] buf;
}

void GEMRawDecoder::Decode( unsigned int * buf, int n) {
    //cout << "GEMRawDecoder::Decode( unsigned int * buf, int n)  enter n=" << n  << endl;
  clearMaps();
  fBufferSize = n;
  if(fBufferSize <= 0) return;
  // fsv SwitchEndianess(buf, fBufferSize);

  int channel_active = 1;
  vector<int> apv;
  apv.clear();
  for(int idata = 0;idata<fBufferSize;idata++) {
      //cout << "buf[idata] 0x" << std::hex << buf[idata] << endl;
    if( ( (buf[idata+1]>>8) & 0xffffff ) == 0x414443 ) {
      //cout << "buf[idata+1] 0x" << std::hex << buf[idata+1] << std::dec << endl;
      if(apv.size() > APV_MIN_LENGTH && channel_active==1) mAPVRawSingleEvent[nfecID][nadcCh] = apv  ;

      apv.clear();
      nadcCh = buf[idata+1] & 0xff; 
      nfecID = (buf[idata+2]>>16) & 0xff;
      idata+=2;
      channel_active = 1;
      if( ! IsAdcChannelActive(nfecID, nadcCh) ) {
         channel_active = 0;
         CheckInactiveChannel(idata+3, buf);
      }
      //cout << "nfecID= " << nfecID << " nadcCh= " << nadcCh << " channel_active= " << channel_active << endl;
    }

    else if( buf[idata+1] == 0xfafafafa ) {
       //cout << "buf[idata+1] 0x" << std::hex << buf[idata+1] << std::dec << " apv size= " << apv.size() << endl;
      if(channel_active) {
	FillAPVRaw(apv, buf[idata]);
    //cout << "apv size= " << apv.size() << endl;
    if(apv.size() > APV_MIN_LENGTH ) mAPVRawSingleEvent[nfecID][nadcCh] = apv;
	apv.clear();
      }
      idata+=1;
    }
    else {
      if(channel_active) FillAPVRaw(apv, buf[idata]);
        //cout << "apv last = " << apv.at(apv.size()-1) << endl;
    }
  }
}

void GEMRawDecoder::DecodeFEC( vector<int> buffer) {
  fBufferSize = buffer.size();
  unsigned int *buf;
  buf = new unsigned int[fBufferSize];
  for(int i=0;i<fBufferSize;i++) buf[i] =(unsigned int) buffer[i];    
  DecodeFEC(buf,fBufferSize);
  delete[] buf;
}

void GEMRawDecoder::DecodeFEC( unsigned int * buf, int n) {
    //cout << "GEMRawDecoder::DecodeFEC( unsigned int * buf, int n) enter n=" << n << endl;
  clearFECMaps();
  fBufferSize = n;
  if(fBufferSize <= 0) return;
  // fsv  !!!  SwitchEndianess(buf, fBufferSize);

  int channel_active = 1;
  vector<int> apv;
  apv.clear();
  for(int idata = 0;idata<fBufferSize;idata++) {          
    //    if (idata ==0) cout<<"buf: "<<fBufferSize<<endl;
    //    if (idata ==0)  printf("       == GEMRawDecoder::DecodeFEC() => bufferSize = %d  \n", fBufferSize) ;
    if( ( (buf[idata+1]>>8) & 0xffffff ) == 0x414443 ) {
      if(apv.size() > APV_MIN_LENGTH && channel_active==1) mFecApvEvent[apvIndex] = apv;
      apv.clear();
      nadcCh = buf[idata+1] & 0xff; 
      nfecID = (buf[idata+2]>>16) & 0xff;
      apvIndex = (nfecID<<4) | nadcCh;
      idata+=2;
      channel_active = 1;
      if( ! IsAdcChannelActive(nfecID, nadcCh) ) {
	channel_active = 0;
	CheckInactiveChannel(idata+3, buf);
      }
    }
    else if( buf[idata+1] == 0xfafafafa ) {
      if(channel_active) {
	FillAPVRaw(apv, buf[idata]);
	if(apv.size() > APV_MIN_LENGTH ) mFecApvEvent[apvIndex] = apv;
	apv.clear();
      }
      idata+=1;
    }
    else {
      if(channel_active) FillAPVRaw(apv, buf[idata]);
    }
  }
}

void GEMRawDecoder::CheckInactiveChannel(int offset, unsigned int * apv_event) {
  //check first 100 words, to make sure no data lost in case using the wrong mapping 
  //by checking if this channel has APV header                                       
  int header = 1500;

  for(int i=offset;i<100;i++) {
    if(apv_event[i] < header) {
      i++;
      if(apv_event[i] < header) {
	i++;
	if(apv_event[i] < header ) {
	  i++;
	  cout<<"## Deocder WARNNING: ##  Found meaningful data in INACTIVE channels..."
	      <<endl;
	  cout<<"##  FEC: "<<nfecID<<"  Channel:  "<<nadcCh<<endl;
	  cout<<"##  check you mapping file, or check the raw data..."<<endl;
	  break;
	}
      }
    }
  }
}

void GEMRawDecoder::FillAPVRaw(vector<int>  & vec, unsigned int word) {
  unsigned int word32bit;
  unsigned int word16bit1;
  unsigned int word16bit2;

  word32bit = word;
  Word32ToWord16(&word32bit, &word16bit1, &word16bit2);
  vec.push_back(word16bit1);
  vec.push_back(word16bit2);
}

map<int, map<int, vector<int> > > GEMRawDecoder::GetDecoded() {
  return mAPVRawSingleEvent;
}

map<int, vector<int> > GEMRawDecoder::GetFECDecoded() {
  return mFecApvEvent;
}

map<int, map<int, TH1F* > >  GEMRawDecoder::GetAPVRawHisto() {
  clearHistos();
  int fec_id=0;
  int adc_ch=0;
  //  cout<<"GEMRawDecoder::GetAPVRawHisto()fec: adc: "<<endl;

  map<int, TH1F*> ch_apv;
  map<int, map<int, vector<int> > >::iterator it;
  for(it = mAPVRawSingleEvent.begin(); it!=mAPVRawSingleEvent.end(); ++it) {
    fec_id = it->first;
    map<int, vector<int> > temp = it->second; 

    map<int, vector<int> >::iterator itt;
    for(itt=temp.begin(); itt!=temp.end(); ++itt) {
      adc_ch = itt->first;
      vector<int> adc_temp = itt->second;

      int N = adc_temp.size();
      TH1F* h = new TH1F(Form("fec_%d_ch_%d",fec_id, adc_ch), Form("fec_%d_ch_%d_raw_data",fec_id, adc_ch), N+128, 0, N+127);
      for(int i=0;i<N;i++) {
        //cout<<"GEMRawDecoder::GetAPVRawHisto()fec: " << fec_id << " adc: " << adc_ch <<  " val= " <<  adc_temp[i] << endl;
        h->Fill(i+1, (Float_t) adc_temp[i]);
      }
      mAPVRawHisto[fec_id][adc_ch] = h;
    }
  }
  return mAPVRawHisto;
}

map<int, TH1F* >  GEMRawDecoder::GetFECAPVRawHisto() {
  clearFECHistos();
  int index=0;
  int fec_id = 0;
  int adc_ch = 0;

  for(auto &i: mFecApvEvent)    {
    index = i.first;
    //	cout<<" GEMRawDecoder::GetFECAPVRawHisto() => histo index:"<<index<<endl;

    adc_ch = index & 0xf;
    fec_id = (index>>4)&0xf;
    int N = i.second.size();
    TH1F* h = new TH1F(Form("fec_%d_ch_%d",fec_id, adc_ch), Form("fec_%d_ch_%d_raw_data",fec_id, adc_ch), N+128, 0, N+127);
    int bin = 1;	
    for(auto &j: i.second) h->Fill( bin++, j);
    mFecApvHisto[index] = h;
  }
  return mFecApvHisto;
}

void GEMRawDecoder::Word32ToWord16(unsigned int *word, unsigned int *word1, unsigned int *word2) {
  unsigned int data1=0;
  unsigned int data2=0;
  unsigned int data3=0;
  unsigned int data4=0;

  data1 = ( (*word)>>24 ) & 0xff;
  data2 = ( (*word)>>16 ) & 0xff;
  data3 = ( (*word)>>8  ) & 0xff;
  data4 = (*word) & 0xff;


  (*word1) = (data2 << 8) | data1;
  (*word2) = (data4 << 8) | data3;
  //fsv
  //(*word1) = ( (*word)>>16 ) & 0xffff;
  //(*word2) =   (*word)       & 0xffff;
  //cout << "Word32ToWord16():: w=0x" << std::hex << (*word) << std::dec << " w1=" << (*word1) << " w2=" << (*word2) << endl;
}
int GEMRawDecoder::IsAdcChannelActive( int &fecid ,  int &ch) {
  int isActive = 0;
  vActiveAdcChannels = mapping->GetActiveADCchannels(fecid);
  if( find( vActiveAdcChannels.begin(), vActiveAdcChannels.end(), ch) != vActiveAdcChannels.end() ) 
    isActive =1;    
    return isActive;
}
