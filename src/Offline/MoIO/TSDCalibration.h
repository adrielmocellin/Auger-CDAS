#ifndef TSDCalibration_H
#define TSDCalibration_H

#include "TROOT.h"
#include "TObject.h"

#include <vector>
#include <string>

enum ECalib {
  EVersion2, ETubeMask,
  EStartSecond, EEndSecond,
  ET1, ET2,
  ETotRate,

  EPast1, EPast2, EPast3,
  ERate1, ERate2, ERate3,

  EAnode1, EAnode2, EAnode3,
  EVarianceAnode1, EVarianceAnode2, EVarianceAnode3,

  EDynode1, EDynode2, EDynode3,
  EVarianceDynode1, EVarianceDynode2, EVarianceDynode3,

  ETriggerDA1, ETriggerDA2, ETriggerDA3,

  EDynodeAnode1,  EDynodeAnode2,  EDynodeAnode3,
  EVarianceDynodeAnode1,  EVarianceDynodeAnode2,  EVarianceDynodeAnode3,

  EArea1, EArea2, EArea3,
  EPeak1, EPeak2, EPeak3,
  ECalibLast
};

class TSDCalibration {
 public:
  TSDCalibration();
  virtual ~TSDCalibration() {};
  void ResetCalibration();
  void Dump();
  void DumpHeader();
  void DumpMembers();

  std::string GetCalibName(ECalib);
  std::string GetCalibPrettyName(ECalib);

  UInt_t fVersion;  // A version number

  Bool_t fIsCalibration; // kTRUE if calibration block has been filled

  UShort_t fVersion2; // For the new calibration block
  UShort_t fTubeMask; // For the new calibration block

  UInt_t  fStartSecond; // Beginning of the calibration procedure
  UInt_t  fEndSecond;   // End of the calibration procedure

  Short_t fT1; // Number of T1 in the calibration procedure period

  Short_t fT2; // Number of T2 in the calibration procedure period
  Float_t fTotRate;  // Rate of the Time over threshold (Tot) triggers
  Short_t fTotD;
  Short_t fMops;


  Short_t fPast[3]; // Last 8 minutes calibration evolution

  Float_t fAnode[3];
  Float_t fVarianceDynode[3];

  Float_t fDynode[3];
  Float_t fVarianceAnode[3];

  Float_t fUBase[4];       //baseline for sPMT, spare and ssd low and high gain.
  Float_t fVarianceUBase[4]; //corresponding variance baseline for sPMT.

  Float_t f70HzRate[3]; //

  Short_t fTriggerDA[3]; // Number of triggers where both anode and dynodto compute Dynode/Anode

  Float_t fDynodeAnode[3];  // Dynode over Anode Ratio
  Float_t fVarianceDynodeAnode[3]; // Variance of the Dynode over Anode Ratio

  Float_t fDynodeAnodeDt[3];  // Dynode over Anode Dt
  Float_t fVarianceDynodeAnodeDt[3]; // Variance of the Dynode over Anode Dt
  Float_t fDynodeAnodeChi2[3];  // Dynode over Anode fit chi2

  Float_t fArea[3]; // VEM charge (ADC)
  Float_t fPeak[3]; // VEM peak (ADC)

  std::vector<std::string> fListOfMembers;

  ClassDef(TSDCalibration, 9 )
};

#endif
