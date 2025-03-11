#include "TSDRawMonitoring.h"

ClassImp(TSDRawMonitoring)

//______________________________________________________________________________
//
// TSDRawMonitoring is the base class defining the raw monitoring variables
//
TSDRawMonitoring::TSDRawMonitoring()
{
  // 
  // The constructor.

  fListOfMembers.push_back("fSecond");
  fListOfMembers.push_back("fSummary");
    
  fListOfMembers.push_back("fPMHV[0]");
  fListOfMembers.push_back("fPMHV[1]");
  fListOfMembers.push_back("fPMHV[2]");
  fListOfMembers.push_back("fPMCur[0]");     
  fListOfMembers.push_back("fPMCur[1]");     
  fListOfMembers.push_back("fPMCur[2]");     
  fListOfMembers.push_back("fPMT[0]");       
  fListOfMembers.push_back("fPMT[1]");       
  fListOfMembers.push_back("fPMT[2]");

  fListOfMembers.push_back("fTElect");     
  fListOfMembers.push_back("fTWater");     
  fListOfMembers.push_back("fTFree");      
  fListOfMembers.push_back("fTBattery[0]");      
  fListOfMembers.push_back("fTBattery[1]");      
  fListOfMembers.push_back("fTAirIn");     
  fListOfMembers.push_back("fTAirOut");   
  fListOfMembers.push_back("fVBattery[0]");      
  fListOfMembers.push_back("fVBattery[1]");      
  fListOfMembers.push_back("fIBattery[0]");     
  fListOfMembers.push_back("fIBattery[1]");     
  fListOfMembers.push_back("fCloud[0]");     
  fListOfMembers.push_back("fCloud[1]");     
  fListOfMembers.push_back("fCloud[2]");     
  fListOfMembers.push_back("fCloud[3]");     
  fListOfMembers.push_back("fPWater");     
  fListOfMembers.push_back("fPAtm");       
  fListOfMembers.push_back("fGround");     
  fListOfMembers.push_back("fBoard3V");    
  fListOfMembers.push_back("fBoard5V");    
  fListOfMembers.push_back("fBoard12V");
  fListOfMembers.push_back("fVSolarPanel[0]"); 
  fListOfMembers.push_back("fVSolarPanel[1]"); 
  
  // DAC Buttons
  fListOfMembers.push_back("fDACPMHV[0]");
  fListOfMembers.push_back("fDACPMHV[1]");
  fListOfMembers.push_back("fDACPMHV[2]");
  fListOfMembers.push_back("fDACLedPulse");
  fListOfMembers.push_back("fDACThreshold[0]");
  fListOfMembers.push_back("fDACThreshold[1]");
  fListOfMembers.push_back("fDACThreshold[2]");
  fListOfMembers.push_back("fDACThreshold[3]");
}

//______________________________________________________________________________
string TSDRawMonitoring::GetMonitName(EMonitor index)
{
  //
  // Returns the name of the member corresponding to the correct EMonitor enum
  return fListOfMembers[index];
}

//______________________________________________________________________________
string TSDRawMonitoring::GetMonitPrettyName(EMonitor index)
{
  //
  // Returns the pretty name (without f) of the member corresponding to the correct EMonitor enum
  string st = fListOfMembers[index];
  st.erase(st.find("f"),1);
  return st;
}

//______________________________________________________________________________
void TSDRawMonitoring::ResetMonitoring()
{
  //
  // Set all the members to 0

 fIsRawMonitoring = 0;

 fSecond = 0;
 fItemNb = 0;
 fSummary = 0;

 for (int i = 0; i<3; i++) {
   fPMHV[i] = 0;
   fPMCur[i] = 0;
   fPMT[i] = 0;
 }
 fTElect = 0;
 fTWater = 0;
 fTFree = 0;
 
 for (int i = 0; i<2; i++) fTBattery[i] = 0;
 fTAirIn = 0;
 fTAirOut = 0;
 for (int i = 0; i<2; i++) fVBattery[i] = 0;
 for (int i = 0; i<2; i++) fIBattery[i] = 0;
 for (int i = 0; i<4; i++) fCloud[i] = 0;
 fPWater = 0;
 fPAtm = 0;
 fGround = 0;
 fBoard3V = 0;
 fBoard5V = 0;
 fBoard12V = 0;
 for (int i = 0; i<2; i++) fVSolarPanel[i] = 0;
 fInputRegister = 0;
 for (int i = 0; i<3; i++) fDACPMHV[i] = 0;
 fDACLedPulse = 0;
 for (int i = 0; i<4; i++) fDACThreshold[i] = 0;
 fOutputRegister = 0;

}

//______________________________________________________________________________
void TSDRawMonitoring::Dump()
{
  //
  // Dump on screen all the members
  DumpHeader();
  DumpMembers();
}

//______________________________________________________________________________
void TSDRawMonitoring::DumpHeader()
{
  //
  // Dump on screen the header for raw monitoring variables
  cout << endl << "  ####################    MONITORING    #####################" << endl;
}

//______________________________________________________________________________
void TSDRawMonitoring::DumpMembers()
{
  //
  // Dump on screen the members of the TSDRawMonitoring class
  int j = 0;
  cout << "     " << GetMonitPrettyName(ESecond) << " = " << fSecond << endl;
  for (int i=EPMT1; i<=EPMT3; i++) {
    cout << "     " << GetMonitPrettyName((EMonitor)i) << " = " << fPMT[j];
    j++;
  }
  cout << "     " << GetMonitPrettyName(ETElect) << " = " << fTElect << endl;
  cout << "     " << GetMonitPrettyName(EBoard3V) << " = " << fBoard3V;
  cout << "     " << GetMonitPrettyName(EBoard5V) << " = " << fBoard5V;
  cout << "     " << GetMonitPrettyName(EBoard12V) << " = " << fBoard12V << endl;
  cout << "     " << GetMonitPrettyName(EVSolarPanel1) << " = " << fVSolarPanel[0];
  cout << "     " << GetMonitPrettyName(EVSolarPanel2) << " = " << fVSolarPanel[1];
  cout << "     " << GetMonitPrettyName(EIBattery1) << " = " << fIBattery[0];
  cout << "     " << GetMonitPrettyName(EIBattery2) << " = " << fIBattery[1]<< endl;
  cout << "    ______________________________________________________________" << endl;
}
