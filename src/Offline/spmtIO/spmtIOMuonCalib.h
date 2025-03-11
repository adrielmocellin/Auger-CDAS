#ifndef _SPMTIOMUONCALIB_H_
#define _SPMTIOMUONCALIB_H_

#include <TObject.h>

class spmtIOMuonCalib : public TObject
{
public:
  spmtIOMuonCalib(){};
  virtual ~spmtIOMuonCalib(){};

  UInt_t ChargeCalibTime; //GPS start second of VEM/MIP calibration
  UInt_t Charge[4];

  ClassDef(spmtIOMuonCalib,1)
};

#endif
