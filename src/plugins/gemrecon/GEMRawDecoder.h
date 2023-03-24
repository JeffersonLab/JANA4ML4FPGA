/*******************************************************************************************
 * Xinzhan Bai
 *
 * Usage:
 *
 * Input:  1), A buffer contains purely SRS data, and the buffer size in [int] unit
 *
 *         2), Or, SRS data filled in a vector<int>
 *
 * Output: 1), map<fec_id, map<ch_id, vector<int> > >, vector: adc values (all time samples)
 *                map<int, map<int, vector<int> > > GetDecoded();
 *
 *         2), map<fec_id, map<ch_id, TH1F* > > , TH1F*: adc values filled in histograms
 *             No need to worry memory leakage, this class owns every object it produced.
 *                map<int, map<int, TH1F* > > GetAPVRawHisto();
 * *****************************************************************************************/

#ifndef __RAW_DECODER_H__
#define __RAW_DECODER_H__

#include <map>
#include <unordered_map>
#include <vector>

class TH1F;
class GemMapping;

using namespace std;

class GEMRawDecoder
{
public:
  GEMRawDecoder();
  GEMRawDecoder( vector<int> );
  GEMRawDecoder( unsigned int *, int);
  ~GEMRawDecoder();

  void SwitchEndianess(unsigned int *, int &);
  void CheckInactiveChannel(int , unsigned int *);
  void Decode(vector<int> );
  void Decode(unsigned int *, int);
  void DecodeFEC(vector<int> );
  void DecodeFEC(unsigned int *, int );
  void FillAPVRaw(vector<int> &, unsigned int);

  map<int, map<int, vector<int> > > GetDecoded();
  map<int, map<int, TH1F* > >  GetAPVRawHisto();
  map<int, vector<int> >  GetFECDecoded();
  map<int, TH1F* > GetFECAPVRawHisto();

  void Word32ToWord16(unsigned int *, unsigned int*, unsigned int*);

  int IsAdcChannelActive(int &, int & );
  void Clear();
  void clearMaps();
  void clearHistos();
  void clearFECMaps();
  void clearFECHistos();


private:
  int  fBufferSize;

  int nfecID;
  int nadcCh;
  int apvIndex;

  map<int, map<int, vector<int> > > mAPVRawSingleEvent;
  map<int, map<int, TH1F* > > mAPVRawHisto;
  map<int, vector<int> > mFecApvEvent;
  map<int, TH1F* >  mFecApvHisto;


  GemMapping* mapping;

  vector<int> vActiveAdcChannels;
};

#endif
