#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/write.hpp>


#include <stdint.h>
#include <arpa/inet.h>

#include "PmProtocol.h"
#include "MrUUBCalibMonit.h"
#include "calmon_pack.h"
#include "cl_msg_unknown_pack.h"

int MrUUBCalibMonit_monit(TMoRawData &monit,
                          int pack_version,int pack_size,unsigned char *buff,
                          int lsId)

{
  uint16_t *spt;
  int32_t *lpt;

  monit.Reset();
  monit.fLsId=lsId;
  monit.fIsFilled=true;

  if(pack_version == 1){
    if( 156 <= pack_size){
      /* |second(4)|68*adc(2)|8*extra(2)| */
      lpt=(int32_t *)buff;
      monit.fSecond         = ntohl(*lpt++);
      if( 1261872000 < monit.fSecond){
        spt = (uint16_t *)lpt;
        monit.fIsUUB = kTRUE;
        monit.fUPMV[0]       = ntohs(*spt++);
        monit.fBatteryT[1]   = ntohs(*spt++);
        monit.f12VRadio      = ntohs(*spt++);
        monit.fUADC[7]       = ntohs(*spt++);
        monit.fPMT[0]        = ntohs(*spt++);
        monit.f12VRadioI     = ntohs(*spt++);
        monit.fUPMV[2]       = ntohs(*spt++);
        monit.fILED          = ntohs(*spt++);  //SC: I_24V_LED
        monit.fPMV[2]        = ntohs(*spt++);
        monit.fBatteryT[0]   = ntohs(*spt++);
        monit.f12VPM         = ntohs(*spt++);
        monit.fUADC[6]       = ntohs(*spt++);
        monit.fPMT[1]        = ntohs(*spt++);
        monit.f12VPMI        = ntohs(*spt++);
        monit.fUPMV[1]       = ntohs(*spt++);
        monit.fVInI          = ntohs(*spt++);
        monit.fPMV[1]        = ntohs(*spt++);
        monit.fCurrentLoad   = ntohs(*spt++);
        monit.f24VExt[1]     = ntohs(*spt++);
        monit.fUADC[5]       = ntohs(*spt++);
        monit.fPMT[2]        = ntohs(*spt++);
        monit.f3V3AnalogI    = ntohs(*spt++);  //SC: I_P5V_ANA
        monit.fUPMI[2]       = ntohs(*spt++);
        monit.f3V3SCI        = ntohs(*spt++);
        monit.fPMV[0]        = ntohs(*spt++);
        monit.fBatteryV[0]   = ntohs(*spt++);  //SC: BAT_CENT
        monit.f24VExt[0]     = ntohs(*spt++);
        monit.fUADC[4]       = ntohs(*spt++);
        monit.fUPMT[0]       = ntohs(*spt++);
        monit.f_3V3AnalogI   = ntohs(*spt++);  //SC: I_N5V_ANA
        monit.fUPMI[1]       = ntohs(*spt++);
        monit.f1V0I          = ntohs(*spt++);
        monit.fUPMI[0]       = ntohs(*spt++);
        monit.fWaterT        = ntohs(*spt++);  //SC: EXT_TEMP - from TPCB.
        monit.fSolarPanelI   = ntohs(*spt++);
        monit.fUADC[0]       = ntohs(*spt++);
        monit.f3V3           = ntohs(*spt++);
        monit.f5VGPSI        = ntohs(*spt++);
        monit.fUPMT[2]       = ntohs(*spt++);
        monit.f12V_BATTEMP[0]= ntohs(*spt++);
        monit.fPMI[2]        = ntohs(*spt++);
        monit.f12V_WT        = ntohs(*spt++);
        monit.fSolarPanelV   = ntohs(*spt++);
        monit.fUADC[1]       = ntohs(*spt++);
        monit.f5VGPS         = ntohs(*spt++);
        monit.f1V2I          = ntohs(*spt++);
        monit.fUPMT[1]       = ntohs(*spt++);
        monit.f1V8           = ntohs(*spt++);
        monit.fPMI[1]        = ntohs(*spt++);
        monit.f12V_WL        = ntohs(*spt++);
        monit.fBatteryV[1]   = ntohs(*spt++);  //SC: BAT_OUT
        monit.fUADC[2]       = ntohs(*spt++);
        monit.f3V3Analog     = ntohs(*spt++);  //SC: V_AN_P5V
        monit.f3V3I          = ntohs(*spt++);
        monit.f10V           = ntohs(*spt++);
        monit.f1V0           = ntohs(*spt++);
        monit.fPMI[0]        = ntohs(*spt++);
        monit.f12V_BATTEMP[1]= ntohs(*spt++);
        monit.fWaterLevel    = ntohs(*spt++);
        monit.fUADC[3]       = ntohs(*spt++);
        monit.f_3V3Analog    = ntohs(*spt++);  //SC:  V_AN_N5V
        monit.f1V8I          = ntohs(*spt++);
        monit.fUADC[8]       = ntohs(*spt++);
        monit.f1V2           = ntohs(*spt++);
        monit.fElectP        = ntohs(*spt++);
        monit.fElectT        = ntohs(*spt++);
        monit.fElectH        = ntohs(*spt++);
        monit.fExtT          = ntohs(*spt++);  //SC: not implemented.
        // 8 extra values
        for (int i=0;i<8;i++)
          monit.fExtra[i]    = ntohs(*spt++);
        // Physical values, from SlowControl.c from KHB. Some are missing
#define LSB_TO_5V  1.8814
#define LSB_TO_24V 8.88
#define LSB_TO_12V 4.43
#define LSB_TO_3V3 1.20
#define LSB_TO_1V8 0.674
#define LSB_TO_1V2 0.421
#define LSB_TO_1V0 0.366

        for (int pmt=0;pmt<4;pmt++) {
          monit.fMonitoring.fPMT[pmt] = monit.fPMT[pmt]*LSB_TO_5V-273.15;
          monit.fMonitoring.fPMV[pmt] = monit.fPMV[pmt]*LSB_TO_5V;
          monit.fMonitoring.fPMI[pmt] = monit.fPMI[pmt]*LSB_TO_5V;
          monit.fMonitoring.fUPMT[pmt]= monit.fUPMT[pmt]*LSB_TO_5V-273.15;
          monit.fMonitoring.fUPMV[pmt]= monit.fUPMV[pmt]*LSB_TO_5V;
          monit.fMonitoring.fUPMI[pmt]= monit.fUPMI[pmt]*LSB_TO_5V;
        }
        monit.fMonitoring.f1V0        = monit.f1V0*LSB_TO_1V0;
        monit.fMonitoring.f1V2        = monit.f1V2*LSB_TO_1V2;
        monit.fMonitoring.f1V8        = monit.f1V8*LSB_TO_1V8;
        monit.fMonitoring.f1V0I       = monit.f1V0I*LSB_TO_1V0/60.*41.67;
        monit.fMonitoring.f1V2I       = monit.f1V2I*LSB_TO_1V0/60.*10.;
        monit.fMonitoring.f1V8I       = monit.f1V8I*LSB_TO_1V0/60.*30.3;
        monit.fMonitoring.fBatteryV[0]= monit.fBatteryV[0]*LSB_TO_5V*18./5000.;
        monit.fMonitoring.fBatteryV[1]= monit.fBatteryV[1]*LSB_TO_5V*36./5000.;
        monit.fMonitoring.fCurrentLoad= monit.fCurrentLoad*LSB_TO_5V/48.;
        monit.fMonitoring.f3V3        = monit.f3V3*LSB_TO_3V3;
        monit.fMonitoring.f3V3I       = monit.f3V3I*LSB_TO_3V3/60.*16.13;
        monit.fMonitoring.f3V3SCI     = monit.f3V3SCI*LSB_TO_1V0/60.*12.2;
        monit.fMonitoring.f3V3Analog  = monit.f3V3Analog*LSB_TO_3V3;  // This is 5V, not 3.3
        monit.fMonitoring.f3V3AnalogI = monit.f3V3AnalogI*LSB_TO_3V3/60.*12.2;
        float Ua=monit.f_3V3Analog*LSB_TO_3V3;
        monit.fMonitoring.f_3V3Analog=
          Ua*2.-(10./7.5 * (2500.-Ua)); // This is -5V, not -3.3
        monit.fMonitoring.f_3V3AnalogI = monit.f_3V3AnalogI*LSB_TO_3V3/60.*12.2;
        monit.fMonitoring.f5VGPS       = monit.f5VGPS*LSB_TO_5V;
        monit.fMonitoring.f5VGPSI      = monit.f5VGPSI*LSB_TO_1V0/60.*10;
        monit.fMonitoring.f12VRadio    = monit.f12VRadio*LSB_TO_12V;
        monit.fMonitoring.f12VRadioI   = monit.f12VRadioI*LSB_TO_1V0/60.*30.3;
        monit.fMonitoring.f12VPM       = monit.f12VPM*LSB_TO_12V;
        monit.fMonitoring.f12VPMI      = monit.f12VPMI*LSB_TO_1V0/60.*30.3;
        monit.fMonitoring.f24VExt[0]   = monit.f24VExt[0]*LSB_TO_24V;
        monit.fMonitoring.f24VExt[1]   = monit.f24VExt[1]*LSB_TO_24V;
        monit.fMonitoring.fVInI        = monit.fVInI*LSB_TO_1V0/60.*21.28;
        monit.fMonitoring.fBatteryT[0] = monit.fBatteryT[0]*LSB_TO_5V-273.15;
        monit.fMonitoring.fBatteryT[1] = monit.fBatteryT[1]*LSB_TO_5V-273.15;
        monit.fMonitoring.fExtT        = monit.fExtT*LSB_TO_5V-273.15;
        monit.fMonitoring.fSolarPanelV = monit.fSolarPanelV*LSB_TO_5V*50./5000.;
        monit.fMonitoring.fSolarPanelI = monit.fSolarPanelI*LSB_TO_5V*5./1000.;
        // End of physical values

        monit.fIsRawMonitoring=1;
        monit.fMonitoring.fIsMonitoring=1;
        monit.fMonitoring.fIsUUB=kTRUE;
        //gLastMonitBookingTime[monit.fLsId]=(UInt_t)time((time_t *)NULL);
        return 0;
      } //end of if(xxx < second)
    } //end of if(pack_size==156)
  } //end of if(pack_version);
  monit.fIsRawMonitoring=0;
  monit.fMonitoring.fIsMonitoring=0;
  return(-1);
}
int MrUUBCalibMonit_calib(TCbRawData &calib,
                          int pack_version,int pack_size,unsigned char *buff)
{
  uint16_t *spt;
  int32_t *lpt;
  int i;
  calib.Reset();
  calib.fIsFilled=true;
  if(pack_version==1){
    if(104<=pack_size){
      calib.fIsCalibration=true;
      spt=(uint16_t *)buff;
      calib.fVersion2    = ntohs(*spt++);
      calib.fTubeMask    = ntohs(*spt++);
      lpt=(int32_t *)spt;
      calib.fStartSecond = ntohl(*lpt++);
      spt=(uint16_t *)lpt;
      calib.fEndSecond   = ntohs(*spt++);
      calib.fTotD = ntohs(*spt++);
      calib.fMops = ntohs(*spt++);
      spt++;
      calib.fT1          = ntohs(*spt++);
      calib.fT2          = ntohs(*spt++);
      calib.fTotRate     = ntohs(*spt++)*1./calib.fEndSecond;
      uint16_t nb_full_buffer = ntohs(*spt++);
      for(i=0;i<3;i++){
        calib.fAnode[i]    = ntohs(*spt++)*.01;
        calib.fDynode[i]   = ntohs(*spt++)*.01;
      }
      for(i=0;i<4;i++){
        calib.fUBase[i] = ntohs(*spt++)*.01;
      }

      for(i=0;i<3;i++){
        calib.fVarianceAnode[i]    = ntohs(*spt++)*.01;
        calib.fVarianceDynode[i]   = ntohs(*spt++)*.01;
      }
      for(i=0;i<4;i++){
        calib.fVarianceUBase[i] = ntohs(*spt++)*.01;
      }
      for(i=0;i<3;i++){
        calib.fPast[i]   = ntohs(*spt++);
      }
      uint16_t fPast_ssd   = ntohs(*spt++);  //it is still not implemented
      for(i=0;i<3;i++){
        calib.fPeak[i]   = ntohs(*spt++)*.1;
      }
      Float_t fPeak_ssd   = ntohs(*spt++)*.1;  //it is still not implemented

      for(i=0;i<3;i++){
        calib.f70HzRate[i] = ntohs(*spt++)*.01;
      }
      Float_t f70HzRate_ssd   = ntohs(*spt++);  //it is still not implemented
      for(i=0;i<3;i++){
        calib.fDynodeAnode[i]=ntohs(*spt++)*.01;       // HG/LG Ratio
      }
      Float_t fHG_LG_ssd   = ntohs(*spt++)*.01;  //it is still not implemented
      for(i=0;i<3;i++){
        calib.fArea[i] = ntohs(*spt++)*.1;
      }
      Float_t fArea_ssd = ntohs(*spt++)*.1;

      return(0);
    } //end of if(...<=pack_size
  } //end of if (pack_version==1)

  calib.fIsCalibration=false;
  return(-1);
}

