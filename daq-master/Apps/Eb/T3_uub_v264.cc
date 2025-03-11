#include <arpa/inet.h>
#include "T3_uub.h"
#include "t3_pack.h"
#include "cl_msg_unknown_pack.h"
#include <time.h>

#define MAX_BUFF_SIZE 65536

IoUSdTrigParam *T3_uub_v264_trig_param(unsigned char *buff,int pack_size,int pack_version)
{
  IoUSdTrigParam *param;
  uint32_t *lpt;

  if( 39*sizeof(uint32_t)<=pack_size && pack_version==1){
    lpt=(uint32_t *)buff;
    param=new IoUSdTrigParam();
    param->PL_version      = ntohl(*lpt++);
    param->TrigMask        = ntohl(*lpt++);
    param->csbt_th[0]      = ntohl(*lpt++);
    param->csbt_th[1]      = ntohl(*lpt++);
    param->csbt_th[2]      = ntohl(*lpt++);
    param->csbt_enable     = ntohl(*lpt++);
    param->ctot_th[0]      = ntohl(*lpt++);
    param->ctot_th[1]      = ntohl(*lpt++);
    param->ctot_th[2]      = ntohl(*lpt++);
    param->ctot_enable     = ntohl(*lpt++);
    param->ctot_occ        = ntohl(*lpt++);
    param->ctotd_thmin[0]  = ntohl(*lpt++);
    param->ctotd_thmin[1]  = ntohl(*lpt++);
    param->ctotd_thmin[2]  = ntohl(*lpt++);
    param->ctotd_thmax[0]  = ntohl(*lpt++);
    param->ctotd_thmax[1]  = ntohl(*lpt++);
    param->ctotd_thmax[2]  = ntohl(*lpt++);
    param->ctotd_enable    = ntohl(*lpt++);
    param->ctotd_occ       = ntohl(*lpt++);
    param->ctotd_fd        = ntohl(*lpt++);
    param->ctotd_fn        = ntohl(*lpt++);
    param->ctotd_int       = ntohl(*lpt++);
    param->cmops_thmin[0]  = ntohl(*lpt++);
    param->cmops_thmin[1]  = ntohl(*lpt++);
    param->cmops_thmin[2]  = ntohl(*lpt++);
    param->cmops_thmax[0]  = ntohl(*lpt++);
    param->cmops_thmax[1]  = ntohl(*lpt++);
    param->cmops_thmax[2]  = ntohl(*lpt++);
    param->cmops_enable    = ntohl(*lpt++);
    param->cmops_occ       = ntohl(*lpt++);
    param->cmops_ofs       = ntohl(*lpt++);
    param->cmops_int       = ntohl(*lpt++);
    param->sbt_th[0]       = ntohl(*lpt++);
    param->sbt_th[1]       = ntohl(*lpt++);
    param->sbt_th[2]       = ntohl(*lpt++);
    param->sbt_th[3]       = ntohl(*lpt++);
    param->sbt_enable      = ntohl(*lpt++);
    param->led_ctrl        = ntohl(*lpt++);
    param->random_mode     = ntohl(*lpt++);
    return(param);
  } else {
    return(NULL);
  }
}

