
#ifndef _TSDRD_H_
#define _TSDRD_H_

#include <stdint.h>

class TSDRd
{
 public:
  TSDRd(){}
  ~TSDRd(){}
  uint32_t hasMonitRd;
  uint32_t ElectrSerial;
  uint32_t ElectrFirm;
  uint32_t UubRdFirm;
  int16_t adc[4];
  
};
#endif
