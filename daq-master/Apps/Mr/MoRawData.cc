#include <stdio.h>
#include <string>

#include <IkC.h>
#include <IkWarning.hxx>

#include <Pm.h>

#include "MoRawData.h"

extern UInt_t gLastMonitBookingTime[];

void TMoRawData::Reset(){
  //
  // Sets the members to 0
  //

  fIsFilled = 0;

  fLsId     = 0;
  fType     = 0;
  fVersion  = 0;
  fSecond   = 0;
  fItemNb   = 0;
  fSummary  = 0;

  for (int i=0; i<kNADC; i++) {
    fADC[i].Reset();
    fPhysADC[i]=0;
  }
  //  fInputRegister = 0;

  for (int i=0; i<kNDAC; i++) fDAC[i] = 0;
  //  fOutputRegister = 0;

  TSDRawMonitoring::ResetMonitoring();
  TSDSystem::ResetSystem();
  fMonitoring.ResetMonitoring();

}

void TMoRawData::FillADC(unsigned short *ptrs) {
  //
  // Fill all the ADC classes with adc values
  //

  unsigned short adc;
  TADC_ByteLevel *temp;

  for (int i=0; i<kNADC; i++) {
    adc = ntohs(*ptrs++);
    temp = (TADC_ByteLevel *)&adc;

    fADC[i].fAdc = (unsigned short)(temp->ADCValueHigh*256+temp->ADCValueLow);
    fADC[i].fWarningRed = (bool)temp->WarningRed;
    fADC[i].fWarningOrange = (bool)temp->WarningOrange;
    fADC[i].fADCNotInUse = (bool)temp->ADCNotInUse;
  }
}

void TMoRawData::FillPhysADC()
{
  //
  // This routine converts the Raw ADC values contained in TADC to
  // physical ADC values For the preproduction tanks, we have the
  // conversion parameters stored locally Have to call for the
  // calibration parmaters of the station stored in Db for the EA
  // tanks
  //


//  if (fLsId >= 100) {
//    if (fLsId >= 71) { // Modified 04/10/2006
  if (fLsId >= 11) { // Modified 04/12/2012 - incorporate the stations of AERA
    //
    // We are dealing with pre-production or production tanks
    //
    SdAdcToPhys();
  } else {
    //
    // We are dealing with engineering array tanks, ignore...
    //
  }
}


void TMoRawData::SdAdcToPhys()
{
  //
  // Converts the ADC valuies into physical ones
  // Constants values are the default ones for preproduction
  //
  // The formula used is : phys = adc*slope + offset;

  fPhysADC[0] = (fADC[0].fAdc)*0.001221;
  fPhysADC[1] = (fADC[1].fAdc)*0.002442;
  fPhysADC[2] = (fADC[2].fAdc)*0.0049206;
  fPhysADC[3] = (fADC[3].fAdc)*0.013431;
  fPhysADC[4] = (fADC[4].fAdc)*0.1221 - 273.15;
  fPhysADC[5] = (fADC[5].fAdc)*0.1221 - 273.15;
  fPhysADC[6] = (fADC[6].fAdc)*0.001172;
  fPhysADC[7] = (fADC[7].fAdc)*0.0127188;
  fPhysADC[8] = (fADC[8].fAdc)*0.1221 - 273.15;
  fPhysADC[9] = (fADC[9].fAdc)*0.254375; // This value has been multiplied by a 10 factor to have the value in mA
  fPhysADC[10] = (fADC[10].fAdc)*0.002442 - fPhysADC[1]; // Have to subtract the 5V to the -3.3V
  //fPhysADC[11] = (fADC[11].fAdc)*0.018315/2.; // Have to divide by 2 to find the correct values ||  OLD VALUES - BUG FIX Cyril 16/03/2006
  fPhysADC[11] = (fADC[11].fAdc)*0.0046520147; // new values 16/03/2006
  //fPhysADC[12] = (fADC[12].fAdc)*0.0091575/2.; OLD VALUES - BUG FIX Cyril 16/03/2006
  fPhysADC[12] = (fADC[12].fAdc)*0.0095238095; // new values 16/03/2006
  fPhysADC[13] = (fADC[13].fAdc)*0.4884; //This is 2000V/5V * 5V/(2^12-1)
  fPhysADC[14] = (fADC[14].fAdc)*0.0049206;
  fPhysADC[15] = (fADC[15].fAdc)*0.0049206;
  fPhysADC[16] = (fADC[16].fAdc)*0.02446896;
  fPhysADC[17] = (fADC[17].fAdc)*0.1221 - 273.15;
  fPhysADC[18] = (fADC[18].fAdc)*0.001221;
  fPhysADC[19] = (fADC[19].fAdc)*0.001221;
  fPhysADC[20] = (fADC[20].fAdc)*0.001221;
  fPhysADC[21] = (fADC[21].fAdc)*0.02446896;
  fPhysADC[22] = (fADC[22].fAdc)*0.4884;
  fPhysADC[23] = (fADC[23].fAdc)*0.1221 - 273.15;
  fPhysADC[24] = (fADC[24].fAdc)*0.1221 - 273.15;
  fPhysADC[25] = (fADC[25].fAdc)*0.02446896;
  fPhysADC[26] = (fADC[26].fAdc)*1.;
  fPhysADC[27] = (fADC[27].fAdc)*0.4884;
  fPhysADC[28] = (fADC[28].fAdc)*0.1221 - 273.15;

}

