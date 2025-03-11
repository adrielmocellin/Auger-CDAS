#ifndef __MoRawData__
#define __MoRawData__

#include <iostream>
#include <endian.h>

#include "TSDRawMonitoring.h" // In MoIO
#include "TSDSystem.h" // In MoIO
#include "monitor.h"

#define kNADC 32 // Number of ADC in UB
#define kNDAC 8  // Number of DAC in UB

typedef struct {
#if __BYTE_ORDER == __BIG_ENDIAN
        unsigned char ADCValueLow;
        unsigned char ADCNotInUse:1;
        unsigned char WarningOrange:1;
        unsigned char WarningRed:1;
        unsigned char dummy:1;
        unsigned char ADCValueHigh:4;
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
        unsigned char ADCValueLow;
        unsigned char ADCValueHigh:4;
        unsigned char dummy:1;
        unsigned char WarningRed:1;
        unsigned char WarningOrange:1;
        unsigned char ADCNotInUse:1;
#endif
} TADC_ByteLevel;

class TADC {
  //
  // Stores the data from one ADC
  //

 public:
  TADC() {};
  ~TADC() {};

  unsigned short fAdc;
  bool fWarningRed;
  bool fWarningOrange;
  bool fADCNotInUse;

  void Reset() {
    fAdc=0; fWarningRed=0; fWarningOrange=0; fADCNotInUse=0;
  }

};

class TMoRawData : public TSDRawMonitoring, public TSDSystem {
  //
  // Inherits from TSDRawMonitoring (defined in MoIO)
  // The TSDRawMonitoring part contained the raw monitoring data
  // To store the physical monitoring data, a member is defined
  // (cannot inherits from TSDMonitoring directly because the 
  // members have the same name) 
  //

  public:
  TMoRawData() {};
  ~TMoRawData() {};

  void Reset();
  void FillADC(unsigned short*);
  int SetData(char*);
  void FillPhysADC();
  void SdAdcToPhys();

  void FillMonitoringData();
  void Dump();

  bool fIsFilled;
  int fLsId;
  int fType;
  int fVersion;

  // ADC 
  TADC fADC[kNADC]; // Stores Raw ADC values

  // DAC
  unsigned short fDAC[kNDAC]; // Stores DAC values

  // Physical ADC
  double fPhysADC[kNADC]; // Stores physical values
  TSDMonitoring fMonitoring; // To store the physical monitoring data before saving them
};
#endif
