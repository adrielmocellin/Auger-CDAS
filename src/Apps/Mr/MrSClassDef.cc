#include <iostream.h>
#include <math.h>
#include "TROOT.h"
#include "TObject.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h" 

#include "MrSClassDef.h"
#include "MrSSummaryFile.h"


// Class TWholeArray functions
void TSDArray::FillOneLSWithValues(TSDMonData* datatostorefrommor,TSDMonRawData* rawdatatostorefrommor,UInt_t atime) {

  TADCArrayForOneStation *ls;
  TFloatValueToStore *headervalues;
  TFloatValueToStore *oneadc;
  TFloatValueToStore *oneadcraw;

  UInt_t ADCindex = 0;
  if (datatostorefrommor->fSd_Id < 0 || datatostorefrommor->fSd_Id > gMaxStationNumber) {return;}
  ls = GetOneLS(datatostorefrommor->fSd_Id);
  ls->NewEntry();

  ls->SetLSId(datatostorefrommor->fSd_Id);
  ls->SetTime(atime);

  //ls->SetSecond(datatostorefrommor->fSecond);
  headervalues = ls->GetOneHeader(0); headervalues->AddOneADCValue(datatostorefrommor->fSecond);
  headervalues = ls->GetOneHeader(1); headervalues->AddOneADCValue(datatostorefrommor->fItemNb);
  headervalues = ls->GetOneHeader(2); headervalues->AddOneADCValue(datatostorefrommor->fSummary);
  headervalues = ls->GetOneHeader(3); headervalues->AddOneADCValue(datatostorefrommor->fRun);
  headervalues = ls->GetOneHeader(4); headervalues->AddOneADCValue(datatostorefrommor->fInputRegister);
  headervalues = ls->GetOneHeader(5); headervalues->AddOneADCValue(datatostorefrommor->fOutputRegister);

  
  for (int i = 0; i < 3; i++) {
    oneadc = ls->GetOneADC(ADCindex); 
    oneadc->AddOneADCValue(datatostorefrommor->fPMHV[i]); 
    ADCindex++;
  }

  for (int i = 0; i < 3; i++) {
    oneadc = ls->GetOneADC(ADCindex); 
    oneadc->AddOneADCValue(datatostorefrommor->fPMCur[i]); 
    ADCindex++;
  }

  for (int i = 0; i < 3; i++) {
    oneadc = ls->GetOneADC(ADCindex); 
    oneadc->AddOneADCValue(datatostorefrommor->fPMT[i]); 
    ADCindex++;
  }

  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fTElect); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fTWater); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fTfree); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fTBat[0]); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fTBat[1]); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fTairIn); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fTairOut); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fVBat[0]); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fVBat[1]); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fIChBat); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fILdBat); ADCindex++;

  for (int i = 0; i < 4; i++) {
    oneadc = ls->GetOneADC(ADCindex); 
    oneadc->AddOneADCValue(datatostorefrommor->fCloud[i]); 
    ADCindex++;
  }

  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fPWater); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fPAtm); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fGround); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fBoard3V); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fBoard5V); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fBoard12V); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fVSolPanel[0]); ADCindex++;
  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fVSolPanel[1]); ADCindex++;

  for (int i = 0; i < 3; i++) {
    oneadc = ls->GetOneADC(ADCindex); 
    oneadc->AddOneADCValue(datatostorefrommor->fDACPMHV[i]); 
    ADCindex++;
  }

  oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValue(datatostorefrommor->fDACLedPulse); ADCindex++;

  for (int i = 0; i < 4; i++) {
    oneadc = ls->GetOneADC(ADCindex); 
    oneadc->AddOneADCValue(datatostorefrommor->fDACThreshold[i]); 
    ADCindex++;
  }

  // Now for Raw data...
  ADCindex = 0;


  for (int i = 0; i < 3; i++) {
    oneadcraw = ls->GetOneRawADC(ADCindex); 
    oneadcraw->AddOneADCValue(rawdatatostorefrommor->fPMHV[i]);
    ADCindex++;
  }

  for (int i = 0; i < 3; i++) {
    oneadcraw = ls->GetOneRawADC(ADCindex); 
    oneadcraw->AddOneADCValue(rawdatatostorefrommor->fPMCur[i]);
    ADCindex++;
  }

  for (int i = 0; i < 3; i++) {
    oneadcraw = ls->GetOneRawADC(ADCindex); 
    oneadcraw->AddOneADCValue(rawdatatostorefrommor->fPMT[i]);
    ADCindex++;
  }

  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fTElect); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fTWater); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fTfree); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fTBat[0]); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fTBat[1]); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fTairIn); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fTairOut); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fVBat[0]); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fVBat[1]); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fIChBat); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fILdBat); ADCindex++;

  for (int i = 0; i < 4; i++) {
    oneadcraw = ls->GetOneRawADC(ADCindex);
    oneadcraw->AddOneADCValue(rawdatatostorefrommor->fCloud[i]);
    ADCindex++;
  }

  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fPWater); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fPAtm); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fGround); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fBoard3V); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fBoard5V); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fBoard12V); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fVSolPanel[0]); ADCindex++;
  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fVSolPanel[1]); ADCindex++;

  for (int i = 0; i < 3; i++) {
    oneadcraw = ls->GetOneRawADC(ADCindex);
    oneadcraw->AddOneADCValue(rawdatatostorefrommor->fDACPMHV[i]);
    ADCindex++;
  }

  oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValue(rawdatatostorefrommor->fDACLedPulse); ADCindex++;

  for (int i = 0; i < 4; i++) {
    oneadcraw = ls->GetOneRawADC(ADCindex);
    oneadcraw->AddOneADCValue(rawdatatostorefrommor->fDACThreshold[i]);
    ADCindex++;
  }
}

