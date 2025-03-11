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

  std::string GetMonitName(EMonitor);
  std::string GetMonitPrettyName(EMonitor);

  std::vector<std::string> fListOfMembers; // Vector containing the list of the members

  Bool_t   fIsRawMonitoring; // kTRUE if monitoring block has been filled

  UInt_t   fSecond; // LS GPS time corresponding to the monitoring block building time
  UInt_t   fItemNb; 
  UInt_t   fSummary;


  Bool_t   fIsUUB;   // kTRUE if is a UUB

  // Photomultiplier, UB and UUB

  UShort_t fPMV[3];          // PM voltage (V)
  UShort_t fPMI[3];          // PM current (uA)
  UShort_t fPMT[3];          // PM temperature (C)

  // Electronic box, UB and UUB

  UShort_t fElectT;          // Electronic box temperature (C)

  // Batteries, UB and UUB

  UShort_t fBatteryT[2];     // Battery temperatures (C) - TPCB
  UShort_t fBatteryV[2];     // Battery voltage ([0] for battery center and [1] for total, so [1]-[0] for the other battery voltage) (V) - TPCB
  UShort_t fCurrentLoad;     // current consumed by the LS (mA) - TPCB

  // Solar Panel, UB and UUB

  UShort_t fSolarPanelV;     // Solar panel voltage (V) - TPCB
  UShort_t fSolarPanelI;     // Solar panel current (A) - TPCB

  // Water, UB and UUB

  UShort_t fWaterLevel;      // Level of the water (V) - TPCB
  UShort_t fWaterT;          // Water temperature (C) - TPCB

  // radio, UB and UUB
  UShort_t f12VRadio;       // 12V going to radio (V)

  // Power Supply and others, UB only
  UShort_t f3V;              // 3.3V  Power supply (V)
  UShort_t f_3V;             // -3.3V Power supply (V)
  UShort_t f5V;              // 5V  Power supply (V)
  UShort_t f12V;             // 12V Power supply (V)
  UShort_t f24V;             // 24V Power supply (V)
  UShort_t fADCBaseline;     // Baseline of all of the ADC measurements (ch)
  UShort_t fDAC4Voltage;     // Voltage of the Led Flasher (V)
  UShort_t f3VAnalogPower;   // 3.3V analog power (V)
  UShort_t f12VMultiplexer;  // 12V before the multiplexer (V)
  UShort_t fDACPM[3];        // DAC controling the PM high voltage
  UShort_t fDACLedVoltage;   // LED flasher voltage

  // UUB specific - Photomultipliers [0]->sPMT; [1]->SSD PMT;[2]->spare.
  UShort_t fUPMV[3];          // PM voltage (V)
  UShort_t fUPMI[3];          // PM current (uA)
  UShort_t fUPMT[3];          // PM temperature (C)

  //  UUB specific -Environment
  UShort_t fExtT;               // Exterior temperature
  UShort_t fElectP;             // inside the electronics box
  UShort_t fElectH;             // 

  //  UUB specific -UUB ADC
  UShort_t fUADC[9];

  // UUB Extra
  UShort_t fExtra[8];
  // Voltages
  UShort_t f12V_WT, f12V_BATTEMP[2], f12V_WL, f12VPM, f24VExt[2], f3V3,
    f5VGPS,f3V3Analog,f_3V3Analog,f1V0,f1V2,f1V8,f10V;

  // Currents
  UShort_t f12VRadioI,f12VPMI,f3V3AnalogI,f_3V3AnalogI,f5VGPSI,f1V0I,f1V2I,f1V8I,f3V3I,fVInI,f3V3SCI,fILED;

  ClassDef(TSDRawMonitoring,9)
};

#endif
