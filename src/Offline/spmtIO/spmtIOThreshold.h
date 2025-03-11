#ifndef _SPMTIOTHREHOLD_H_
#define _SPMTIOTHREHOLD_H_

#include <TObject.h>

class spmtIOThreshold: public TObject
{
 public:
  spmtIOThreshold(){};
  virtual ~spmtIOThreshold(){};
  UInt_t time;// GPS time of the threshold change for the event selection
  UInt_t mask;
  UInt_t Th[3]; // thresholds for the event selection
  Float_t HG_LG[3];
  UInt_t VEM[4];

  ClassDef(spmtIOThreshold,1)

};
#endif
