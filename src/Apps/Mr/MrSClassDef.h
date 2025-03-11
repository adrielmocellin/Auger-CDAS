#ifndef _MrS_CLASSDEF_H
#define _MrS_CLASSDEF_H

#include <TObject.h>

#include "MoIO_MrClassDef.h"
//#include "MoIO_CalibClassDef.h"

#define kMonitoringFields 32
#define kRawMonitoringFields 45
#define kCalibrationFields 30

#define kMaxStationNumber (2000+2048)

class TSDArray {
 public:
  TSDArray(){};
  ~TSDArray(){};

  TSDSummaryMonCal *GetOneLS(Int_t ls_id) {return &OneLS[ls_id];}

  void FillOneLSWithMrValues(TSDMonCal*, UInt_t);
  void FillArrayWithExistingValues(UInt_t, UInt_t);
  void ComputeAverage();
  void StoreData(Int_t flag);

 private:
  TSDSummaryMonCal OneLS[kMaxStationNumber];
};

#endif
