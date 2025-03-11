#include "IkC.h"
#include "IkSuPing.hxx"
#include "IkSuPong.hxx"
#include "IkT3.hxx"
#include "DbCArray.h"
#include "UiMessage.h"
#include "Fd.h"
#include <netinet/in.h>
#include <fstream>

#include <math.h>
#include <time.h>

#include "LatLong-UTMConversion.h"
#include "IoSdData.h"
#define cmicro 299.792458

#define MAXTANKID 2048
int nstat=0;
struct DbCSimpleStation* array1=NULL;
double X[MAXTANKID]; // X of tanks
double Y[MAXTANKID]; // Y of tanks
unsigned int eye[MAXTANKID]; // closest eye of tanks

int FdIkPingValue;

int FdVeto=0;
int lastFdsecond=0;

//  New Veto stuff
vector <unsigned int> t3secondsveto[6];

// For Laser study
#define CLF_ID 203 //Celeste
#define CLF_LASER_TIME 500000.0
#define BAD_CLF_LASER_TIME 250000.0
#define XLF_ID 805 //Ramiro
#define XLF_LASER_TIME 700000.0
#define BAD_XLF_LASER_TIME 350000.0

static int distmicro(int i, int eye) {
  int eye_aux;
  eye_aux=eye;
  if(eye_aux==5){
     //rsato - 2013/May/30 - It looks to have some problems related with
     //   HEAT. As HEAT is not official, in IoSD there are not definition
     //   of eye=5;
     eye_aux=4;
  }
  return (int)(sqrt(pow(X[i]-kIoSd::FdX(eye_aux),2)+pow(Y[i]-kIoSd::FdY(eye_aux),2))/cmicro);
}

void FdTriggerArrayInit() {
  for (int i=1;i<6;i++)
    while (t3secondsveto[i].size()<3) t3secondsveto[i].push_back(0);
  if (array1) delete array1;
  array1=DbCSimpleArrayInit();
  nstat=DbCSimpleArrayGetNbStat(array1);
  for(int i=0; i<nstat; i++)
    if (array1[i].Id<MAXTANKID) {
      ne2xy(array1[i].Northing,array1[i].Easting,&(X[array1[i].Id]),&(Y[array1[i].Id]));
      eye[array1[i].Id]=1;
      double dmin=999999;
      for (int j=1;j<5;j++) { // NDX: Use only 4 official eyes
        double d=distmicro(array1[i].Id,j)*cmicro;
        if (d<dmin) {
          dmin=d;
          eye[array1[i].Id]=j;
        }
      }
    }
  stringstream s;
  s << nstat << " stations in array";
  InternalLog(s.str().c_str(),IKINFO);
}

int FdIk(IkMessage *ikm) {
  switch (ikm->get_type()) {
  case IKSUPING:
    FdIkPingValue=((IkSuPing*)ikm)->PingValue;
    break;
  default:
    IkWarningSend("Received Unknown IkMessage: %s",ikm->to_text().c_str());
    break;
  }
  return 0;
}

void FdIkPong() {
  if (FdIkPingValue) {
    IkSuPong pong;
    pong.PongValue=FdIkPingValue;
    pong.send(SU);
    FdIkPingValue=0;
  }
}

int IsValidT3(T3Notify *t3) {
  unsigned int tm=(unsigned int)time(NULL);
  tm-=315964800;
  tm+=15;
  // Laser
  stringstream s;
  if (fabs(t3->GetT3GPSNanoTime()/1000. - distmicro(XLF_ID,t3->GetEyeId()) - BAD_XLF_LASER_TIME) < 30) {
    s << "Event time-vetoed: candidate laser at 1/4 second: " << t3->GetT3GPSTime() << " " << t3->GetT3GPSNanoTime();
    InternalLog(s.str().c_str(),IKINFO);
    return 0;
  }
  if (fabs(t3->GetT3GPSNanoTime()/1000. - distmicro(CLF_ID,t3->GetEyeId()) - BAD_CLF_LASER_TIME) < 30) {
    s << "Event time-vetoed: candidate laser at 1/4 second: " << t3->GetT3GPSTime() << " " << t3->GetT3GPSNanoTime();
    InternalLog(s.str().c_str(),IKINFO);
    return 0;
  }
  // Lidar
  if ((t3->GetT3GPSNanoTime()/1000+10000-25)%10000 < 50) {
    s << "Event time-vetoed: candidate lidar at periodic moment " << t3->GetT3GPSTime() << " " << t3->GetT3GPSNanoTime();
    InternalLog(s.str().c_str(),IKINFO);
    return 0;
  }
  // T3 from future
  if (t3->GetT3GPSTime()>tm+5) {
    s << "Event time-vetoed: received at second " << tm << " has time " << t3->GetT3GPSTime();
    s << ": coming from future, rejected";
    InternalLog(s.str().c_str(),IKWARNING);
    return 0;
  }
  // T3 from past
  if (t3->GetT3GPSTime()+10<tm) {
    s << "Event time-vetoed: received at second " << tm << " has time " << t3->GetT3GPSTime();
    s << ": coming too late, rejected";
    InternalLog(s.str().c_str(),IKWARNING);
    return 0;
  }
  // New Veto
  double dt=t3->GetT3GPSTime()-t3secondsveto[t3->GetEyeId()][0];
  double nu=dt*PHYSRATE;
  double proba=1-exp(-nu)*(1+nu+nu*nu/2);
  t3secondsveto[t3->GetEyeId()].erase(t3secondsveto[t3->GetEyeId()].begin());
  t3secondsveto[t3->GetEyeId()].push_back(t3->GetT3GPSTime());
  if (proba<DEADTIME) {
    s << "Event received at second " << tm;
    s << " statistic-vetoed for eye " << t3->GetEyeId() << " (proba= " << proba << " < " << DEADTIME << ")";
    InternalLog(s.str().c_str(),IKWARNING);
    return 0;
  }
  return 1;
}

