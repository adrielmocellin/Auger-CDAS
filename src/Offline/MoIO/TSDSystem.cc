#include "TSDSystem.h"

ClassImp(TSDSystem)

//______________________________________________________________________________
//
// TSDSystem is the base class containing the system informations
//
void TSDSystem::ResetSystem()
{
  //
  // Set all the members to 0

  fIsSystem = 0;

  fCal100   = 0;
  fCal40    = 0;
  fDeadTime = 0;
  fFreeDisk = 0;
  fFreeRam  = 0;
}

//______________________________________________________________________________
void TSDSystem::Dump()
{
  //
  // Dump on screen all the members

  DumpHeader();
  DumpMembers();
}

//______________________________________________________________________________
void TSDSystem::DumpHeader()
{
  //
  // Dump on screen the header for monitoring variables
  std::cout << std::endl << "####################    SYSTEM    #####################" << std::endl;
}

//______________________________________________________________________________
void TSDSystem::DumpMembers()
{ 
  //
  // Dump on screen the members of the TSDSystem class
  std::cout << "     DeadTime        : " <<fDeadTime << std::endl;
  std::cout << "     Free Disk Space : " << fFreeDisk;
  std::cout << "      Free Ram Space : " << fFreeRam << std::endl;
  std::cout << "     100 MHz         : " << fCal100;
  std::cout << "      40 MHz   : " << fCal40 << std::endl;

  std::cout << "    ______________________________________________________________" << std::endl;
}

