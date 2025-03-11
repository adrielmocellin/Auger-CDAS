#ifndef MoIO_MrClassDef_H
#define MoIO_MrClassDef_H

#include <string>
#include <vector>
#include <iostream>

#include "TROOT.h"
#include "TObject.h"

#include "TSDMonitoring.h"

class TSDRawMonitoring {
  //
  // This class contains the raw monitoring data
  //

 public:
  TSDRawMonitoring();
  virtual ~TSDRawMonitoring() {};

  void ResetMonitoring();
  void Dump();
  void DumpHeader();
  void DumpMembers();

  string GetMonitName(EMonitor);
  string GetMonitPrettyName(EMonitor);

  vector<string> fListOfMembers; // Vector containing the list of the members

  Bool_t   fIsRawMonitoring; // kTRUE if monitoring block has been filled

  UInt_t   fSecond;
  UInt_t   fItemNb;
  UInt_t   fSummary;

  UShort_t fPMHV[3];         // PM high voltage
  UShort_t fPMCur[3];        // PM current
  UShort_t fPMT[3];          // PM temperature
  UShort_t fTElect;
  UShort_t fTWater;
  UShort_t fTFree;
  UShort_t fTBattery[2];     // Batery temperature
  UShort_t fTAirIn;
  UShort_t fTAirOut;
  UShort_t fVBattery[2];     // Batery voltage
  UShort_t fIBattery[2];     // Batery current
  UShort_t fCloud[4];
  UShort_t fPWater;
  UShort_t fPAtm;
  UShort_t fGround;
  UShort_t fBoard3V;         // Board voltage : 3V
  UShort_t fBoard5V;         // Board voltage : 5V
  UShort_t fBoard12V;        // Board voltage :12V
  UShort_t fVSolarPanel[2];  // Solar panel voltage
  UShort_t fInputRegister;

  UShort_t fDACPMHV[3];
  UShort_t fDACLedPulse;
  UShort_t fDACThreshold[4]; // Be careful : the 3 last fields are in reality the DAC PM HV !!!!!!!!!!!
  UShort_t fOutputRegister;

  ClassDef(TSDRawMonitoring,2)
};

#endif
