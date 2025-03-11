
#include "TSDMonCal.h"

ClassImp(TSDMonCal)

//______________________________________________________________________________
//
// TSDMonCal is the class containing the monitoring and calibration to be stored in ROOT files
//
TSDMonCal::TSDMonCal()
{
  //
  // Constructor.
  fLsId=0;
  fTime=0;
  fCDASTime=0;
  fMonitoring.ResetMonitoring();
  fRawMonitoring.ResetMonitoring();
  fCalibration.ResetCalibration();
  fSystem.ResetSystem();
}

//______________________________________________________________________________
void TSDMonCal::Dump()
{
  //
  // Dump on screen all the members
  std::cout << std::endl << "    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
          std::cout << "    !!!               Monitoring and calibration                     !!!";
  std::cout << std::endl << "    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl << std::endl;

  std::cout << "       LS ID : " << fLsId << "    LS time : " << fTime << "    CDAS time : " << fCDASTime << std::endl << std::endl;

  fCalibration.DumpMembers();
  fRawMonitoring.DumpMembers();
  fMonitoring.DumpMembers();
  fSystem.DumpMembers();
  std::cout << std::endl << "    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
}
