#include <math.h>
#include <iostream>
#include <sstream>
//BOOST bzip2
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/write.hpp>

#include <stdio.h>

#include "Pm.h"

#include "SPMTData.h"

#include "spmtIO.h"

#include "spmt_prot.h" /*data format from the UUB */

using namespace std;

SPMTData::SPMTData()
{
  fFileManager=new SPMTRoot();
  //debug_raw_index=0;
}

SPMTData::~SPMTData()
{
  delete(fFileManager);
}

void SPMTData::SetOutPath(const char *path)
{
  fFileManager->SetOutPath(path);
}


int SPMTData::SetData(char *data)
{

  /*data is the raw data from Pm.
   * return: bits error code:
   *   0: fail to uncompress data
   *   1: uncompressed data had different size of expected size
   *   2: header looks to be inconsistent
   *   3: uncompressed data format looks to be wrong.
   */
  unsigned char buf[SPMT_PROT_BUFF_SIZE_MAX];

  int lsId,type,version,len;

  int err=0;

  PmLsRawMessage *raw = (PmLsRawMessage *)(data+sizeof(longWord));

  /* ... |version(longWord)|output(PmLsRawMessage)|
   *   version is the version between the processes in CDAS.
   *
   * PmLsRawMessage: |lsId(longWord)|mess(PmLsMessage)|
   * PmLsMessage: |length|type|version|data(char)|, length...version-longWord
   *    lsId, length, type, version: network byte order.
   */
  lsId    = ntohl(raw->lsId);
  len     = ntohl(raw->mess.length); //it include all the "mess" size
  type    = ntohl(raw->mess.type);
  version = ntohl(raw->mess.version);

  /* data format (struct spmt_msg_h):
   *   |version(2)|nevt(2)|size_comp(2)|size_uncomp(2)|comp_data|
   *  -> version: version of the message
   *  -> nevt
   *  -> size_comp: size of the comp_data. Not the entire size
   *  -> size_uncomp
   *  -> comp_data: compressed (bzip2) data with length size_comp.
   *       + after uncompressed the size is size_uncomp.
   *       + If the size_comp==size_uncomp, the data is not compressed.
   */

  if(20<len) { /* 3*4 + 4*2: len, type, version(comms);
		*version(spmt), nevt, size_comp, size_uncomp
		*/
    struct spmt_msg_h *header;

    header=(struct spmt_msg_h *)&(raw->mess.data);

    header->version     = ntohs(header->version);
    header->nevt        = ntohs(header->nevt);
    header->size_comp   = ntohs(header->size_comp);
    header->size_uncomp = ntohs(header->size_uncomp);

    //{
    //  FILE *arq;
    //  char fname_out[100];
    //  if(len<1000000){
    //	sprintf(fname_out,"/Raid/monit/Sd/spmt_raw_%04d",debug_raw_index);
    //	arq=fopen(fname_out,"w");
    //	if(arq!=NULL){
    //	  fwrite(data,1,len+40,arq);
    //	  fclose(arq);
    //	  debug_raw_index++;
    //	}
    //  }
    //}

    if(header->size_comp == len-20 && //
       header->size_uncomp < SPMT_PROT_BUFF_SIZE_MAX ) {
      if(header->size_comp < header->size_uncomp){
	std::istringstream iss(std::ios::binary );
	boost::iostreams::filtering_istreambuf zdat;
	int outsize;

	iss.rdbuf()->pubsetbuf((char*)(header+1), header->size_comp );
	zdat.push(boost::iostreams::bzip2_decompressor());
	zdat.push(iss);

	try {
	  outsize=boost::iostreams::read(zdat,(char*)buf,SPMT_PROT_BUFF_SIZE_MAX);
	} catch(...) {
	  err |= 0x1;
	  printf("data compression problem!!\n");
	}
	if(outsize!=header->size_uncomp){
	  err |= 0x2;
	}
      } else {
	memcpy(buf,header+1, header->size_uncomp);
      }
    } else {
      err|=0x4;
      printf("Message size or header not consistent.\n");
    }
    if(err==0){
      if(_Process_data(lsId,buf,header->size_uncomp)){
	err |=0x8;
	printf("Wrong data format \n");
      }
    }
  } else {
    err |= 0x4;
  }
  return(err);
}


