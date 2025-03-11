#ifndef _SPMTIO_H_
#define _SPMTIO_H_

#include <TObject.h>
#include <iostream>
#include <vector>

#include "spmtIOThreshold.h"
#include "spmtIOEvt.h"
#include "spmtIOMuonCalib.h"

class spmtIO : public TObject
{
 public:

  spmtIO(){ };
  virtual ~spmtIO(){ };
  void ResetData();
  void Print() const;

  UInt_t LsId;
  std::vector<spmtIOEvt> evt;
  std::vector<spmtIOMuonCalib> muon;
  std::vector<spmtIOThreshold> thres;

  ClassDef(spmtIO,1)
};
#endif
