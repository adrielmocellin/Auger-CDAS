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
  std::cout << std::endl << "####################    MONITORING    #####################" << std::endl;
}

//______________________________________________________________________________
void TSDMonitoring::DumpMembers()
{ 
  //
  // Dump on screen the members of the TSDMonitoring class
  std::cout << "     ** Power supply **" << std::endl;
  std::cout << "     3.3 V   : " << f3V;
  std::cout << "     -3.3 V   : " << f_3V << std::endl;
  std::cout << "     5 V   : " << f5V;
  std::cout << "     12V   : " << f12V;
  std::cout << "     24V   : " << f24V << std::endl<< std::endl;

  std::cout << "     ** Photomultipliers **" << std::endl;
  std::cout << "     PM1 V : " << fPMV[0];
  std::cout << "     PM2 V : " << fPMV[1];
  std::cout << "     PM3 V : " << fPMV[2] << std::endl;
  std::cout << "     PM1 I : " << fPMI[0];
  std::cout << "     PM2 I : " << fPMI[1];
  std::cout << "     PM3 I : " << fPMI[2] << std::endl;
  std::cout << "     PM1 T : " << fPMT[0];
  std::cout << "     PM2 T : " << fPMT[1];
  std::cout << "     PM3 T : " << fPMT[2] << std::endl << std::endl;

  std::cout << "     ** Batteries **" << std::endl;
  std::cout << "     Battery Center V : " << fBatteryV[0];
  std::cout << "     Battery Total V : " << fBatteryV[1] << std::endl;
  std::cout << "     Battery1 T : " << fBatteryT[0];
  std::cout << "     Battery2 T : " << fBatteryT[1] << std::endl << std::endl;

  std::cout << "     ** Solar panels **" << std::endl;
  std::cout << "     Solar Panel V : " << fSolarPanelV;
  std::cout << "     Solar Panel I : " << fSolarPanelI << std::endl << std::endl;

  std::cout << "     ** Water **" << std::endl;
  std::cout << "     Water Temperature : " << fWaterT;
  std::cout << "     Water Level : " << fWaterLevel << std::endl << std::endl;


  std::cout << "     ** Others **" << std::endl;
  std::cout << "     Elect T : " << fElectT;
  std::cout << "     Current Load : " << fCurrentLoad << std::endl;;
  std::cout << "     ADC BaseLine : " << fADCBaseline;
  std::cout << "     DAC4 Voltage : " << fDAC4Voltage << std::endl;
  std::cout << "     3V Analog Power : " << f3VAnalogPower;
  std::cout << "     12V Multiplexer : " << f12VMultiplexer;
  std::cout << "     12V Radio : " << f12VRadio << std::endl;
  std::cout << "    ______________________________________________________________" << std::endl;
}

