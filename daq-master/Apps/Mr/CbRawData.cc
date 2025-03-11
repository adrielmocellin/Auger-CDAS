#include "CbRawData.h"
#include <Pm.h>
#include <math.h>
#include <MrRoot.h>
#include <iostream>

using namespace std;

extern string gRootFileId;

void TCbRawData::Reset(){
  //
  // Sets all the members to 0
  //

  fIsFilled = 0;
  fLsId = 0;
  fType = 0;
  fVersion = 0;
  ResetCalibration();
}

float TCbRawData::ComputeSigma(double var, double sqrvar, short entries) {
  //
  // Be careful : var must have been divided previously by entries !!!!
  // var is the sum of the variable divided by entries
  // sqrvar is the running sum of the square of the variable
  // It returns the value of sigma
  //
  
  float sigma = 0;
  if (entries > 0) sigma = sqrvar-var*var;
  if (sigma > 0) sigma = sqrt(sigma);
  else sigma=0;
  return sigma;
}

int TCbRawData::SetData_UUB_v262(char *dat,int size)
{
  //return 0 on success
  //      -1 in case of error
  uint32_t *ptr;
  uint16_t *spt;
  uint8_t *pt;
  int i;
  ptr=(uint32_t *)dat;

  uint32_t requestId = ntohl(*ptr++);
  uint32_t bufferOK  = ntohl(*ptr++);
  fIsFilled=true;
  if(size < 104 + 2*sizeof(uint32_t)){
    fIsCalibration=false;

    return(-1);
  }
  fIsCalibration=true;
  spt=(uint16_t *)ptr;
  fVersion2    = ntohs(*spt++);
  fTubeMask    = ntohs(*spt++);
  ptr=(uint32_t *)spt;
  fStartSecond = ntohl(*ptr++);
  spt=(uint16_t *)ptr;
  fEndSecond   = ntohs(*spt++);
  fTotD = ntohs(*spt++);
  fMops = ntohs(*spt++);
  spt++;
  fT1          = ntohs(*spt++);
  fT2          = ntohs(*spt++);
  fTotRate     = ntohs(*spt++)*1./fEndSecond;
  uint16_t nb_full_buffer = ntohs(*spt++);
  for(i=0;i<3;i++){
    fAnode[i]    = ntohs(*spt++)*.01;
    fDynode[i]   = ntohs(*spt++)*.01;
  }
  for(i=0;i<4;i++){
    fUBase[i] = ntohs(*spt++)*.01;
  }

  for(i=0;i<3;i++){
    fVarianceAnode[i]    = ntohs(*spt++)*.01;
    fVarianceDynode[i]   = ntohs(*spt++)*.01;
  }
  for(i=0;i<4;i++){
    fVarianceUBase[i] = ntohs(*spt++)*.01;
  }
  for(i=0;i<3;i++){
    fPast[i]   = ntohs(*spt++);
  }

  uint16_t fPast_ssd   = ntohs(*spt++);  //it is still not implemented
  for(i=0;i<3;i++){
    fPeak[i]   = ntohs(*spt++)*.1;
  }
  Float_t fPeak_ssd   = ntohs(*spt++)*.1;  //it is still not implemented

  for(i=0;i<3;i++){
    f70HzRate[i] = ntohs(*spt++)*.01;
  }
  Float_t f70HzRate_ssd   = ntohs(*spt++);  //it is still not implemented
  for(i=0;i<3;i++){
    fDynodeAnode[i]=ntohs(*spt++)*.01;       // HG/LG Ratio
  }
  Float_t fHG_LG_ssd   = ntohs(*spt++)*.01;  //it is still not implemented
  for(i=0;i<3;i++){
    fArea[i] = ntohs(*spt++)*.1;
  }
  Float_t fArea_ssd = ntohs(*spt++)*.1;
  return(0);
}

