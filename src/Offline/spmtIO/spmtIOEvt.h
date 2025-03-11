#ifndef _SPMTIOEVT_H_
#define _SPMTIOEVT_H_

#include <TObject.h>

class spmtIOEvt: public TObject
{
 public:
  spmtIOEvt(){};
  virtual ~spmtIOEvt(){};
  UInt_t GPSsec;
  UInt_t GPSTicks;
  UInt_t charge_FADC[6];//0...2: WCD PMT1, 2, 3; 3: sPMT; 4,5: SSD LG, HG
  UInt_t peak_FADC[6];
  UInt_t saturation[6];

  ClassDef(spmtIOEvt,1)
};
#endif