IoSdHisto *T3_uub_v264_histo(unsigned char *buff,int pack_size,int pack_version)
{
  IoSdHisto *h;

  uint32_t h_type;
  unsigned short Offset[4];

  float BaseAvg[4];

  int i,j;

  uint8_t *pt;
  uint16_t *spt;
  uint32_t *lpt;

  h=NULL;

  if(7336<=pack_size && pack_version==1){
    h=new IoSdHisto();

    lpt=(uint32_t *)buff;
    h->type = ntohl(*lpt++);
    pt=(uint8_t *)lpt;
    for(j=0;j<4;j++){
      h->Pk_bit_shift[j] = *pt++;
    }
    for(j=0;j<4;j++)
      h->Ch_bit_shift[j] = *pt++;

    spt=(uint16_t *)pt;
    for(j=0;j<4;j++)
      for(i=0;i<2;i++)
        h->Pk_bin[i][j] = ntohs(*spt++);

    for(j=0;j<4;j++)
      for(i=0;i<2;i++)
        h->Ch_bin[i][j] = ntohs(*spt++);

    for(j=0;j<4;j++)
      h->BaseAvg[j] = ntohs(*spt++)/100.;

    for (i=0;i<4;i++)
      Offset[i] = ntohs(*spt++);
    // got the baselines used for all 4 channels: PMT 1-3 and SSD
    for (i=0;i<3;i++) {
      h->Offset[i]   = Offset[i]; // baselines
      h->Offset[i+3] = Offset[i]; // peak
      h->Offset[i+6] = (h->Ch_bin[1][i] - h->Ch_bin[0][i])*Offset[i];
    }
    h->Offset[9] = 69*Offset[3]; // charge SSD histogram, old. Actually is incorrect since SSD charge isn't computed on 69 bins
    // setting SSD histograms offset values;
    h->Offset3[0] = Offset[3];
    h->Offset3[1] = Offset[3];
    h->Offset3[2] = Offset[3]*(h->Ch_bin[1][3] - h->Ch_bin[0][3]);
    for (i=0;i<3;i++)
      for (j=0;j<20;j++)
        h->Base[i][j] = ntohs(*spt++);
    for (j=0;j<20;j++)
      h->Base3[j] = ntohs(*spt++); //special treatment of SSD baseline histogram
    for (i=0;i<3;i++)
      for (j=0;j<150;j++)
        h->Peak[i][j] = ntohs(*spt++);
    for (j=0;j<150;j++)
      h->Peak3[j] = ntohs(*spt++); // special treatment of SSD peak histogram
    for (i=0;i<4;i++)
      for (j=0;j<600;j++)
        h->Charge[i][j] = ntohs(*spt++);
    lpt=(uint32_t *)spt;
    for (i=0;i<4;i++)
      for (j=0;j<69;j++)
        h->UShape[i][j] = ntohl(*lpt++);

    h->tStart     =ntohl(*lpt++);
    h->tEnd       =ntohl(*lpt++);
    h->Entries    =ntohl(*lpt++);

  } else if(pack_size==0){
    h=new IoSdHisto();
  }
  return(h);
}

IoSdHistoCoinc *T3_uub_v264_histo_coinc(unsigned char *buff, int pack_size, int pack_version)
{
  IoSdHistoCoinc *h = NULL;

  if (pack_size < 4504 || pack_version != 1)
    return(h);

  h = new IoSdHistoCoinc();

  uint16_t *pt = (uint16_t *) buff;

  h->HasHisto = ntohs(*pt++);
  h->SSDThreshold = ntohs(*pt++);

  for (unsigned int pmt = 0; pmt < 3; pmt++) {
    for (unsigned int bin = 0; bin < 600; bin++) {
      h->Charge[pmt][bin] = ntohs(*pt++);
    }
  }

  for (unsigned int pmt = 0; pmt < 3; pmt++) {
    for (unsigned int bin = 0; bin < 150; bin++) {
      h->Peak[pmt][bin] = ntohs(*pt++);
    }
  }

  return(h);    
}

