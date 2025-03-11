#ifndef TSdMonitoring_H
#define TSdMonitoring_H

#include <string>
#include <vector>
#include <iostream>

#include "TROOT.h"
#include "TObject.h"

//______________________________________________________________________________
//
// EMonitor contains a list of enum related to the monitoring
//
enum EMonitor {
  //
  // EMonitor contains a list of enum related to the monitoring

  ESecond,       ESummary,

  EPMHV1,        EPMHV2,     EPMHV3,
  EPMCur1,       EPMCur2,    EPMCur3,
  EPMT1,         EPMT2,      EPMT3,
  ETElect,       ETWater,    ETFree,
  ETBattery1,    ETBattery2,
  ETAirIn,       ETAirOut,
  EVBattery1,    EVBattery2, EIBattery1, EIBattery2,
  ECloud1,       ECloud2,    ECloud3,    ECloud4,
  EPWater,       EPAtm,      EGround,
  EBoard3V,      EBoard5V,   EBoard12V,
  EVSolarPanel1, EVSolarPanel2,

  EDACPMHV1, EDACPMHV2, EDACPMHV3, EDACLedPulse,
  EDACTreshold1, EDACTreshold2, EDACTreshold3, EDACTreshold4,
  EMonitorLast
};


enum EWorkingMonitor {
  EWPMT1 = EPMT1, EWPMT2 = EPMT2, EWPMT3 = EPMT3,
  EWTElect = ETElect,
  EWBoard3V = EBoard3V, EWBoard5V = EBoard5V, EWBoard12V = EBoard12V,
  EWVSolarPanel1 = EVSolarPanel1, EWVSolarPanel2 = EVSolarPanel2,
  EWIBattery1 = EIBattery1, EWIBattery2 = EIBattery2,
  EWorkingMonitorLast
};


class TSDMonitoring {
  //
  // This class contains the physical monitoring data.
  //

 public:
  TSDMonitoring() {};
  virtual ~TSDMonitoring() {};
  void ResetMonitoring();
  void Dump();
  void DumpHeader();
  void DumpMembers();

  Bool_t   fIsMonitoring;   // kTRUE if monitoring block has been filled

  Float_t  fPMHV[3];        // PM high voltage 
  Float_t  fPMCur[3];       // PM current
  Float_t  fPMT[3];         // PM temperature
  Float_t  fTElect;         // Electronic temperature
  Float_t  fTWater;
  Float_t  fTFree;
  Float_t  fTBattery[2];    // Batery temperature
  Float_t  fTAirIn;
  Float_t  fTAirOut;
  Float_t  fVBattery[2];    // Batery voltage
  Float_t  fIBattery[2];    // Batery current
  Float_t  fCloud[4];
  Float_t  fPWater;
  Float_t  fPAtm;
  Float_t  fGround;
  Float_t  fBoard3V;        // Board voltage : 3V
  Float_t  fBoard5V;        // Board voltage : 5V
  Float_t  fBoard12V;       // Board voltage :12V
  Float_t  fVSolarPanel[2]; // Solar panel voltage

  ClassDef(TSDMonitoring,2)
};


#endif
