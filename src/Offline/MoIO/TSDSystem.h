#ifndef TSdSystem_H
#define TSdSystem_H

#include <iostream>

#include "TROOT.h"
#include "TObject.h"

//______________________________________________________________________________
//
// EMonitor contains a list of enum related to the monitoring
//
enum ESystem {
  //
  // ESystem contains a list of enum related to the system informations

  ECal100,   ECal40,
  EDeadTime,
  EFreeDisk, EFreeRam,
  ESystemLast
};

class TSDSystem 
{
  //
  // This class contains the data containing system information
  //

 public:
  TSDSystem() {};
  virtual ~TSDSystem() {};
  void ResetSystem();
  void Dump();
  void DumpHeader();
  void DumpMembers();

  Bool_t   fIsSystem;   // kTRUE if system block has been filled

  UInt_t fCal100;
  UInt_t fCal40;
  UInt_t fDeadTime;
  UInt_t fFreeDisk;
  UInt_t fFreeRam;

  ClassDef(TSDSystem,2)
};


#endif
