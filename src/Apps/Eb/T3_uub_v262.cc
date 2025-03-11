#include <arpa/inet.h>
#include "T3_uub.h"
int T3_uub_v262(IoSdStation *dat,uint8_t *buf,int nd)
{
  /* for version 262
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

  static int debug_evt_index=1000;
  char fname[100];
  FILE *fp;

  //sprintf(fname,"out/evt_%03d",debug_evt_index);
  //fp=fopen(fname,"w");
  //if(fp!=NULL){
  //  fwrite(buf,1,nd,fp);
  //  fclose(fp);
  //} else {
  //  printf("Not possible to store event\n");
  //}

  err=dat->Error; /* it has been assined with information of the beggining of
                   * message
                   */
  dat->Error |= IoSdEvent::eBadCompress;

  if(nd<4){
    return(IoSdEvent::eBadCompress);
  }
  lpt=(uint32_t *)buf;

  version=ntohl(*lpt++);
  if(version!=262){
    return(IoSdEvent::eBadCompress);
  }
  nnd=4;

  dat->IsUUB = 1;

  // In future, calib version will depend (maybe) on this version
  dat->Calib = new IoSdCalib();
  dat->Calib->Version=version;

  //2048 - number of timebins; 5-ADC; 10 additional parameter
  nnd += (2048*5 + 10) * sizeof(uint32_t);

  if( nd < nnd )
    return (IoSdEvent::eBadCompress);

  InternalId=ntohl(*lpt++);
  // Then 2 trigger types
  unsigned int type1=ntohl(*lpt++);
  unsigned int type2=ntohl(*lpt++);
  // Then trace start
  unsigned int tracestart=ntohl(*lpt++);
  // GPS basic data
  unsigned int gpssec=ntohl(*lpt++);
  unsigned int ticks=ntohl(*lpt++);
  unsigned int prevtics=ntohl(*lpt++);
  // offset and samples
  unsigned int offset=ntohl(*lpt++);
  unsigned int samples=ntohl(*lpt++);
  // filling data
  dat->Trigger.Type = 0; // For UUB, Type is 0 and Type is in Type1 and 2
  dat->Trigger.Type1 = type1;
  dat->Trigger.Type2 = type2;
  // UB stuff or unused yet
  dat->Fadc = NULL;
  // UUB FADC
  // the trace. It is version 3, mixed with GPS
  dat->UFadc = new IoUSdFadc();
  dat->UFadc->CodingVersion=3; // only current one
  dat->UFadc->TraceStart = tracestart;
  // Time stamp GPS : second, ticks, prevticks (which is 120MHz of prev sec)
  dat->Gps = new IoSdGps();
  dat->Gps->IsUUB = 1;
  dat->Gps->Second = gpssec;
  dat->Gps->Tick = ticks;
  dat->Gps->TickFall = prevtics;
  // basic GPS done
  // micro offset from Patrick, to the bin for UUB
  // FADC buffer
  dat->UFadc->NSample = samples;
  // check samples to 2048, else we could die making a wrong vector
  if (samples!=2048) {
    return IoSdEvent::eBadCompress;
  }
  // now dump the new array
  int nbval=dat->UFadc->NSample*10; // 5 channels, high and low
  dat->UFadc->Traces.reserve(nbval);
  spt=(uint16_t *)lpt;
  dat->UFadc->Traces.assign(spt, spt + nbval);
  spt=spt+nbval;
  lpt=(uint32_t *)spt;
  dat->UFadc->ShwrBufSt = ntohl(*lpt++);

  /* -------------------- */
  nnd += 7336;/* sizeof(uint32_t)*(4+4*69) + sizeof(uint8_t)*2*4 +
               *  sizeof(uint16_t)*(4*(6+20+150+600));
               */
  if(nd<nnd){
    return(IoSdEvent::eBadCompress);
  }

  dat->Histo = new IoSdHisto();

  uint32_t h_type;
  unsigned short Offset[4];
  uint8_t Pk_bit_shift[4];
  uint8_t Ch_bit_shift[4];

  uint16_t Pk_bin[4][2];
  uint16_t Ch_bin[4][2];

  float BaseAvg[4];

  int i,j;

  dat->Histo->type = ntohl(*lpt++);
  pt=(uint8_t *)lpt;
  for(j=0;j<4;j++)
    dat->Histo->Pk_bit_shift[j] = *pt++;

  for(j=0;j<4;j++)
    dat->Histo->Ch_bit_shift[j] = *pt++;

  spt=(uint16_t *)pt;
  for(i=0;i<2;i++)
    for(j=0;j<4;j++)
      dat->Histo->Pk_bin[i][j] = ntohs(*spt++);

  for(j=0;j<4;j++)
    for(i=0;i<2;i++)
      dat->Histo->Ch_bin[j][i] = ntohs(*spt++);

  for(j=0;j<4;j++)
    dat->Histo->BaseAvg[j] = ntohs(*spt++)/100.;

  for (i=0;i<4;i++)
    Offset[i]=ntohs(*spt++);
  // got the baselines used for all 4 channels: PMT 1-3 and SSD
  for (i=0;i<3;i++) {
    dat->Histo->Offset[i]=Offset[i]; // baselines
    dat->Histo->Offset[i+3]=Offset[i]; // peak
    dat->Histo->Offset[i+6]=
      (dat->Histo->Ch_bin[1][i] - dat->Histo->Ch_bin[0][j])*
      Offset[i]; // charge, 69 bins
  }
  dat->Histo->Offset[9]=69*Offset[3]; // charge SSD histogram, old. Actually is incorrect since SSD charge isn't computed on 69 bins
  // setting SSD histograms offset values;
  dat->Histo->Offset3[0]=Offset[3];
  dat->Histo->Offset3[1]=Offset[3];
  dat->Histo->Offset3[2]=
    Offset[3]*(dat->Histo->Ch_bin[1][3] - dat->Histo->Ch_bin[0][3]);
  for (i=0;i<3;i++)
    for (j=0;j<20;j++)
      dat->Histo->Base[i][j] = ntohs(*spt++);
  for (j=0;j<20;j++)
    dat->Histo->Base3[j] = ntohs(*spt++); //special treatment of SSD baseline histogram
  for (i=0;i<3;i++)
    for (j=0;j<150;j++)
      dat->Histo->Peak[i][j] = ntohs(*spt++);
  for (j=0;j<150;j++)
    dat->Histo->Peak3[j] = ntohs(*spt++); // special treatment of SSD peak histogram
  for (i=0;i<4;i++)
    for (j=0;j<600;j++)
      dat->Histo->Charge[i][j] = ntohs(*spt++);
  lpt=(uint32_t *)spt;
  for (i=0;i<4;i++)
    for (j=0;j<69;j++)
      dat->Histo->UShape[i][j] = ntohl(*lpt++);

  dat->Histo->tStart     =ntohl(*lpt++);
  dat->Histo->tEnd       =ntohl(*lpt++);
  dat->Histo->Entries   =ntohl(*lpt++);

  nnd += 8 ; /* sizeof(uint32_t) + 4 */
  if(nd<nnd){
    return(IoSdEvent::eBadCompress);
  }
  // more complete GPS data and extra values
  // for simplicity, the old GPS structure is used
  // the Next100 hosts the Next120
  // and the 2 ST are used as before
  dat->Gps->Next100 = ntohl(*lpt++);
  signed char * sc=(signed char *)lpt;
  // keeping prev prev ST in 40MHz temporarily
  dat->Gps->Current40  = (*sc++);
  dat->Gps->PreviousST = (*sc++);
  dat->Gps->CurrentST  = (*sc++);
  dat->Gps->NextST = (*sc++);

  lpt=(uint32_t *)sc;
  //========= calibration data =================

  spt=(uint16_t *)sc;
  lpt=(uint32_t *)sc;

  nnd += 104; /*sizeof(uint16_t)*(2+4+20+4*6)+sizeof(uint32_t)+4 */
  if(nd<nnd){
    return(IoSdEvent::eBadCompress);
  }

  //dat->Calib->Version      = ntohs(*spt++); //already set at beginning
  spt++; //it is version in the calibration structure.
  dat->Calib->TubeMask     = ntohs(*spt++);
  lpt = (uint32_t *)spt;
  dat->Calib->StartSecond  = ntohl(*lpt++);
  spt = (uint16_t *)lpt;
  dat->Calib->EndSecond    = ntohs(*spt++);
  dat->Calib->NbTOTD       = ntohs(*spt++);
  dat->Calib->NbMOPS       = ntohs(*spt++);
  *spt++; //skip the number of external trigger.
  dat->Calib->NbT1         = ntohs(*spt++);
  dat->Calib->NbT2         = ntohs(*spt++);
  dat->Calib->NbTOT        = ntohs(*spt++);
  dat->Calib->NbFullBuff   = ntohs(*spt++);

  for(i=0;i<6;i++)
    dat->Calib->Base[i]       = ntohs(*spt++)/100.;
  for(i=0;i<4;i++)
    dat->Calib->UBase[i]    = ntohs(*spt++)/100.;
  for(i=0;i<6;i++)
    dat->Calib->SigmaBase[i]  = ntohs(*spt++)/100.;
  for(i=0;i<4;i++)
    dat->Calib->SigmaUBase[i]= ntohs(*spt++)/100.;

  for(i=0;i<3;i++)
    dat->Calib->Evolution[i] = ntohs(*spt++);
  *spt++; //skip the Evolution for SSD.
  for(i=0;i<3;i++)
    dat->Calib->VemPeak[i]    = ntohs(*spt++)/10.;
  *spt++; //skip the "VEM" for SSD - not calculated yet
  for(i=0;i<3;i++)
    dat->Calib->Rate[i]      = ntohs(*spt++)/100.;
  *spt++; //skip the Rate for SSD - not calculated yet
  for(i=0;i<3;i++)
    dat->Calib->DA[i]       = ntohs(*spt++)/100.; //it is HG/LG ratio.
  *spt++; //skip the HG/LG for SSD - not calculated yet
  for(i=0;i<3;i++)
    dat->Calib->VemCharge[i]        = ntohs(*spt++)/10.;
  *spt++; //skip the "VEM Charge" for SSD - not calculated yet

  lpt=(uint32_t *)spt;

  //========= t1 parameters  (39*sizeof(uint32_t) ================

  nnd +=sizeof(uint32_t)*39;// 39 is to total number of trigger configuration.
  if(nd<nnd){
    return(IoSdEvent::eBadCompress);
  }
  dat->UTrigParam=new IoUSdTrigParam();

  dat->UTrigParam->PL_version      = ntohl(*lpt++);
  dat->UTrigParam->TrigMask        = ntohl(*lpt++);
  dat->UTrigParam->csbt_th[0]      = ntohl(*lpt++);
  dat->UTrigParam->csbt_th[1]      = ntohl(*lpt++);
  dat->UTrigParam->csbt_th[2]      = ntohl(*lpt++);
  dat->UTrigParam->csbt_enable     = ntohl(*lpt++);
  dat->UTrigParam->ctot_th[0]      = ntohl(*lpt++);
  dat->UTrigParam->ctot_th[1]      = ntohl(*lpt++);
  dat->UTrigParam->ctot_th[2]      = ntohl(*lpt++);
  dat->UTrigParam->ctot_enable     = ntohl(*lpt++);
  dat->UTrigParam->ctot_occ        = ntohl(*lpt++);
  dat->UTrigParam->ctotd_thmin[0]  = ntohl(*lpt++);
  dat->UTrigParam->ctotd_thmin[1]  = ntohl(*lpt++);
  dat->UTrigParam->ctotd_thmin[2]  = ntohl(*lpt++);
  dat->UTrigParam->ctotd_thmax[0]  = ntohl(*lpt++);
  dat->UTrigParam->ctotd_thmax[1]  = ntohl(*lpt++);
  dat->UTrigParam->ctotd_thmax[2]  = ntohl(*lpt++);
  dat->UTrigParam->ctotd_enable    = ntohl(*lpt++);
  dat->UTrigParam->ctotd_occ       = ntohl(*lpt++);
  dat->UTrigParam->ctotd_fd        = ntohl(*lpt++);
  dat->UTrigParam->ctotd_fn        = ntohl(*lpt++);
  dat->UTrigParam->ctotd_int       = ntohl(*lpt++);
  dat->UTrigParam->cmops_thmax[0]  = ntohl(*lpt++);
  dat->UTrigParam->cmops_thmax[1]  = ntohl(*lpt++);
  dat->UTrigParam->cmops_thmax[2]  = ntohl(*lpt++);
  dat->UTrigParam->cmops_thmin[0]  = ntohl(*lpt++);
  dat->UTrigParam->cmops_thmin[1]  = ntohl(*lpt++);
  dat->UTrigParam->cmops_thmin[2]  = ntohl(*lpt++);
  dat->UTrigParam->cmops_enable    = ntohl(*lpt++);
  dat->UTrigParam->cmops_occ       = ntohl(*lpt++);
  dat->UTrigParam->cmops_ofs       = ntohl(*lpt++);
  dat->UTrigParam->cmops_int       = ntohl(*lpt++);
  dat->UTrigParam->sbt_th[0]       = ntohl(*lpt++);
  dat->UTrigParam->sbt_th[1]       = ntohl(*lpt++);
  dat->UTrigParam->sbt_th[2]       = ntohl(*lpt++);
  dat->UTrigParam->sbt_th[3]       = ntohl(*lpt++);
  dat->UTrigParam->sbt_enable      = ntohl(*lpt++);
  dat->UTrigParam->led_ctrl        = ntohl(*lpt++);
  dat->UTrigParam->random_mode     = ntohl(*lpt++);

  // 32 extra unsigned ints
  nnd += 128 ;   /*sizeof(uint32_t)*32 */
  if(nd<nnd){
    return(IoSdEvent::eBadCompress);
  }

  for (int i=0;i<32;i++) {
    dat->Extra[i] = ntohl(*lpt++);
  }
  nnd += 4 ;/*sizeof(uint32_t) */
  if(nd<nnd){
    return(IoSdEvent::eBadCompress);
  }
  dat->UFadc->RdBufSt = ntohl(*lpt++);

  if( dat->UFadc->RdBufSt ) {
    nnd += 8192; /* 2*2048*uint(32_t)...; */
    if( nnd < nd ) {
      return(IoSdEvent::eBadCompress);
    }
    // one extra trace for radio data
    int nbval=dat->UFadc->NSample*12; // 5*2 high/low ch + 2 radio ch
    dat->UFadc->Traces.reserve(nbval);
    spt=(uint16_t *)lpt;
    for (int i=0;i<2048*2;i++)
      dat->UFadc->Traces.push_back(*spt++);
  }
  dat->Error = err;//IoSdEvent::eNoError;
  return dat->Error;
}
