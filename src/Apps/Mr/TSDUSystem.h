#ifndef _TSDSYSTEM_H_
#define _TSDSYSTEM_H_
#include <stdint.h>

class TSDUSystem
{
public:
  TSDUSystem(){}
  ~TSDUSystem(){}
  
  uint16_t UUBSerial;
  uint16_t radioSerial;
  char gpsSerial[8];
  
  uint32_t firmware_ver;
  uint32_t soft_ver;
  
};
#endif
