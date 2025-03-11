#include "TSDMonitoring.h"

ClassImp(TSDMonitoring)

//______________________________________________________________________________
//
// TSDMonitoring is the base class defining the monitoring variables
//
void TSDMonitoring::ResetMonitoring()
{
  //
  // Set all the members to 0

 fIsMonitoring = 0;

 for (int i = 0; i< 3; i++) {
   fPMHV[i] = 0;
   fPMCur[i] = 0;
   fPMT[i] = 0;
 }
 fTElect = 0;
 fTWater = 0;
 fTFree = 0;
 
 for (int i = 0; i< 2; i++) fTBattery[i] = 0;
 fTAirIn = 0;
 fTAirOut = 0;
 for (int i = 0; i< 2; i++) fVBattery[i] = 0;
 for (int i = 0; i< 2; i++) fIBattery[i] = 0;
 for (int i = 0; i< 4; i++) fCloud[i] = 0;
 fPWater = 0;
 fPAtm = 0;
 fGround = 0;
 fBoard3V = 0;
 fBoard5V = 0;
 fBoard12V = 0;
 for (int i = 0; i< 2; i++) fVSolarPanel[i] = 0;
}

//______________________________________________________________________________
void TSDMonitoring::Dump()
{
  //
  // Dump on screen all the members

  DumpHeader();
  DumpMembers();
}

//______________________________________________________________________________
void TSDMonitoring::DumpHeader()
{
  //
  // Dump on screen the header for monitoring variables
  cout << endl << "####################    MONITORING    #####################" << endl;
}

//______________________________________________________________________________
void TSDMonitoring::DumpMembers()
{ 
  //
  // Dump on screen the members of the TSDMonitoring class
  cout << "     PM1 T : " << fPMT[0];
  cout << "     PM2 T : " << fPMT[1];
  cout << "     PM3 T : " << fPMT[2];
  cout << "     BoardT : " << fTElect << endl;
  cout << "     3 V   : " << fBoard3V;
  cout << "     5 V   : " << fBoard5V;
  cout << "     12V   : " << fBoard12V << endl;
  cout << "     VSolarPanel : " << fVSolarPanel[0];
  cout << "     VSolarPanel : " << fVSolarPanel[1];
  cout << "     IBattery   : " << fIBattery[0];
  cout << "     IBattery   : " << fIBattery[1]<< endl; 
  cout << "    ______________________________________________________________" << endl;
}