int ReadLastT3Id() {
  int id = 6000;
  ifstream rdr("/Raid/var/Fd/t3id");
  rdr >> id;
  rdr.close();
  if (id<4096 || id>=8192) id=6000;
  return id;
}

void SaveLastT3Id (int id) {
  ofstream wtr("/Raid/var/Fd/t3id");
  wtr << id;
  wtr.close();
}

int FdTriggerT3Emit(unsigned int site,unsigned short t3id,unsigned int second, unsigned int tzusec, double phi) {
  static int myT3Id = ReadLastT3Id() + 10 ;
  IkT3 ikm;
  int uSecond;
  char triggeralgo[256];
  if (t3id<4096 || t3id>=8192) {
    IkWarningSend("Bad t3id %d from fluorescence site %d",t3id,site);
    return 0;
  }
  switch (site) {
  case 1:
    ikm.algo="Los Leones";
    break;
  case 2:
    ikm.algo="Los Morados";
    break;
  case 3:
    ikm.algo="Loma Amarilla";
    break;
  case 4:
    ikm.algo="Coihueco";
    break;
  case 5:
    ikm.algo="HEAT";
    break;
  default:
    IkWarningSend("unknown site %d",site);
    return 0;
  }
  if (myT3Id > 8191) myT3Id = 4096;
  ikm.id=myT3Id;
  SaveLastT3Id(myT3Id);
  myT3Id++;
  ikm.refSecond=second;
  ikm.refuSecond=tzusec;
  ikm.SDPAngle=phi;
  ikm.mode=LIST;
  // check for Laser
  if (fabs(tzusec - distmicro(XLF_ID,site) - XLF_LASER_TIME) < 30) {
    snprintf(triggeralgo,256,"Laser %s (%d)",ikm.algo.c_str(),t3id);
    uSecond = (int) (tzusec - distmicro(XLF_ID,site) + TRACELENGTH);
    ikm.addresses.push_back(XLF_ID);
    ikm.offsets.push_back(0);
    ikm.window.push_back(150);
  } else if (fabs(tzusec - distmicro(CLF_ID,site) - CLF_LASER_TIME) < 30) {
    snprintf(triggeralgo,256,"Laser %s (%d)",ikm.algo.c_str(),t3id);
    uSecond = (int) (tzusec - distmicro(CLF_ID,site) + TRACELENGTH);
    ikm.addresses.push_back(CLF_ID);
    ikm.offsets.push_back(0);
    ikm.window.push_back(150);
  } else {
    snprintf(triggeralgo,256,"FD %s (%d)",ikm.algo.c_str(),t3id);
    int d=0;
    int psite=site;
    if (site==5) psite=4; // principal site of HEAT is Coihueco
    // looking for tank further away from eye
    for(int i=0; i<nstat; i++) {
      if (array1[i].Id>=MAXTANKID || eye[array1[i].Id]!=psite) continue;
      if (distmicro(array1[i].Id,site)>d) d=distmicro(array1[i].Id,site);
    }
    // magic formula
    uSecond=tzusec+TRACELENGTH-d;
    // Moving up in time this minimum uSecond by 120 usec
    // to optimize the "char" range
    uSecond+=CENTRALOFFSET;
    for(int i=0; i<nstat; i++) {
      if (array1[i].Id>=MAXTANKID || eye[array1[i].Id]!=psite) continue;
      ikm.addresses.push_back(array1[i].Id);
      ikm.offsets.push_back(d-distmicro(array1[i].Id,site)-CENTRALOFFSET);
      ikm.window.push_back(WINDOW);
    }
  }
  if (uSecond<0) {
    ikm.refuSecond=uSecond+1000000;
    ikm.refSecond--;
  } else ikm.refuSecond=uSecond;
  ikm.algo=triggeralgo;
  if (ikm.addresses.size()>0) ikm.send(PmName);
  else if (verbose) IkInfoSend("T3 from fluorescence without GA tank in range");
  if (debug) {
    stringstream s;
    s << "** T3 " << ikm.algo << " #"<< myT3Id << " sent to Sd array **";
    InternalLog(s.str().c_str(),IKDEBUG);
  }
  return 1;
}

