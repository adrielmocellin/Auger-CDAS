#include "Mr_ClassDef.h"
#include <iostream>

TSDMrMonCal::TSDMrMonCal(int lsid, TMoRawData* rawmonitdata, TCbRawData* rawcalibdata) {
  //
  // This constructor copy the content of TMoRawData and TCbRawData
  // After this operation, data are ready to be stored in the root file

  unsigned int gps_offset = 315964800;
  unsigned int gps_leap = 13;

  fLsId = lsid;

  fCDASTime = (UInt_t)time((time_t *)NULL);

  if (rawmonitdata->fSecond > 750000000 && rawmonitdata->fSecond < 2000000000) 
    fTime = (UInt_t)rawmonitdata->fSecond + gps_offset - gps_leap;
  //else if (rawcalibdata->fEndSecond > 750000000 && rawcalibdata->fEndSecond < 2000000000)
    //fTime = (UInt_t)rawcalibdata->fEndSecond + gps_offset - gps_leap;
  else fTime = fCDASTime; 

  std::cout << "LS : " << fLsId  << "   Time : " << fTime << "   MONIT SECOND : " << rawmonitdata->fSecond << "   CALIB SECOND : " << rawcalibdata->fEndSecond << std::endl;


  fCalibration = (TSDCalibration)*rawcalibdata;
  fRawMonitoring = (TSDRawMonitoring)*rawmonitdata;
  fMonitoring = (TSDMonitoring)rawmonitdata->fMonitoring;
  fSystem = (TSDSystem)*rawmonitdata;
}
