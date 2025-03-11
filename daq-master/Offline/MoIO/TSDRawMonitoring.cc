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

  fListOfMembers.push_back("f3V");
  fListOfMembers.push_back("f_3V");
  fListOfMembers.push_back("f5V");
  fListOfMembers.push_back("f12V");
  fListOfMembers.push_back("f24V");
    
  fListOfMembers.push_back("fPMV[0]");
  fListOfMembers.push_back("fPMV[1]");
  fListOfMembers.push_back("fPMV[2]");
  fListOfMembers.push_back("fPMI[0]");     
  fListOfMembers.push_back("fPMI[1]");     
  fListOfMembers.push_back("fPMI[2]");     
  fListOfMembers.push_back("fPMT[0]");       
  fListOfMembers.push_back("fPMT[1]");       
  fListOfMembers.push_back("fPMT[2]");

  fListOfMembers.push_back("fElectT");

  fListOfMembers.push_back("fBatteryT[0]");
  fListOfMembers.push_back("fBatteryT[1]");
  fListOfMembers.push_back("fBatteryV[0]");
  fListOfMembers.push_back("fBatteryV[1]");

  fListOfMembers.push_back("fSolarPanelV");
  fListOfMembers.push_back("fSolarPanelI");

  fListOfMembers.push_back("fWaterLevel");
  fListOfMembers.push_back("fWaterT");

  fListOfMembers.push_back("fCurrentLoad");
  fListOfMembers.push_back("fADCBaseline");
  fListOfMembers.push_back("fDAC4Voltage");
  fListOfMembers.push_back("f3VAnalogPower");
  fListOfMembers.push_back("f12VMultiplexer");
  fListOfMembers.push_back("f12VRadio");
  
  // DAC Buttons
  fListOfMembers.push_back("fDACPM[0]");
  fListOfMembers.push_back("fDACPM[1]");
  fListOfMembers.push_back("fDACPM[2]");
  fListOfMembers.push_back("fDACLedVoltage");
 
  // put properly fIsUUB on new events ?
  fIsUUB = kFALSE;
}

//______________________________________________________________________________
std::string TSDRawMonitoring::GetMonitName(EMonitor index)
{
  //
  // Returns the name of the member corresponding to the correct EMonitor enum
  return fListOfMembers[index];
}

//______________________________________________________________________________
std::string TSDRawMonitoring::GetMonitPrettyName(EMonitor index)
{
  //
  // Returns the pretty name (without f) of the member corresponding to the correct EMonitor enum
  std::string st = fListOfMembers[index];
  st.erase(st.find("f"),1);
  return st;
}

//______________________________________________________________________________
void TSDRawMonitoring::ResetMonitoring()
{
  //
  // Set all the members to 0
 fIsUUB = kFALSE;

 fIsRawMonitoring = 0;

 fSecond = 0; fItemNb = 0; fSummary = 0;

 f3V = 0; f_3V = 0; f5V = 0; f12V = 0; f24V = 0;

 for (int i = 0; i<3; i++) {fPMV[i] = 0; fPMI[i] = 0; fPMT[i] = 0;}

 fElectT = 0;
 
 for (int i = 0; i<2; i++) {fBatteryT[i] = 0; fBatteryV[i] = 0;}

 fSolarPanelV = 0; fSolarPanelI = 0;

 fWaterLevel = 0; fWaterT = 0;

 fCurrentLoad    = 0;
 fADCBaseline    = 0;
 fDAC4Voltage    = 0;
 f3VAnalogPower  = 0;
 f12VMultiplexer = 0;
 f12VRadio       = 0;

 for (int i = 0; i<3; i++) fDACPM[i] = 0;
 fDACLedVoltage = 0;
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
  std::cout << std::endl << "  ####################    MONITORING    #####################" << std::endl;
}

//______________________________________________________________________________
void TSDRawMonitoring::DumpMembers()
{
  //
  // Dump on screen the members of the TSDRawMonitoring class
  int j = 0;
  std::cout << "     " << GetMonitPrettyName(ESecond) << " = " << fSecond << std::endl;
  for (int i=EPMT1; i<=EPMT3; i++) {
    std::cout << "     " << GetMonitPrettyName((EMonitor)i) << " = " << fPMT[j];
    j++;
  }
  std::cout << "     " << GetMonitPrettyName(EElectT) << " = " << fElectT << std::endl;
  std::cout << "     " << GetMonitPrettyName(E3V) << " = " << f3V;
  std::cout << "     " << GetMonitPrettyName(E5V) << " = " << f5V;
  std::cout << "     " << GetMonitPrettyName(E12V) << " = " << f12V << std::endl;
  std::cout << "     " << GetMonitPrettyName(ESolarPanelV) << " = " << fSolarPanelV;
  std::cout << "     " << GetMonitPrettyName(ESolarPanelI) << " = " << fSolarPanelI<< std::endl;
  std::cout << "    ______________________________________________________________" << std::endl;
}
