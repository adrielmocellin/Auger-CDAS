#include "Eb.h"
#include "EbEvent.h"

#include <unistd.h>
#include <Pm.h>
#include <central_local.h>
#include <DbC.h>

extern int gDebug, gVerbose;
extern unsigned long gCurrentEventNumber;
extern string gT3Sender;
extern DbCStation *gDBStations;
extern int gDBNbStations;

/*-----------------------------------------------------------------------*/ 
/* It builds an EbEvent with an EbT3Trigger.                             */
EbEvent::EbEvent() : IoSdEvent() {
/*-----------------------------------------------------------------------*/ 
// default constructor
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* It builds an EbEvent with an EbT3Trigger.                             */
EbEvent::EbEvent(IkT3* ikm) : IoSdEvent() {
/*-----------------------------------------------------------------------*/ 
  if (ikm == NULL) return;
  unsigned int i; /* compteur */

  // fill Trigger
  Trigger.Id = ikm->id;
  Trigger.Sender = gT3Sender;
  Trigger.NumberOfStation = ikm->addresses.size();
  Trigger.Second = ikm->refSecond;
  Trigger.MicroSecond = ikm->refuSecond;
  Trigger.SDPAngle = ikm->SDPAngle;
  Trigger.Algo = ikm->algo;

  // allocate stations
  for (i=0; i<ikm->addresses.size(); i++) {
    IoSdStation stat;
    stat.Id = ikm->addresses[i];
    stat.Error = IoSdEvent::eDataLost;
    stat.Trigger.Offset = ikm->offsets[i];
    stat.Trigger.Window = ikm->window[i];
    /* Coordinates */
    int j;
    for (j=0; j<gDBNbStations; j++) if (stat.Id == (unsigned int)gDBStations[j].Id) {
      stat.Easting = gDBStations[j].Easting;
      stat.Northing = gDBStations[j].Northing;
      stat.Altitude = gDBStations[j].Altitude;
      stat.Name = gDBStations[j].Name;
      break;
    }
    if (j==gDBNbStations) { // new station in "run" ?
      gDBStations = DbCArrayInit();
      gDBNbStations = DbCArrayGetNbStat(gDBStations);
      int j2;
      for (j2=0; j2<gDBNbStations; j2++) if (stat.Id == (unsigned int)gDBStations[j2].Id) {
	stat.Easting = gDBStations[j2].Easting;
	stat.Northing = gDBStations[j2].Northing;
	stat.Altitude = gDBStations[j2].Altitude;
	stat.Name = gDBStations[j2].Name;
	break;
      }
      if (j2==gDBNbStations) { // no
	IkWarningSend ("T3 with ls %d which is not registered in Db", stat.Id);
	if (gDebug || gVerbose) {
	  stringstream s; s<< "T3 with ls " << stat.Id << " which is not registered in Db" << endl;
	  InternalLog(s.str().c_str(),IKDEBUG);
	}
      }
    }
    Stations.push_back(stat);
  }  

  // local informations
  Id = gCurrentEventNumber;
  Request = 0;
  time(&ConstructionTime);
}
/*-----------------------------------------------------------------------*/ 


/*-----------------------------------------------------------------------*/ 
EbEvent::EbEvent(IoSdStation* stat, int t3id) : IoSdEvent() {
/*-----------------------------------------------------------------------*/ 
  Id = 0; // pour un temps court normalement...
  Request = 0;
  time(&ConstructionTime);
  Trigger.Id = t3id;
  IoSdStation s=*stat;
  Stations.push_back(s);
}
/*-----------------------------------------------------------------------*/ 