void TWholeArray::FillArrayWithExistingValues(UInt_t lowertime, UInt_t uppertime)
{
  Int_t nevent = (Int_t)gSDSummaryMonTreeSpecial->GetEntries();

  TADCArrayForOneStation *ls;
  TFloatValueToStore *headervalues;
  TFloatValueToStore *oneadc;
  TFloatValueToStore *oneadcraw;

    for (Int_t i=0;i<nevent;i++) {
      gSDSummaryMonTreeSpecial->GetEvent(i);
      gSDSummaryRawMonTreeSpecial->GetEvent(i);
      if (gSDSummaryMonData->fTime > lowertime && gSDSummaryMonData->fTime < uppertime) {
	// this is a good event need to store it again with the others in the file to be summarized
	if (gSDSummaryMonData->fSd_Id < 0 || gSDSummaryMonData->fSd_Id > gMaxStationNumber) {return;}
	//cout << "Station : " << sdsummarymondata->fSd_Id << endl;
	ls = GetOneLS(gSDSummaryMonData->fSd_Id);

	ls->SetEntry(gSDSummaryMonData->fEntries);
	ls->SetLSId(gSDSummaryMonData->fSd_Id);
	ls->SetTime(gSDSummaryMonData->fTime);

	headervalues = ls->GetOneHeader(0); headervalues->AddOneADCValueSpecial(gSDSummaryMonData->fSecond,gSDSummaryMonData->fSigmaSecond);
	headervalues = ls->GetOneHeader(1); headervalues->AddOneADCValueSpecial(gSDSummaryMonData->fItemNb,gSDSummaryMonData->fSigmaItemNb);
	headervalues = ls->GetOneHeader(2); headervalues->AddOneADCValueSpecial(gSDSummaryMonData->fSummary,gSDSummaryMonData->fSigmaSummary);
	headervalues = ls->GetOneHeader(3); headervalues->AddOneADCValueSpecial(gSDSummaryMonData->fRun,gSDSummaryMonData->fSigmaRun);
	headervalues = ls->GetOneHeader(4); headervalues->AddOneADCValueSpecial(gSDSummaryMonData->fInputRegister,gSDSummaryMonData->fSigmaInputRegister);
	headervalues = ls->GetOneHeader(5); headervalues->AddOneADCValueSpecial(gSDSummaryMonData->fOutputRegister,gSDSummaryMonData->fSigmaOutputRegister);

	UInt_t ADCindex = 0;
	for (int i = 0; i < 3; i++) {
	  oneadc = ls->GetOneADC(ADCindex); 
	  oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fPMHV[i],gSDSummaryMonData->fSigmaPMHV[i]);
	  ADCindex++;
	}

	for (int i = 0; i < 3; i++) {
	  oneadc = ls->GetOneADC(ADCindex); 
	  oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fPMCur[i],gSDSummaryMonData->fSigmaPMCur[i]); 
	  ADCindex++;
	}

	for (int i = 0; i < 3; i++) {
	  oneadc = ls->GetOneADC(ADCindex);
	  oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fPMT[i],gSDSummaryMonData->fSigmaPMT[i]);
	  ADCindex++;
	}

	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fTElect,gSDSummaryMonData->fSigmaTElect); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fTWater,gSDSummaryMonData->fSigmaTWater); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fTfree,gSDSummaryMonData->fSigmaTfree); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fTBat[0],gSDSummaryMonData->fSigmaTBat[0]); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fTBat[1],gSDSummaryMonData->fSigmaTBat[1]); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fTairIn,gSDSummaryMonData->fSigmaTairIn); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fTairOut,gSDSummaryMonData->fSigmaTairOut); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fVBat[0],gSDSummaryMonData->fSigmaVBat[0]); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fVBat[1],gSDSummaryMonData->fSigmaVBat[1]); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fIChBat,gSDSummaryMonData->fSigmaIChBat); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fILdBat,gSDSummaryMonData->fSigmaILdBat); ADCindex++;

	for (int i = 0; i < 4; i++) {
	  oneadc = ls->GetOneADC(ADCindex);
	  oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fCloud[i],gSDSummaryMonData->fSigmaCloud[i]);
	  ADCindex++;
	}

	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fPWater,gSDSummaryMonData->fSigmaPWater); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fPAtm,gSDSummaryMonData->fSigmaPAtm); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fGround,gSDSummaryMonData->fSigmaGround); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fBoard3V,gSDSummaryMonData->fSigmaBoard3V); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fBoard5V,gSDSummaryMonData->fSigmaBoard5V); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fBoard12V,gSDSummaryMonData->fSigmaBoard12V); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fVSolPanel[0],gSDSummaryMonData->fSigmaVSolPanel[0]); ADCindex++;
	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fVSolPanel[1],gSDSummaryMonData->fSigmaVSolPanel[1]); ADCindex++;

	for (int i = 0; i < 3; i++) {
	  oneadc = ls->GetOneADC(ADCindex);
	  oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fDACPMHV[i],gSDSummaryMonData->fSigmaDACPMHV[i]);
	  ADCindex++;
	}

	oneadc = ls->GetOneADC(ADCindex); oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fDACLedPulse,gSDSummaryMonData->fSigmaDACLedPulse); ADCindex++;

	for (int i = 0; i < 4; i++) {
	  oneadc = ls->GetOneADC(ADCindex);
	  oneadc->AddOneADCValueSpecial(gSDSummaryMonData->fDACThreshold[i],gSDSummaryMonData->fSigmaDACThreshold[i]);
	  ADCindex++;
	}
	
	// Now for Raw data...
	ADCindex = 0;
	for (int i = 0; i < 3; i++) {
	  oneadcraw = ls->GetOneRawADC(ADCindex); 
	  oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fPMHV[i],gSDSummaryRawMonData->fSigmaPMHV[i]);
	  ADCindex++;
	}

	for (int i = 0; i < 3; i++) {
	  oneadcraw = ls->GetOneRawADC(ADCindex);
	  oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fPMCur[i],gSDSummaryRawMonData->fSigmaPMCur[i]);
	  ADCindex++;
	}

	for (int i = 0; i < 3; i++) {
	  oneadcraw = ls->GetOneRawADC(ADCindex);
	  oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fPMT[i],gSDSummaryRawMonData->fSigmaPMT[i]);
	  ADCindex++;
	}

	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fTElect,gSDSummaryRawMonData->fSigmaTElect); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fTWater,gSDSummaryRawMonData->fSigmaTWater); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fTfree,gSDSummaryRawMonData->fSigmaTfree); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fTBat[0],gSDSummaryRawMonData->fSigmaTBat[0]); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fTBat[1],gSDSummaryRawMonData->fSigmaTBat[1]); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fTairIn,gSDSummaryRawMonData->fSigmaTairIn); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fTairOut,gSDSummaryRawMonData->fSigmaTairOut); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fVBat[0],gSDSummaryRawMonData->fSigmaVBat[0]); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fVBat[1],gSDSummaryRawMonData->fSigmaVBat[1]); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fIChBat,gSDSummaryRawMonData->fSigmaIChBat); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fILdBat,gSDSummaryRawMonData->fSigmaILdBat); ADCindex++;

	for (int i = 0; i < 4; i++) {
	  oneadcraw = ls->GetOneRawADC(ADCindex);
	  oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fCloud[i],gSDSummaryRawMonData->fSigmaCloud[i]);
	  ADCindex++;
	}

	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fPWater,gSDSummaryRawMonData->fSigmaPWater); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fPAtm,gSDSummaryRawMonData->fSigmaPAtm); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fGround,gSDSummaryRawMonData->fSigmaGround); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fBoard3V,gSDSummaryRawMonData->fSigmaBoard3V); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fBoard5V,gSDSummaryRawMonData->fSigmaBoard5V); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fBoard12V,gSDSummaryRawMonData->fSigmaBoard12V); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fVSolPanel[0],gSDSummaryRawMonData->fSigmaVSolPanel[0]); ADCindex++;
	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fVSolPanel[1],gSDSummaryRawMonData->fSigmaVSolPanel[1]); ADCindex++;

	for (int i = 0; i < 3; i++) {
	  oneadcraw = ls->GetOneRawADC(ADCindex);
	  oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fDACPMHV[i],gSDSummaryRawMonData->fSigmaDACPMHV[i]);
	  ADCindex++;
	}


	oneadcraw = ls->GetOneRawADC(ADCindex); oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fDACLedPulse,gSDSummaryRawMonData->fSigmaDACLedPulse); ADCindex++;

	for (int i = 0; i < 4; i++) {
	  oneadcraw = ls->GetOneRawADC(ADCindex);
	  oneadcraw->AddOneADCValueSpecial(gSDSummaryRawMonData->fDACThreshold[i],gSDSummaryRawMonData->fSigmaDACThreshold[i]);
	  ADCindex++;
	}

	//if (ls->GetLSId() == 37) {cout << endl << "In Fill existing values"<< endl; ls->DumpOnScreen();cout << "!!!!!!!!!"<< endl;}
      }
      else {cout << "Be careful it's strange...." << endl;}
    }
}

void TWholeArray::ComputeAverage() {
  TSDSummaryMonCal *ls;
  
  for (Int_t i = 0; i < kMaxStationNumber; i++) {
    ls = GetOneLS(i);
    if (ls->fEntries() > 0) {
      //if (i==37) {cout << endl << "In ComputeAverage Before Average" << endl;ls->DumpOnScreen();cout << "-------"<< endl;}
      ls->ComputeAverageOneLS();
      //if (i==37) {cout << "In ComputeAverage After Average" << endl;ls->DumpOnScreen();cout << "-------"<< endl;}
    }
  }
}

void TSDArray::StoreData(Int_t flag) {
  TSDSummaryMonCal *ls;
  
  for (Int_t i= 0; i < kMaxStationNumber; i++) {
    ls = GetOneLS(i);
    if (ls->fEntries > 0) {
      //if (i==37) {cout << endl << "In store data"<< endl; ls->DumpOnScreen();cout << "~~~~~~~~"<< endl;}
      ls->WriteMonDataInSummaryFile(flag);
    }
  }
}