int TCbRawData::SetData(char* data) {
  //
  // Decode the calibration message from Pm
  //

  // Pointers to be used

  longWord *ptr;
  short *sP;
  int version;
  //short base[6];
  //short sigmabase[6];

  PmLsRawMessage *rawP = (PmLsRawMessage *)(data+sizeof(longWord)); 
  PmLsMessage *messP = &(rawP->mess);

  fLsId = ntohl(rawP->lsId);

  fType = ntohl(messP->type);
  fVersion = ntohl(messP->version);
  longWord messize = ntohl(messP->length);

  ptr = (longWord*)&(messP->data);
  longWord requestId = ntohl(*ptr++);
  longWord bufferOK = ntohl(*ptr++);

  fStartSecond = 0;
  fEndSecond = 0;
  if (bufferOK != 0) {
    cout << " Corrupted calibration block from LS " << fLsId << endl;
    fIsCalibration=false;
    return -1;
  }
  if(8<messize){
    version=ntohs(*(short *)ptr);
    if(256 < version){ //It is UUB.
      if(262<=version && version<300){
        return(SetData_UUB_v262(messP->data,messize));
      }
      fIsFilled=false;
      fIsCalibration=false;
      return(-1);
    } 
  }
  if (messize != 104 && messize!=124) {
    cout << " Corrupted calibration block from LS " << fLsId << " invalid size " << messize << endl;
    FILE *file2 = fopen("/Raid/var/Mr_log/DebugXBRR.txt","a");
    fprintf(file2,"#BAD-size %d %d\n",fLsId,messize);
    fclose(file2);
    fIsFilled=false;
    fIsCalibration=false;
    return -1;
  }

  // Now we start decoding the calib message

#define NEW_CALIB_VERSION

#ifdef NEW_CALIB_VERSION
  sP = (short*)ptr;
  fVersion2 = ntohs(*sP++);

  /*if (fVersion2 != 4) {
    cout << " WRONG CALIBRATION BLOCK. MR CANNOT STORE IT !!!!!!!!!!" << endl;
    return -1;
  }*/
  fTubeMask = ntohs(*sP++);

  ptr = (longWord*)sP;
  fStartSecond = ntohl(*ptr++);
  fEndSecond = ntohl(*ptr++);

  sP=(short*)ptr;
  fT1 = ntohs(*sP++);
  fT2 = ntohs(*sP++);

  for (int i=0;i<3;i++)
    fPast[i] = ntohs(*sP++);

  for (int i=0;i<3;i++)
    fDynode[i] = ntohs(*sP++)*0.01;
  for (int i=0;i<3;i++)
    fAnode[i] = ntohs(*sP++)*0.01;
  for (int i=0;i<3;i++)
    fVarianceDynode[i] = ntohs(*sP++)*0.01;
  for (int i=0;i<3;i++)
    fVarianceAnode[i] = ntohs(*sP++)*0.01;

  for (int i=0;i<3;i++)
    fPeak[i] = ntohs(*sP++)*0.1;

  for (int i=0;i<3;i++)
    f70HzRate[i]=ntohs(*sP++)*0.01;

  for (int i=0;i<3;i++)
    fTriggerDA[i]=ntohs(*sP++);

  for (int i=0;i<3;i++)
    fDynodeAnode[i] = ntohs(*sP++)*0.01;
  for (int i=0;i<3;i++)
    fVarianceDynodeAnode[i] = ntohs(*sP++)*0.01;

  for (int i=0;i<3;i++)
    fArea[i] = ntohs(*sP++)*0.1;

  fTotRate = ntohs(*sP++)*0.01;

  if (messize==124) {
    for (int j=0; j<3; j++) fDynodeAnodeDt[j] = ntohs(*sP++)/100.;
    for (int j=0; j<3; j++) fVarianceDynodeAnodeDt[j] = ntohs(*sP++)/100.;
    for (int j=0; j<3; j++) fDynodeAnodeChi2[j] = ntohs(*sP++)/100.;
    sP++; // (we have 9 shorts. To int-pad it, we need a 10th short.
  }

#endif

#ifdef OLD_CALIB_VERSION
  fStartSecond = ntohl(*ptr++);
  fEndSecond = ntohl(*ptr++);

  sP=(short*)ptr;
  fNumberT1 = ntohs(*sP++);
  for (int i=0;i<6;i++)
    Base[i] = ntohs(*sP++);
  for (int i=0;i<6;i++)
    BaseD[i] = ntohs(*sP++);
  for (int i=0;i<6;i++)
    Base2[i] = ntohs(*sP++);
  for (int i=0;i<6;i++)
    Base2D[i] = ntohs(*sP++);
  for (int i=0;i<3;i++)
    fTubeOk[i] = ntohs(*sP++);
  for (int i=0;i<3;i++)
    fVEM[i] = ntohs(*sP++);
  short short_temp;
  for (int i=0;i<3;i++) {
    short_temp=ntohs(*sP++);
    f70HzRate[i]=short_temp*0.01;
  }
  for (int i=0;i<3;i++)
    fNumberTriggerDA[i]=ntohs(*sP++);
  sP++;
  longWord* lP=(longWord*)sP;
  longWord temp;
  for (int i=0;i<3;i++) {
    temp=ntohl(*lP++);
    if (fNumberTriggerDA[i] > 1) fDynodeAnode[i]=.01*temp/fNumberTriggerDA[i];
    else fDynodeAnode[i] = 0;
  }
  for (int i=0;i<3;i++) {
    temp = ntohl(*lP++);
    if (fNumberTriggerDA[i] > 1) fSquareDynodeAnode[i]=.01*temp/fNumberTriggerDA[i];
    else fSquareDynodeAnode[i] = 0;
  }

  for (int i=0;i<3;i++)
    fVarianceDynodeAnode[i]=ComputeSigma((double)fDynodeAnode[i],(double)fSquareDynodeAnode[i],fNumberTriggerDA[i]);

  for (int i=0;i<6;i++) { 
    fBase[i]=(Base[i]+BaseD[i]*0.0001);        // Fill the value of the bases
    fSquareBase[i]=Base2[i]+Base2D[i]*0.0001;
    fVarianceBase[i] = ComputeSigma((double)fBase[i],(double)fSquareBase[i],fNumberTrigger);
  }
#endif

#ifdef VERBOSE
  //Dump();
#endif

  // What to do for corrupted blocks ???

  //if (fVersion2 > 300 || fEndSecond >= (fStartSecond + 10000) || fEndSecond <= fStartSecond || fTubeMask > 20) {  // was like that, simplified
  //if (fVersion2 > 300 || fTubeMask > 40) { // changed again, ASCII has TubeMask==22
  if (fVersion2 > 300 || fTubeMask > 64) { // changed again, SSD has 3 more bits used
    //if (fVersion2 == 12 ) {
    cout << "JUNK CALIB BLOCK" << endl;
    string filename = "/Raid/var/Mr_log/BadCalibFiles_" + gRootFileId + ".txt";
    FILE *file = fopen(filename.c_str(),"a");

    int toto = (int)7*sizeof(ptr) + (int)38*sizeof(sP);

    for (int i=0; i<toto; i++) fprintf(file,"%2.2X",((unsigned int)data[i]) & 0xFF);
    fprintf(file," \t ");

    fprintf(file,"\t %i \t %i \t %i \t %i \t %i \t %hu \t %hu \t %u \t %u \t %hi \t %hi \n", fLsId, fType, fVersion, requestId, bufferOK, fVersion2, fTubeMask, fStartSecond, fEndSecond, fT1, fT2);
    fclose(file);
    FILE *file2 = fopen("/Raid/var/Mr_log/DebugXBRR.txt","a");
    fprintf(file2,"#BAD-Cyril %d %d\n",fLsId,messize);
    fclose(file2);
    return -1;
  }

  fIsFilled = 1;
  fIsCalibration = 1;
  return 0;
}

void TCbRawData::Dump(){
  //
  // Dump on screen all the members
  //
  
  DumpHeader();

  cout << endl << "     fLsId = " << fLsId;
  cout << "     fType = " << fType;
  cout << "     fVersion = " << fVersion << endl << endl;
  
  DumpMembers();
}
