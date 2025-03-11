#ifndef TSdMonitoring_H
#define TSdMonitoring_H

#include <string>
//#include <vector>
#include <iostream>

#include "TROOT.h"
#include "TObject.h"

//______________________________________________________________________________
//
// EMonitor contains a list of enum related to the monitoring
//
enum EMonitor {
  //
  // EMonitor contains a list of enum related to the UB monitoring (not UUB)

  ESecond,        ESummary,

  E3V,            E_3V,
  E5V,            E12V,       E24V,

  EPMV1,          EPMV2,      EPMV3,
  EPMI1,          EPMI2,      EPMI3,
  EPMT1,          EPMT2,      EPMT3,

  EElectT,
  EBatteryT1,     EBatteryT2,
  EBatteryV1,     EBatteryV2,

  ESolarPanelV,   ESolarPanelI,

  EWaterLevel,    EWaterT,

  ECurrentLoad,   EADCBaseLine,    EDAC4Voltage,
  E3VAnalogPower, E12VMultiplexer, E12VRadio,

  EDACPM1,        EDACPM2,         EDACPM3,
  EDACLedVoltage,
  EMonitorLast
};

/*
enum EWorkingMonitor {
  EWPMT1 = EPMT1, EWPMT2 = EPMT2, EWPMT3 = EPMT3,
  EWTElect = ETElect,
  EWBoard3V = EBoard3V, EWBoard5V = EBoard5V, EWBoard12V = EBoard12V,
  EWVSolarPanel1 = EVSolarPanel1, EWVSolarPanel2 = EVSolarPanel2,
  EWIBattery1 = EIBattery1, EWIBattery2 = EIBattery2,
  EWorkingMonitorLast
};
*/

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

  Bool_t   fIsUUB;   // kTRUE if is a UUB

  // Photomultiplier, UB and UUB

  Float_t fPMV[3];          // PM voltage (V)
  Float_t fPMI[3];          // PM current (uA)
  Float_t fPMT[3];          // PM temperature (C)

  // Electronic box, UB and UUB

  Float_t fElectT;          // Electronic box temperature (C)

  // Batteries, UB and UUB

  Float_t fBatteryT[2];     // Battery temperatures (C)
  Float_t fBatteryV[2];     // Battery voltage ([0] for battery center and [1] for total, so [1]-[0] for the other battery voltage) (V)
  Float_t fCurrentLoad;     // current consumed by the LS (mA)

  // Solar Panel, UB and UUB

  Float_t fSolarPanelV;     // Solar panel voltage (V)
  Float_t fSolarPanelI;     // Solar panel current (A)

  // Water, UB and UUB

  Float_t fWaterLevel;      // Level of the water (V)
  Float_t fWaterT;          // Water temperature (C)

  // radio, UB and UUB
  Float_t f12VRadio;       // 12V going to radio (V)

  // Power Supply and others, UB only
  Float_t f3V;              // 3.3V  Power supply (V)
  Float_t f_3V;             // -3.3V Power supply (V)
  Float_t f5V;              // 5V  Power supply (V)
  Float_t f12V;             // 12V Power supply (V)
  Float_t f24V;             // 24V Power supply (V)
  Float_t fADCBaseline;     // Baseline of all of the ADC measurements (ch)
  Float_t fDAC4Voltage;     // Voltage of the Led Flasher (V)
  Float_t f3VAnalogPower;   // 3.3V analog power (V)
  Float_t f12VMultiplexer;  // 12V before the multiplexer (V)

  // UUB specific
  // Photomultipliers
  Float_t fUPMV[3];          // PM voltage (V)
  Float_t fUPMI[3];          // PM current (uA)
  Float_t fUPMT[3];          // PM temperature (C)

  // Environment
  Float_t fExtT;             // Exterior temperature
  Float_t fElectP;          // Air pressure
  Float_t fElectH;          // Electronic box temperature (C)


  // ADC
  Float_t fADC[9];

  // Voltages
  Float_t f12V_WT, f12VPMT[3], f12VPM, f24VExt[2],f3V3,f5VGPS,f3V3Analog,f_3V3Analog,f5VUSB,f1V0,f1V2,f1V8,f10V;

  // Currents
  Float_t f12VRadioI,f12VPMI,f3V3AnalogI,f_3V3AnalogI,f5VGPSI,f1V0I,f1V2I,f1V8I,f3V3I,fVInI,f3V3SCI,fILED;

  ClassDef(TSDMonitoring,7)
};


#endif
