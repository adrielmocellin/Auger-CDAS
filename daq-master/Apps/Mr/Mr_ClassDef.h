#ifndef Mr_ClassDef_H
#define Mr_ClassDef_H

#include "TSDMonCal.h" // In MoIO

#include "MoRawData.h"
#include "CbRawData.h"

class TSDMrMonCal : public TSDMonCal {
  //
  // This class is used to write the data in the root file
  // in the format described by TSDMonCal (defined in MoIO)
  //
 public:
  TSDMrMonCal(int,TMoRawData*,TCbRawData*);
};

#endif