int TMoRawData::SetData(char* data)
{
  //
  // Converts the monitoring Pm message in data.
  // Call the conversion to physical value.
  //

  PmLsRawMessage *rawP = (PmLsRawMessage *)(data+sizeof(longWord));
  PmLsMessage *messP = &(rawP->mess);

  fLsId = ntohl(rawP->lsId);
  fType = ntohl(messP->type);
  fVersion = ntohl(messP->version);
  longWord messize = ntohl(messP->length);

  unsigned int *ptr = (unsigned int *)&(messP->data);
  ptr+=1; // zap the first word of the monitoring block (i.e. requestId)

  unsigned int bufferOK = ntohl(*ptr++);

  if (bufferOK != 0) {
    cout << endl << endl << " Corrupted monitoring block from station"
         << fLsId << endl << endl;
    return -1;
  }
// UUB
  if (messize == 176) { // Should be UUB, don't go further in the
                        // routine, fill the data
    fSecond = ntohl(*ptr++);
    if (fSecond>1160000000) { // check time is in 2016+
      fIsUUB = kTRUE;
      fIsRawMonitoring = 1;
      unsigned short *ptrs = (unsigned short*)ptr;

      fUPMV[0]=ntohs(*ptrs++);
      fBatteryT[1]=ntohs(*ptrs++);
      f12VRadio=ntohs(*ptrs++);
      fUADC[7]=ntohs(*ptrs++);
      fPMT[0]=ntohs(*ptrs++);
      f12VRadioI=ntohs(*ptrs++);
      fUPMV[2]=ntohs(*ptrs++);
      fILED=ntohs(*ptrs++);        //SC: I_24V_LED
      fPMV[2]=ntohs(*ptrs++);
      fBatteryT[0]=ntohs(*ptrs++);
      f12VPM=ntohs(*ptrs++);
      fUADC[6]=ntohs(*ptrs++);
      fPMT[1]=ntohs(*ptrs++);
      f12VPMI=ntohs(*ptrs++);
      fUPMV[1]=ntohs(*ptrs++);
      fVInI=ntohs(*ptrs++);
      fPMV[1]=ntohs(*ptrs++);
      fCurrentLoad=ntohs(*ptrs++);
      f24VExt[1]=ntohs(*ptrs++);
      fUADC[5]=ntohs(*ptrs++);
      fPMT[2]=ntohs(*ptrs++);
      f3V3AnalogI=ntohs(*ptrs++);  //SC: I_P5V_ANA
      fUPMI[2]=ntohs(*ptrs++);
      f3V3SCI=ntohs(*ptrs++);
      fPMV[0]=ntohs(*ptrs++);
      fBatteryV[0]=ntohs(*ptrs++); //SC: BAT_CENT
      f24VExt[0]=ntohs(*ptrs++);
      fUADC[4]=ntohs(*ptrs++);
      fUPMT[0]=ntohs(*ptrs++);
      f_3V3AnalogI=ntohs(*ptrs++); //SC: I_N5V_ANA
      fUPMI[1]=ntohs(*ptrs++);
      f1V0I=ntohs(*ptrs++);
      fUPMI[0]=ntohs(*ptrs++);
      fWaterT=ntohs(*ptrs++);      //SC: EXT_TEMP - from TPCB.
      fSolarPanelI=ntohs(*ptrs++);
      fUADC[0]=ntohs(*ptrs++);
      f3V3=ntohs(*ptrs++);
      f5VGPSI=ntohs(*ptrs++);
      fUPMT[2]=ntohs(*ptrs++);
      f12V_BATTEMP[0]=ntohs(*ptrs++);
      fPMI[2]=ntohs(*ptrs++);
      f12V_WT=ntohs(*ptrs++);
      fSolarPanelV=ntohs(*ptrs++);
      fUADC[1]=ntohs(*ptrs++);
      f5VGPS=ntohs(*ptrs++);
      f1V2I=ntohs(*ptrs++);
      fUPMT[1]=ntohs(*ptrs++);
      f1V8=ntohs(*ptrs++);
      fPMI[1]=ntohs(*ptrs++);
      f12V_WL=ntohs(*ptrs++);
      fBatteryV[1]=ntohs(*ptrs++); //SC: BAT_OUT
      fUADC[2]=ntohs(*ptrs++);
      f3V3Analog=ntohs(*ptrs++);   //SC: V_AN_P5V
      f3V3I=ntohs(*ptrs++);
      f10V=ntohs(*ptrs++);
      f1V0=ntohs(*ptrs++);
      fPMI[0]=ntohs(*ptrs++);
      f12V_BATTEMP[1]=ntohs(*ptrs++);
      fWaterLevel=ntohs(*ptrs++);
      fUADC[3]=ntohs(*ptrs++);
      f_3V3Analog=ntohs(*ptrs++);  //SC:  V_AN_N5V
      f1V8I=ntohs(*ptrs++);
      fUADC[8]=ntohs(*ptrs++);
      f1V2=ntohs(*ptrs++);

      fElectP=ntohs(*ptrs++);
      fElectT=ntohs(*ptrs++);
      fElectH=ntohs(*ptrs++);
      fExtT=ntohs(*ptrs++);        //SC: not implemented.
      // 8 extra values
      for (int i=0;i<8;i++)
        fExtra[i]=ntohs(*ptrs++);

      // Physical values, from SlowControl.c from KHB. Some are missing
#define LSB_TO_5V  1.8814
#define LSB_TO_24V 8.88
#define LSB_TO_12V 4.43
#define LSB_TO_3V3 1.20
#define LSB_TO_1V8 0.674
#define LSB_TO_1V2 0.421
#define LSB_TO_1V0 0.366

      for (int pmt=0;pmt<4;pmt++) {
        fMonitoring.fPMT[pmt]=fPMT[pmt]*LSB_TO_5V-273.15;
        fMonitoring.fPMV[pmt]=fPMV[pmt]*LSB_TO_5V;
        fMonitoring.fPMI[pmt]=fPMI[pmt]*LSB_TO_5V;
        fMonitoring.fUPMT[pmt]=fUPMT[pmt]*LSB_TO_5V-273.15;
        fMonitoring.fUPMV[pmt]=fUPMV[pmt]*LSB_TO_5V;
        fMonitoring.fUPMI[pmt]=fUPMI[pmt]*LSB_TO_5V;
      }
      fMonitoring.f1V0=f1V0*LSB_TO_1V0;
      fMonitoring.f1V2=f1V2*LSB_TO_1V2;
      fMonitoring.f1V8=f1V8*LSB_TO_1V8;
      fMonitoring.f1V0I=f1V0I*LSB_TO_1V0/60.*41.67;
      fMonitoring.f1V2I=f1V2I*LSB_TO_1V0/60.*10.;
      fMonitoring.f1V8I=f1V8I*LSB_TO_1V0/60.*30.3;
      fMonitoring.fBatteryV[0]=fBatteryV[0]*LSB_TO_5V*18./5000.;
      fMonitoring.fBatteryV[1]=fBatteryV[1]*LSB_TO_5V*36./5000.;
      fMonitoring.fCurrentLoad=fCurrentLoad*LSB_TO_5V/48.;
      fMonitoring.f3V3=f3V3*LSB_TO_3V3;
      fMonitoring.f3V3I=f3V3I*LSB_TO_3V3/60.*16.13;
      fMonitoring.f3V3SCI=f3V3SCI*LSB_TO_1V0/60.*12.2;
      fMonitoring.f3V3Analog=f3V3Analog*LSB_TO_3V3;  // This is 5V, not 3.3
      fMonitoring.f3V3AnalogI=f3V3AnalogI*LSB_TO_3V3/60.*12.2;
      float Ua=f_3V3Analog*LSB_TO_3V3;
      fMonitoring.f_3V3Analog=
        Ua*2.-(10./7.5 * (2500.-Ua)); // This is -5V, not -3.3
      fMonitoring.f_3V3AnalogI=f_3V3AnalogI*LSB_TO_3V3/60.*12.2;
      fMonitoring.f5VGPS=f5VGPS*LSB_TO_5V;
      fMonitoring.f5VGPSI=f5VGPSI*LSB_TO_1V0/60.*10;
      fMonitoring.f12VRadio=f12VRadio*LSB_TO_12V;
      fMonitoring.f12VRadioI=f12VRadioI*LSB_TO_1V0/60.*30.3;
      fMonitoring.f12VPM=f12VPM*LSB_TO_12V;
      fMonitoring.f12VPMI=f12VPMI*LSB_TO_1V0/60.*30.3;
      fMonitoring.f24VExt[0]=f24VExt[0]*LSB_TO_24V;
      fMonitoring.f24VExt[1]=f24VExt[1]*LSB_TO_24V;
      fMonitoring.fVInI=fVInI*LSB_TO_1V0/60.*21.28;
      fMonitoring.fBatteryT[0]=fBatteryT[0]*LSB_TO_5V-273.15;
      fMonitoring.fBatteryT[1]=fBatteryT[1]*LSB_TO_5V-273.15;
      fMonitoring.fExtT=fExtT*LSB_TO_5V-273.15;
      fMonitoring.fSolarPanelV=fSolarPanelV*LSB_TO_5V*50./5000.;
      fMonitoring.fSolarPanelI=fSolarPanelI*LSB_TO_5V*5./1000.;
      // End of physical values

      fIsFilled = 1;
      gLastMonitBookingTime[fLsId]=(UInt_t)time((time_t *)NULL);
      return 0;
    } else return -1;
  }
  //  if ((fLsId>70 && messize != 180)|| (fLsId<71 && messize != 116)) { // EA is different
  if ((fLsId>=11 && messize != 180)|| (fLsId<11 && messize != 116)) { // it is just to include the AERA stations (2012/Dec/11)
    cout << " Corrupted monitoring block from LS " << fLsId << " invalid size " << messize << endl;
    FILE *file2 = fopen("/Raid/var/Mr_log/DebugXBRR.txt","a");
    fprintf(file2,"#BAD-mo-size %d %d\n",fLsId,messize);
    fclose(file2);
    return -1;
  }

  fSecond = ntohl(*ptr++);
  fIsUUB = kFALSE;

#if 0
  bool junkblock = 0;
  if (fSecond < 758000000 || fSecond > 770000000) {
    cout << endl << endl << " JUNK MONITORING BLOCK " << fSecond << " "
         << data << endl << endl;
    junkblock = 1;
  }
#endif

  fItemNb = ntohl(*ptr++);
  fSummary = ntohl(*ptr++);

  unsigned short *ptrs = (unsigned short*)ptr;

  // ADC
  FillADC(ptrs); // Fill all the ADC with their values
  for (int i=0; i<kNADC; i++) *ptrs++; // Positioning the pointer at the end of the ADC
  /* UShort_t inputRegister = */ ntohs(*ptrs++);

  // DAC
  for (int i=0; i<kNDAC; i++) fDAC[i] = ntohs(*ptrs++);
  /*UShort_t outputRegister = */ ntohs(*ptrs++);

  ptr = (unsigned int*)ptrs;

  // Now decode the System part if fVersion >= 4
  if ( fVersion == 4 || fVersion == 0) {
    fCal100   = ntohl(*ptr++);
    fCal40    = ntohl(*ptr++);
    fDeadTime = ntohl(*ptr++);
    fFreeDisk = ntohl(*ptr++);
    fFreeRam  = ntohl(*ptr++);
  }

  FillPhysADC(); // Converts the raw data to physical data (using Db)
  FillMonitoringData(); // Converts the tables to fields (see below)
#if 0
  if (junkblock == 1) {
    TSDRawMonitoring::DumpHeader();
    TSDRawMonitoring::Dump();
  }
#endif
  fIsFilled = 1;
  gLastMonitBookingTime[fLsId]=(UInt_t)time((time_t *)NULL);
  return 0;
}