IoSdCalib *T3_uub_v264_calib(unsigned char *buff,int size,int version)
{
  IoSdCalib *calib;
  if(104<=size && version==1){
    int i;
    uint32_t *lpt;
    uint16_t *spt;

    spt=(uint16_t *)buff;

    calib = new IoSdCalib();
    calib->Version      = ntohs(*spt++);
    calib->TubeMask     = ntohs(*spt++);
    lpt = (uint32_t *)spt;
    calib->StartSecond  = ntohl(*lpt++);
    spt = (uint16_t *)lpt;
    calib->EndSecond    = ntohs(*spt++);
    calib->NbTOTD       = ntohs(*spt++);
    calib->NbMOPS       = ntohs(*spt++);
    *spt++; //skip the number of external trigger.
    calib->NbT1         = ntohs(*spt++);
    calib->NbT2         = ntohs(*spt++);
    calib->NbTOT        = ntohs(*spt++);
    calib->NbFullBuff   = ntohs(*spt++);
    calib->NEntries = 0; //The events considered are all the T1.
    for(i=0;i<6;i++)
      calib->Base[i]       = ntohs(*spt++)/100.;
    for(i=0;i<4;i++)
      calib->UBase[i]    = ntohs(*spt++)/100.;
    for(i=0;i<6;i++)
      calib->SigmaBase[i]  = ntohs(*spt++)/100.;
    for(i=0;i<4;i++)
      calib->SigmaUBase[i]= ntohs(*spt++)/100.;

    for(i=0;i<3;i++)
      calib->Evolution[i] = ntohs(*spt++);
    *spt++; //skip the Evolution for SSD.
    for(i=0;i<3;i++)
      calib->VemPeak[i]    = ntohs(*spt++)/10.;
    *spt++; //skip the "VEM" for SSD - not calculated yet
    for(i=0;i<3;i++)
      calib->Rate[i]      = ntohs(*spt++)/100.;
    *spt++; //skip the Rate for SSD - not calculated yet
    for(i=0;i<3;i++)
      calib->DA[i]       = ntohs(*spt++)/100.; //it is HG/LG ratio.
    *spt++; //skip the HG/LG for SSD - not calculated yet
    for(i=0;i<3;i++)
      calib->VemCharge[i]        = ntohs(*spt++)/10.;
    *spt++; //skip the "VEM Charge" for SSD - not calculated yet
  } else {
    calib=NULL;
  }
  return(calib);
}

int T3_uub_v264_ttag(unsigned char *buff,int size,int version,IoSdStation *dat)
{
  IoSdGps *ttag;
  uint32_t *lpt;
  int8_t *pt;
  if(8<=size && version==1){
    if(dat->Gps==NULL){
      dat->Gps=new IoSdGps();
    }
    ttag=dat->Gps;
    lpt=(uint32_t *)buff;

    ttag->IsUUB = 1;
    ttag->Next100 = ntohl(*lpt++);

    pt=(int8_t *)lpt;
    ttag->Current40  = (*pt++);
    ttag->PreviousST = (*pt++);
    ttag->CurrentST  = (*pt++);
    ttag->NextST     = (*pt++);
    return(0);
  }
  return(1);
}

int T3_uub_v264_trace(unsigned char *buff,int size,int version,IoSdStation *dat)
{
  IoUSdFadc *fadc;

  uint32_t *lpt;

  int internalId;

  if(size<41000 || version !=1){
    return(1);
  }

  dat->Fadc=NULL;
  if(dat->UFadc ==NULL){
    dat->UFadc = new IoUSdFadc();
  }

  if(dat->Gps==NULL){
    dat->Gps = new IoSdGps();
  }

  dat->UFadc->CodingVersion=3;
  dat->Gps->IsUUB=1;
  dat->Trigger.Type=0; //it is a UUB.

  /*data format:
   * |InternalId(4) |Type1(4) |Type2(4) |trace_start(4) |gps_info(3*4) |
   * |micro_off(4) |nsamples(4) |fadc_raw(5*4*2048) |buffer_status(4) |
   * gps_info: |sec(4)|ticks(4)|tics_prev_pps(4)|
   */
  fadc = dat->UFadc;

  lpt=(uint32_t *)buff;

  internalId             = ntohl(*lpt++);
  dat->Trigger.Type1     = ntohl(*lpt++);
  dat->Trigger.Type2     = ntohl(*lpt++);
  dat->UFadc->TraceStart = ntohl(*lpt++);
  dat->Gps->Second       = ntohl(*lpt++);
  dat->Gps->Tick         = ntohl(*lpt++);
  dat->Gps->TickFall     = ntohl(*lpt++);
  lpt++;//gps offset

  fadc->NSample          = ntohl(*lpt++);
  if(fadc->NSample == 2048){
    int nbval=fadc->NSample * 10;
    int16_t *spt=(int16_t *)lpt;
    fadc->Traces.reserve(nbval);
    fadc->Traces.assign(spt,spt+nbval);
    spt+=nbval;
    lpt=(uint32_t *)spt;
    fadc->ShwrBufSt=ntohl(*lpt++);
    fadc->RdBufSt = 0;/* conside no Rd trace - RD block come always
                         after UUB traces*/
    return(0);
  }
  return(1);
}
int T3_uub_v264_extra(unsigned char *buff,int size,int version,uint32_t *extra)
{
  uint32_t *lpt;
  int i;
  lpt=(uint32_t *)buff;
  for(i=0;i<32;i++){
    extra[i]=ntohl(lpt[i]);
  }
  return(0);
}

