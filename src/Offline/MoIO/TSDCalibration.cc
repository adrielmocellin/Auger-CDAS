#include <iostream>

#include "TSDCalibration.h"

ClassImp(TSDCalibration)

//______________________________________________________________________________
//
// TSDCalibration is the base class defining the calibration variables
//
TSDCalibration::TSDCalibration()
{
  //
  // Constructor. Fill the name of the members
  //

  fListOfMembers.push_back("fVersion2");
  fListOfMembers.push_back("fTubeMask");
  fListOfMembers.push_back("fStartSecond");
  fListOfMembers.push_back("fEndSecond");
  fListOfMembers.push_back("fT1");
  fListOfMembers.push_back("fT2");
  fListOfMembers.push_back("fTotRate");

  fListOfMembers.push_back("fPast[0]");
  fListOfMembers.push_back("fPast[1]");
  fListOfMembers.push_back("fPast[2]");

  fListOfMembers.push_back("f70HzRate[0]");
  fListOfMembers.push_back("f70HzRate[1]");
  fListOfMembers.push_back("f70HzRate[2]");

  fListOfMembers.push_back("fAnode[0]");
  fListOfMembers.push_back("fAnode[1]");
  fListOfMembers.push_back("fAnode[2]");
  fListOfMembers.push_back("fVarianceAnode[0]");
  fListOfMembers.push_back("fVarianceAnode[1]");
  fListOfMembers.push_back("fVarianceAnode[2]");


  fListOfMembers.push_back("fDynode[0]");
  fListOfMembers.push_back("fDynode[1]");
  fListOfMembers.push_back("fDynode[2]");
  fListOfMembers.push_back("fVarianceDynode[0]");
  fListOfMembers.push_back("fVarianceDynode[1]");
  fListOfMembers.push_back("fVarianceDynode[2]");

  fListOfMembers.push_back("fTriggerDA[0]");
  fListOfMembers.push_back("fTriggerDA[1]");
  fListOfMembers.push_back("fTriggerDA[2]");

  fListOfMembers.push_back("fDynodeAnode[0]");
  fListOfMembers.push_back("fDynodeAnode[1]");
  fListOfMembers.push_back("fDynodeAnode[2]");
  fListOfMembers.push_back("fVarianceDynodeAnode[0]");
  fListOfMembers.push_back("fVarianceDynodeAnode[1]");
  fListOfMembers.push_back("fVarianceDynodeAnode[2]");

  fListOfMembers.push_back("fDynodeAnodeDt[0]");
  fListOfMembers.push_back("fDynodeAnodeDt[1]");
  fListOfMembers.push_back("fDynodeAnodeDt[2]");
  fListOfMembers.push_back("fVarianceDynodeAnodeDt[0]");
  fListOfMembers.push_back("fVarianceDynodeAnodeDt[1]");
  fListOfMembers.push_back("fVarianceDynodeAnodeDt[2]");
  fListOfMembers.push_back("fDynodeAnodeChi2[0]");
  fListOfMembers.push_back("fDynodeAnodeChi2[1]");
  fListOfMembers.push_back("fDynodeAnodeChi2[2]");

  fListOfMembers.push_back("fArea[0]");
  fListOfMembers.push_back("fArea[1]");
  fListOfMembers.push_back("fArea[2]");

  fListOfMembers.push_back("fPeak[0]");
  fListOfMembers.push_back("fPeak[1]");
  fListOfMembers.push_back("fPeak[2]");
}

//______________________________________________________________________________
void TSDCalibration::ResetCalibration()
{
  //
  // Sets all the members to 0
  //

  fIsCalibration = 0;

  fVersion = 1;

  fVersion2 = 0;
  fTubeMask = 0;

  fStartSecond = 0;
  fEndSecond = 0;

  fT1 = 0;
  fT2 = 0;

  fTotRate = 0;

  for (int i=0; i<3; i++) {
    fPast[i] = 0;
    f70HzRate[i] = 0;
    fAnode[i] = 0;
    fVarianceAnode[i] = 0;
    fDynode[i] = 0;
    fVarianceDynode[i] = 0;
    fTriggerDA[i] = 0;
    fDynodeAnode[i] = 0; 
    fVarianceDynodeAnode[i] = 0;
    fDynodeAnodeDt[i] = 0; 
    fVarianceDynodeAnodeDt[i] = 0;
    fDynodeAnodeChi2[i] = 0; 
    fArea[i] = 0;
    fPeak[i] = 0;
  }
}

