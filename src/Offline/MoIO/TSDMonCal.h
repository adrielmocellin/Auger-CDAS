#ifndef TSdMonCal_H
#define TSdMonCal_H

#include "TROOT.h"
#include "TObject.h"

#include "TSDMonitoring.h"
#include "TSDRawMonitoring.h"
#include "TSDCalibration.h"
#include "TSDSystem.h"

class TSDMonCal : public TObject {
  //
  // The class which contains the monitoring and calibration data
  // of the Pierre Auger Observatory
  //

 public:
  TSDMonCal();

  void Dump();

  Int_t  fLsId; // ID of the station
  UInt_t fTime; // time of the LS 
  UInt_t fCDASTime; // time when CDAS receive the Monit block 

  TSDMonitoring    fMonitoring;    // Contains the monitoring part
  TSDRawMonitoring fRawMonitoring; // Contains the raw monitoring part
  TSDCalibration   fCalibration;   // Contains the calibration part
  TSDSystem        fSystem;        // Contains the system part

  ClassDef(TSDMonCal,5)
};

#endif