int SPMTData::_Process_data(int lsId,unsigned char *buf,int size)
{
  /* lsId: Station Id.
   * buf: data buffer;
   *     It may have multiple packets of format:
   *        |size(1)|type(1)|data(size - 2)|
   *       size is the total size of each packet
   *       type is data data: espmt_prot_EVT=1,
   *                          espmt_prot_MUON_INFO=2
   *                          espmt_prot_DAQ_INFO=3
   *       data - may have different format, depends on the type.
   *     The last valid byte the the value 0.
   * size: amount of data valid in buf (in bytes)
   *
   * Not all packets of type EVT will have a corresponding
   *   MUON_INFO and DAQ_INFO. In those cases, the corresponding values
   *   are previously received.
   * Anyway, before the first EVT packet on the data block,
   *  it would have MUON_INFO and DAQ_INFO. Therefore, it would
   *  not be necessary to store or guess the previous values used
   *  in the station.
   */
  int npt;

  spmtIO data;

  struct spmt_prot_evt_str evt;
  struct spmt_prot_muon muon;
  struct spmt_prot_add_info daq_info;

  int err,i;

  memset(&evt,0,sizeof(evt));
  memset(&muon,0,sizeof(muon));
  memset(&daq_info,0,sizeof(daq_info));

  data.LsId = lsId;

  for(npt=0,err=0; npt<size && 0<buf[npt] && err==0; npt+=buf[npt]){
    switch(buf[npt+1]){
    case(espmt_prot_EVT):
      /* |sec(4)|ticks(4)|nevt(4)|AREA_PEAK(6*4)|
       * sec - second of the event.
       * ticks -> number of thicks after the last PPS
       * nevt -> event index, just to track possible event lost
       * AREA_PEAK[6]:
       *   0: WCD PMT1
       *   1: WCD PMT2
       *   2: WCD PMT3
       *   3: small PMT
       *   4: SSD PMT - low gain.
       *   5: SSD PMT - high gain.
       *        bit 31: if the channel is saturated.
       *        bit 30-12: AREA of the signal
       *        bit 11-0 : Peak of the signal
       */
      if(buf[npt]-2 ==sizeof(evt)){
	spmtIOEvt e;
	uint32_t v;
	memcpy(&evt,&(buf[npt+2]),sizeof(evt));
	e.GPSsec   = ntohl(evt.sec  );
	e.GPSTicks = ntohl(evt.ticks);
	for(i=0;i<6;i++){
	  v= ntohl(evt.AREA_PEAK[i]);
	  /*
           *
	   * the AREA_PEAK is a 32 bits word with the following informations:
	   * bits 18 ~ 0: AREA
	   * bits 30 ~19: PEAK
	   * bit  31: if the chanel is saturated.
	   * see "Shower Features" session of SDE_PLD_Firmware_Spec
	   *    of Programable Logic documentation.
	   */
	  e.charge_FADC[i] =  v      & 0x3FFFF;
	  e.peak_FADC[i]   = (v>>19) & 0xFFF ;
	  e.saturation[i]  = (v>>31) & 0x1;
	}
        data.evt.push_back(e);
      }else{
	printf("Data format error!!\n");
	err=1;
      }
      break;
    case(espmt_prot_MUON_INFO):
      /* |StartSec(4)|charge(4*2)|
       * StartSec - start time which the histogram has been started.
       * charge[4] - charge of the VEM, extracted from the histogram.
       *    0: WCD PMT1
       *    1: WCD PMT2
       *    2: WCD PMT3
       *    3: SSD PMT
       */
      if(buf[npt]-2 == sizeof(muon)){
	spmtIOMuonCalib m;
	memcpy(&muon,&(buf[npt+2]),sizeof(muon));
	m.ChargeCalibTime = ntohl(muon.StartSec);
	for(i=0;i<4;i++){
	  m.Charge[i] = ntohs(muon.charge[i]);
	}
	data.muon.push_back(m);
      } else {
	err=1;
      }
      break;
    case(espmt_prot_DAQ_INFO):
      /* acquisition parameters
       * |t_start(4)|th(3*2)|mask(2)|r(3*2)|vem(3*2)|
       * t_start - time which the th, r, vem has been started to be used.
       * th - threshold,
       * r - HG/LG ratio.
       * vem - vem value estimated in the trigger2.
       *   0,1,2: WCD PMT1, PMT2, PMT3.
       */
      if(buf[npt]-2 == sizeof(daq_info)){
	spmtIOThreshold t;
	memcpy(&daq_info,&(buf[npt+2]),sizeof(daq_info));
	t.time = ntohl(daq_info.t_start);
	t.mask = ntohs(daq_info.mask);
	for(i=0;i<3;i++){
	  t.Th[i] = ntohs(daq_info.th[i] );
	  t.HG_LG[i]  = ntohs(daq_info.r[i]  );
	  t.VEM[i]= ntohs(daq_info.vem[i]);
	}
	data.thres.push_back(t);
      } else {
	err=1;
      }
      break;
    default:
      printf("wrong type %d\n",buf[npt+1]);
    }
  }
  if(err==0){
    fFileManager->AddData(&data);
  }
  data.ResetData();
  return(err);
}