void TMoRawData::FillMonitoringData()
{
  //
  // The value stored in the ADC, DAC and PhysADC tables
  // are stored in the classes TSDRawMonitoring and TSDMonitoring.
  //

  //  if (fLsId >= 100) {
  //  if (fLsId >= 71) { // Modified 04/10/2006
  if (fLsId >=11){ //Modified 2012/Dec/11
    //
    // We are dealing with pre-production or production tanks
    // Have to use the new data format
    //

    fIsRawMonitoring = 1;

    f3V             = fADC[0].fAdc;
    f5V             = fADC[1].fAdc;
    f12V            = fADC[2].fAdc;
    f24V            = fADC[3].fAdc;
    fElectT         = fADC[4].fAdc;
    fBatteryT[0]    = fADC[5].fAdc;
    fSolarPanelI    = fADC[6].fAdc;
    fSolarPanelV    = fADC[7].fAdc;
    fBatteryT[1]    = fADC[8].fAdc;
    fCurrentLoad    = fADC[9].fAdc;
    f_3V            = fADC[10].fAdc;
    fBatteryV[0]    = fADC[11].fAdc;
    fBatteryV[1]    = fADC[12].fAdc;
    fPMV[0]         = fADC[13].fAdc;
    f12VMultiplexer = fADC[14].fAdc;
    f12VRadio       = fADC[15].fAdc;
    fPMI[0]         = fADC[16].fAdc;
    fWaterT         = fADC[17].fAdc;
    f3VAnalogPower  = fADC[18].fAdc;
    fWaterLevel     = fADC[19].fAdc;
    fDAC4Voltage    = fADC[20].fAdc;
    fPMI[2]         = fADC[21].fAdc;
    fPMV[2]         = fADC[22].fAdc;
    fPMT[1]         = fADC[23].fAdc;
    fPMT[2]         = fADC[24].fAdc;
    fPMI[1]         = fADC[25].fAdc;
    fADCBaseline    = fADC[26].fAdc;
    fPMV[1]         = fADC[27].fAdc;
    fPMT[0]         = fADC[28].fAdc;

    fDACPM[0]       = fDAC[0];
    fDACPM[1]       = fDAC[1];
    fDACPM[2]       = fDAC[2];
    fDACLedVoltage  = fDAC[3];

    fMonitoring.fIsMonitoring = 1;

    fMonitoring.f3V             = fPhysADC[0];
    fMonitoring.f5V             = fPhysADC[1];
    fMonitoring.f12V            = fPhysADC[2];
    fMonitoring.f24V            = fPhysADC[3];
    fMonitoring.fElectT         = fPhysADC[4];
    fMonitoring.fBatteryT[0]    = fPhysADC[5];
    fMonitoring.fSolarPanelI    = fPhysADC[6];
    fMonitoring.fSolarPanelV    = fPhysADC[7];
    fMonitoring.fBatteryT[1]    = fPhysADC[8];
    fMonitoring.fCurrentLoad    = fPhysADC[9];
    fMonitoring.f_3V            = fPhysADC[10];
    fMonitoring.fBatteryV[0]    = fPhysADC[11];
    fMonitoring.fBatteryV[1]    = fPhysADC[12];
    fMonitoring.fPMV[0]         = fPhysADC[13];
    fMonitoring.f12VMultiplexer = fPhysADC[14];
    fMonitoring.f12VRadio       = fPhysADC[15];
    fMonitoring.fPMI[0]         = fPhysADC[16];
    fMonitoring.fWaterT         = fPhysADC[17];
    fMonitoring.f3VAnalogPower  = fPhysADC[18];
    fMonitoring.fWaterLevel     = fPhysADC[19];
    fMonitoring.fDAC4Voltage    = fPhysADC[20];
    fMonitoring.fPMI[2]         = fPhysADC[21];
    fMonitoring.fPMV[2]         = fPhysADC[22];
    fMonitoring.fPMT[1]         = fPhysADC[23];
    fMonitoring.fPMT[2]         = fPhysADC[24];
    fMonitoring.fPMI[1]         = fPhysADC[25];
    fMonitoring.fADCBaseline    = fPhysADC[26];
    fMonitoring.fPMV[1]         = fPhysADC[27];
    fMonitoring.fPMT[0]         = fPhysADC[28];
  }
  else {
    //
    // We are dealing with engineering array tanks
    // Have to use the old data format
    //

    fIsRawMonitoring = 1;

    fPMT[0]             = fADC[0].fAdc;
    //                  = fADC[1].fAdc;
    //                  = fADC[2].fAdc;
    fPMT[1]             = fADC[3].fAdc;
    //                  = fADC[4].fAdc;
    //                  = fADC[5].fAdc;
    //                  = fADC[6].fAdc;
    //                  = fADC[7].fAdc;
    //                  = fADC[8].fAdc;
    //                  = fADC[9].fAdc;
    fElectT             = fADC[10].fAdc;
    f3V                 = fADC[11].fAdc;
    //                  = fADC[12].fAdc;
    //                  = fADC[13].fAdc;
    fPMT[2]             = fADC[14].fAdc;
    //                  = fADC[15].fAdc;
    fSolarPanelV        = fADC[16].fAdc + fADC[19].fAdc;
    // Current Battery                 = fADC[17].fAdc;
    //                  = fADC[18].fAdc;
    // Already use for fSolarPanelV    = fADC[19].fAdc;
    // Current Battery                 = fADC[20].fAdc;
    fBatteryV[1]        = fADC[21].fAdc + fADC[23].fAdc; // Now fBatteryV[1] must store the total battery voltage
    //                  = fADC[22].fAdc;
    fBatteryV[0]        = fADC[23].fAdc;
    f5V                 = fADC[24].fAdc;
    //                  = fADC[25].fAdc;
    //                  = fADC[26].fAdc;
    f12V                = fADC[27].fAdc;
    //                  = fADC[28].fAdc;

    fDACPM[0]       = fDAC[5]; // In EA DAC's end stores PM HV
    fDACPM[1]       = fDAC[6];
    fDACPM[2]       = fDAC[7];
    fDACLedVoltage  = fDAC[3];

    fMonitoring.fIsMonitoring = 1;

    fMonitoring.fPMT[0]         = fPhysADC[0];
    //fMonitoring.fPMCur[1]       = fPhysADC[1];
    //fMonitoring.fPMCur[2]       = fPhysADC[2];
    fMonitoring.fPMT[1]         = fPhysADC[3];
    //fMonitoring.fPMCur[0]       = fPhysADC[4];
    //fMonitoring.fPMHV[2]        = fPhysADC[5];
    //fMonitoring.fPMHV[0]        = fPhysADC[6];
    //fMonitoring.fPMHV[1]        = fPhysADC[7];
    //fMonitoring.fGround         = fPhysADC[8];
    //fMonitoring.fTFree          = fPhysADC[9];
    fMonitoring.fElectT         = fPhysADC[10];
    fMonitoring.f3V             = fPhysADC[11];
    //fMonitoring.fTWater         = fPhysADC[12];
    //fMonitoring.fTAirOut        = fPhysADC[13];
    fMonitoring.fPMT[2]         = fPhysADC[14];
    //fMonitoring.fTAirIn         = fPhysADC[15];
    fMonitoring.fSolarPanelV    = fPhysADC[16] + fPhysADC[19];
    //fMonitoring.fIBattery[1]    = fPhysADC[17];
    //fMonitoring.fTBattery[1]    = fPhysADC[18];
    //fMonitoring.fVSolarPanel[1] = fPhysADC[19];
    //fMonitoring.fIBattery[0]    = fPhysADC[20];
    fMonitoring.fBatteryV[1]    = fPhysADC[21] + fPhysADC[23]; // Now fBatteryV[1] must store the total battery voltage
    //fMonitoring.fTBattery[0]    = fPhysADC[22];
    fMonitoring.fBatteryV[0]    = fPhysADC[23];
    fMonitoring.f5V             = fPhysADC[24];
    //fMonitoring.fPWater         = fPhysADC[25];
    //fMonitoring.fPAtm           = fPhysADC[26];
    fMonitoring.f12V            = fPhysADC[27];
  }
}