int MrUUBCalibMonit_Rd(TSDRd &rd,
                       int pack_version,int pack_size,unsigned char *buff)
{
  return(0);
}
int MrUUBCalibMonit_System(TSDUSystem &sys,
                           int pack_version,int pack_size,unsigned char *buff)
{
  uint32_t *lpt;
  int i;
  printf("System %d\nversion:",pack_size);
  for(i=0;i<pack_size;i++){
    printf("%02x%c",buff[i],(((i+1)%8==0)?'\n':' '));
  }
  printf("\n");
  for(i=0;i<3;i++){
    printf("===%s===;",buff+(i*8));
  }
  printf("\n");
  lpt=(uint32_t *)(buff+24);
  printf("%x %x\n",lpt[0],lpt[1]);
  return(0);
}


int MrUUBCalibMonit_Extra(TSDExtra &extra,
                           int pack_version,int pack_size,unsigned char *buff)
{
  return(0);
}


int MrUUBCalibMonit(TMoRawData &monit,TCbRawData &calib,char *buff_in)
{
  /*
   * m -> monitoring data structure - will be update with buff information
   * c -> calibration data structure - will be udated with buff information
   * buff_in -> input data buffer.
   * |xxx(4)|PmLsRawMessage|. PmLsRawMessage is defined at Pm/PmProtocol.h
   *
   * PmLsRawMessage: |lsId(4)|mess(PmLsMessage)| def. at Pm/PmProtocol.h
   * PmLsMessage: |length(4)|type(4)|version(4)|data(n)| def. at Pm/PmProtocol.h
   * The data has the internal format:
   * |requestId(4)|bufferOK(4)|msg_size(2)|comp_size(2)|compressed_data(n)|
   * - msg_size is the length of compressed_data after uncompress
   * - comp_size is the compressed_data length
   * - the compressed_data (after uncompressed will have the format:
   *
   * |H|h1(4)|data1(n)|h2(4)|data2(n)|...|
   *
   * in case of calibration/monitoring, the there is not "H"
   * which is interpreted by the "msg_unpack.c" library
   */
  PmLsRawMessage *rawP = (PmLsRawMessage *)(buff_in+sizeof(longWord));
  PmLsMessage *messP =  &(rawP->mess);
  int lsId = ntohl(rawP->lsId);
  int rcvcode = ntohl(messP->type);
  int fVersion=ntohl(messP->version);
  int errcode=0;
  unsigned char *dat=(unsigned char *)(messP->data);

  int dat_len;
  unsigned char buff_uncomp[65536];
  struct msg_pack_unpack_str pack;
  unsigned char pack_buff[65536];
  unsigned int pack_type,pack_version;
  int pack_size;


  TSDRd rd;
  TSDUSystem sys;
  TSDExtra extra;

  longWord *lpt;
  uint16_t *spt;
  longWord reqId,buffOk;
  int size,csize,ss;

  cl_msg_unknown_pack unknown_pack("monit","!monit!!");

  dat_len = ntohl(messP->length);
  monit.fVersion = ntohl(messP->version);
  calib.fVersion = monit.fVersion;
  ntohl(messP->type);

  if(dat_len<12){
    return(-1);
  }
  lpt=(longWord *)messP->data;
  reqId=ntohl(*lpt++);
  buffOk=ntohl(*lpt++);
  spt=(uint16_t *)lpt;
  size=ntohs(*spt++);
  csize=ntohs(*spt++);

  if(65536<size){
    return(-2);
  }

  /*============ uncompress data. mostly the same as in Eb =========== */
  if(size!=csize){
    std::istringstream iss(std::ios::binary);
    iss.rdbuf()->pubsetbuf((char*)spt, csize);

    boost::iostreams::filtering_istreambuf zdat;
    zdat.push(boost::iostreams::bzip2_decompressor());
    zdat.push(iss);

    try {
      ss=boost::iostreams::read(zdat, (char*)buff_uncomp, 65536);
    } catch(...) {
      printf("Invalid Compressed data from station %d", lsId);
      return(-3);
    }
    if(size!=ss){
      printf("Wrong message size from station %d: len: %d; expected: %d ",
             lsId,ss,size);
      return(-4);
    }
  } else {
    memcpy(buff_uncomp,spt,csize);
  }
  /*=================== interprete data =================== */
  msg_unpack_assign_data(&pack,size,buff_uncomp);

  do{
    pack_size=msg_unpack_get_data(&pack,
                                  &pack_type,&pack_version,pack_buff,65536);
    if(0<=pack_size){
      switch(pack_type){
      case(CALMON_PACK_MONIT):
        MrUUBCalibMonit_monit(monit, pack_version, pack_size, pack_buff,
                              lsId);
        break;
      case(CALMON_PACK_CALIB):
        MrUUBCalibMonit_calib(calib, pack_version, pack_size, pack_buff);
        break;

      //case(CALMON_PACK_RD):
      //  printf("rd\n");
      //  MrUUBCalibMonit_Rd(rd, pack_version, pack_size, pack_buff);
      //  break;
      //case(CALMON_PACK_SYSTEM):
      //  printf("sys\n");
      //  MrUUBCalibMonit_System(sys, pack_version, pack_size, pack_buff);
      //  break;
      //case(CALMON_PACK_EXTRA):
      //  printf("extra\n");
      //  MrUUBCalibMonit_Extra(extra, pack_version, pack_size, pack_buff);
      //  break;
      default:
        printf("unknwon pack. Type,version,size=%d %d %d\n",
               pack_type,pack_version,pack_size);

        unknown_pack.add_msg(pack_type,pack_version,pack_size,
                             (char *)pack_buff);
        break;
      }
    }
  } while(0<=pack_size);
  if(unknown_pack.has_packs()){
    time_t t_pack;
    t_pack=time(NULL);
    unknown_pack.store(lsId,0,0,t_pack);
  }
  calib.fLsId = monit.fLsId;
  calib.fType=rcvcode;
  monit.fType=rcvcode;
  return(0);
}