int T3_uub_v264_rd(unsigned char *buff,int size,int version,IoSdStation *dat)
{
  /* include Rd trace
   * return
   *  0 on success
   *  1 - it would have had UFadc allocated before
   *  2 - not enough data for RD
   */
  int nb;
  IoUSdFadc *fadc;
  uint32_t *lpt;

  if(dat->UFadc==NULL){
    return(1);
  }
  nb = sizeof(uint32_t);
  fadc = dat->UFadc;
  lpt=(uint32_t *)buff;
  if(size < nb || version!=1){
    return(1);
  }

  fadc->RdBufSt = ntohl(*lpt++);
  if(fadc->RdBufSt==0){
    return(0);
  }
  nb += fadc->NSample * sizeof(uint32_t) ;
  if(size < nb){
    return(2);
  }

  int nbval=dat->UFadc->NSample*12;// 5*2 high/low ch + 2 radio ch

  fadc->Traces.reserve(nbval);
  int16_t *spt=(int16_t *)lpt;
  for (int i=0;i<2048*2;i++)
    fadc->Traces.push_back(*spt++);
  return(0);
}

int T3_uub_v264(IoSdStation *dat,uint8_t *buf,int nd)
{
  /* for version 263
   *  *buf: data buffer from the UUB.
   *  nd  : number of bytes
   *  *dat: data buffer to be stored.
   *
   */
  uint32_t *lpt;
  uint16_t *spt;
  uint8_t *pt;
  unsigned int version,InternalId;
  int nnd;
  short int err;
  unsigned char buff[MAX_BUFF_SIZE];

  unsigned int pack_type,pack_ver;
  int pack_size;
  bool pack_list[T3_PACK_LAST];
  struct msg_pack_unpack_str pk;
  
  cl_msg_unknown_pack unknown_pack("sd","!!_T3_!!");
  int i;

  for(i=0;i<T3_PACK_LAST;i++){
    pack_list[i]=false;
  }
  msg_unpack_assign_data(&pk,nd,buf);

  if(msg_unpack_get_header(&pk,(unsigned char *)&version, sizeof(uint32_t))<0){
    dat->Error = IoSdEvent::eBadCompress;
    return (IoSdEvent::eBadCompress);
  }
  version=ntohl(version);
  //printf("version: %d\n",version);
  if(version<264 || 300<version){
    dat->Error = IoSdEvent::eBadCompress;
    return(IoSdEvent::eBadCompress);
  }

  do{
    pack_size = msg_unpack_get_data(&pk, &pack_type, &pack_ver, buff,
                                    MAX_BUFF_SIZE);
    if(pack_size < -1){
      printf("Interpretation error!!!\n");
    }
    if(0 <= pack_size ){
      if( 0 <= pack_type && pack_type < T3_PACK_LAST ){
        pack_list[ pack_type ] = true;
      }
      /*interprete the data */
      switch( pack_type ) {
      case T3_PACK_TRIG_PARAM:
        dat->UTrigParam = T3_uub_v264_trig_param(buff,pack_size,pack_ver);
        if(dat->UTrigParam==NULL){
          pack_list[ pack_type ] = false;
          unknown_pack.add_msg(pack_type,pack_ver,pack_size,(char *)buff);
        } else {
          pack_list[ pack_type ] = true;
        }
        break;
      case T3_PACK_HISTO:
        dat->Histo = T3_uub_v264_histo(buff,pack_size,pack_ver);
        if(dat->Histo==NULL){
          pack_list[ pack_type ] = false;
          unknown_pack.add_msg(pack_type,pack_ver,pack_size,(char *)buff);
        } else { 
          pack_list[ pack_type ] = true;
        }
        break;
      case T3_PACK_HISTO_COINC:
        dat->HistoCoinc = T3_uub_v264_histo_coinc(buff, pack_size, pack_ver);
        if (dat->HistoCoinc == NULL) {
          pack_list[ pack_type ] = false;
          unknown_pack.add_msg(pack_type, pack_ver, pack_size, (char *)buff);
        } else {
          pack_list[ pack_type ] = true;
        }
        break;
      case T3_PACK_CALIB:
        dat->Calib=T3_uub_v264_calib(buff,pack_size,pack_ver);
        if(dat->Calib==NULL){
          pack_list[ pack_type ] = false;
          unknown_pack.add_msg(pack_type,pack_ver,pack_size,(char *)buff); 
        } else {
          pack_list[ pack_type ] = true;
        }
        break;
      case T3_PACK_TTAG:
        if(T3_uub_v264_ttag(buff,pack_size,pack_ver,dat)){
          pack_list [ pack_type ] = false;
          unknown_pack.add_msg(pack_type,pack_ver,pack_size,(char *)buff); 
        } else {
          pack_list [ pack_type ] = true;
        }
        break;
      case T3_PACK_TRACE:
        if(T3_uub_v264_trace(buff,pack_size,pack_ver,dat)){
          pack_list [ pack_type ] = false;
          unknown_pack.add_msg(pack_type,pack_ver,pack_size,(char *)buff); 
        } else {
          pack_list [ pack_type ] = true;
        }
        break;
      case T3_PACK_EXTRA:
        if(T3_uub_v264_extra(buff,pack_size,pack_ver,dat->Extra)){
          pack_list [ pack_type ] = false;
          unknown_pack.add_msg(pack_type,pack_ver,pack_size,(char *)buff); 
        } else {
          pack_list [ pack_type ] = true;
        }
        break;
      case T3_PACK_RD:
        if(T3_uub_v264_rd(buff,pack_size,pack_ver,dat)){
          pack_list [ pack_type ] = false;
          unknown_pack.add_msg(pack_type,pack_ver,pack_size,(char *)buff); 
        } else {
          pack_list [ pack_type ] = true;
        }
        break;
      default:
        printf("Type not defined %d\n",pack_type);
        unknown_pack.add_msg(pack_type,pack_ver,pack_size,(char *)buff); 
      }
    }
  } while(0<=pack_size);
  if(pack_list[T3_PACK_TTAG]==false || 
     pack_list[T3_PACK_TRACE]==false){
    dat->Error |= IoSdEvent::eBadCompress;
  } else {// in case of no error, it would have set in in EbManager
    dat->Error |= IoSdEvent::eNoError;
  }
  printf("unknown packs: %d - %d\n",
         pack_list[T3_PACK_TRACE],unknown_pack.has_packs());
  if(pack_list[T3_PACK_TRACE]==true && unknown_pack.has_packs()){
    time_t t_pack;
    t_pack=time(NULL);
    unknown_pack.store(dat->Id, dat->Gps->Second, dat->Gps->Tick, t_pack);
  }
  
  dat->IsUUB = 1;
  return dat->Error;
}
