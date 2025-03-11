#ifndef __CbRawData__
#define __CbRawData__

#include "TSDCalibration.h"

class TCbRawData : public TSDCalibration {
  //
  // Inherits from TSDCalibration (defined in CDAS/MoIO)
  // Stores the calibration data
  //

 private:
  int SetData_UUB_v262(char *dat,int size);
 public:
  TCbRawData(){};
  ~TCbRawData(){};

  void Reset();
  float ComputeSigma(double, double, short);
  int SetData(char*);
  void Dump();
  bool fIsFilled;

  int fLsId;
  int fType;
  int fVersion;
};

#endif