void TMoRawData::Dump() {
  //
  // Dump on screen
  //

  cout << endl << " fLsId = " << fLsId;
  cout << "     fType = " << fType;
  cout << "     fVersion = " << fVersion << endl;

  cout << endl << " fSecond = " << fSecond;
  cout << "     fItemNb = " << fItemNb;
  cout << "     fSummary = " << fSummary << endl << endl;

  int counter=0;
  bool flag=0;
  for (int i=0; i<kNADC; i++) {
    counter++;
    if (flag == 0)
      cout << " fADC[" << i << "] = " << fADC[i].fAdc << "     ";
    else {
      cout << " fADC[" << i << "] = " << fADC[i].fAdc << endl;
      flag=0; counter=0;
    }
    if (counter==3) flag=1;
  }
  cout << endl;

  counter = 0;
  flag = 0;
  for (int i=0; i<kNDAC; i++) {
    counter++;
    if (flag == 0)
      cout << " fDAC[" << i << "] = " << fDAC[i] << "     ";
    else {
      cout << " fDAC[" << i << "] = " << fDAC[i] << endl;;
      flag=0; counter=0;
    }
    if (counter==3) flag=1;
  }

  cout << endl;
  //  cout << " fInputRegister = " << fInputRegister;
  //  cout << "             fOutputRegister = " << fOutputRegister << endl;
  cout << endl;

  counter = 0;
  flag = 0;
  for (int i=0; i<kNADC; i++) {
    counter++;
    if (flag == 0)
      cout << " fADC[" << i << "] = " << fPhysADC[i] << "     ";
    else {
      cout << " fADC[" << i << "] = " << fPhysADC[i] << endl;
      flag=0; counter=0;
    }
    if (counter==3) flag=1;
  }
  cout << "______________________________________________________________" << endl;
}