//______________________________________________________________________________
void TSDCalibration::Dump()
{
  DumpHeader();
  DumpMembers();
}

//______________________________________________________________________________
void TSDCalibration::DumpHeader()
{
  std::cout << std::endl << "  ####################    CALIBRATION    #####################" << std::endl;
}

//______________________________________________________________________________
void TSDCalibration::DumpMembers()
{

  int j = 0;

  std::cout << "     " << GetCalibPrettyName(EVersion2) << " = " << fVersion2;
  std::cout << "   " << GetCalibPrettyName(ETubeMask) << " = " << fTubeMask << std::endl;

  std::cout << "     " << GetCalibPrettyName(EStartSecond) << " = " << fStartSecond;
  std::cout << "   " << GetCalibPrettyName(EEndSecond) << " = " << fEndSecond << std::endl;

  std::cout << "     " << GetCalibPrettyName(ET1) << " = " << fT1;
  std::cout << "   " << GetCalibPrettyName(ET2) << " = " << fT2 << std::endl;

  if ((fEndSecond - fStartSecond) > 0) {
    std::cout << "     <T1 rate> = " << fT1/(fEndSecond - fStartSecond) << " Hz";
    std::cout << "      <T2 rate> = " << fT2/(fEndSecond - fStartSecond) << " Hz";
    std::cout << "      <Tot rate> = " << fTotRate << " Hz";
    std::cout << std::endl;
  }

  for (int i=EPast1; i<=EPast3; i++) {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << fPast[j];
    j++;
  }
  std::cout << std::endl; j = 0;
  for (int i=ERate1; i<=ERate3; i++)  {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << f70HzRate[j];
    j++;
  }
  std::cout << std::endl; j = 0;
  for (int i=EAnode1; i<=EAnode3; i++)  {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << fAnode[j];
    j++;
  }
  std::cout << std::endl; j = 0;
  for (int i=EDynode1; i<=EDynode3; i++)  {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << fDynode[j];
    j++;
  }
  std::cout << std::endl; j = 0;
  for (int i=EVarianceAnode1; i<=EVarianceAnode3; i++)  {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << fVarianceAnode[j];
    j++;
  }
  std::cout << std::endl; j = 0;
  for (int i=EVarianceDynode1; i<=EVarianceDynode3; i++)  {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << fVarianceDynode[j];
    j++;
  }
  std::cout << std::endl; j = 0;
  for (int i=ETriggerDA1; i<=ETriggerDA3; i++)  {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << fTriggerDA[j];
    j++;
  }
  std::cout << std::endl; j = 0;
  for (int i=EDynodeAnode1; i<=EDynodeAnode3; i++)  {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << fDynodeAnode[j];
    j++;
  }
  std::cout << std::endl; j = 0;
  for (int i=EVarianceDynodeAnode1; i<=EVarianceDynodeAnode3; i++)  {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << fVarianceDynodeAnode[j];
    j++;
  }
  std::cout << std::endl; j = 0;
  for (int i=EArea1; i<=EArea3; i++)  {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << fArea[j];
    j++;
  }
  std::cout << std::endl; j = 0;
  for (int i=EPeak1; i<=EPeak3; i++)  {
    std::cout << "     " << GetCalibPrettyName((ECalib)i) << " = " << fPeak[j];
    j++;
  }
  std::cout << std::endl;

  std::cout << "    ______________________________________________________________" << std::endl;
}

//______________________________________________________________________________
std::string TSDCalibration::GetCalibName(ECalib index) 
{
  return fListOfMembers[index];
}

//______________________________________________________________________________
std::string TSDCalibration::GetCalibPrettyName(ECalib index) 
{
  std::string st = fListOfMembers[index];
  st.erase(st.find("f"),1);
  return st;
}
