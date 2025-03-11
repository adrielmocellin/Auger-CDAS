#include "Rc.h"
#include <IkLsReady.hxx>
#include <IkLsGenMess.hxx>
#include <IkLsStartAck.hxx>
#include <IkLsOS9.hxx>
#include <IkLsSaveConf.hxx>
#include <IkLsReboot.hxx>
#include <IkLsWakeUp.hxx>
#include <stdlib.h>
#include <stdio.h>

int m_debug = 0;

void RunControl::RcIkHandler (IkMessage* ikm)
{
  switch (ikm->get_type()) 
    {
      
    case IKLSREADY:
      if(m_debug) {
	stringstream s; 
	s <<"\t\t------> Received IkLsReady from station "<<((IkLsReady*)ikm)->StationId; 
	InternalLog(s.str().c_str(), IKDEBUG);
	}
      theRunControl()->LsReadyHandler((IkLsReady*)ikm);
      break;
    case IKLSGENMESS:
      if (m_debug) {
	stringstream s; 
	s <<"\t\t-----> Received IkLsGenMess from station "<<((IkLsGenMess*)ikm)->StationId;
	InternalLog(s.str().c_str(), IKDEBUG);
	}
      theRunControl()->LsGenMessHandler((IkLsGenMess*)ikm);
      break;
    case IKLSOS9ACK :
      if (m_debug) {
	stringstream s; 
	s <<"\t\t-----> Received IkLsOS9Ack from station "<<((IkLsGenMess*)ikm)->StationId; 
	InternalLog(s.str().c_str(), IKDEBUG);
	}
      theRunControl()->LsOS9AckHandler((IkLsOS9Ack*)ikm);
      break;
    case IKLSSTARTACK:
      if(m_debug) {
	stringstream s; s <<"station "<<((IkLsStartAck*)ikm)->StationId<<" in acquisition."; 
	InternalLog(s.str().c_str(), IKDEBUG);
	}
      break;
      
    default:
      stringstream s; s << "Unknown message received by Ik. ";
      InternalLog(s.str().c_str(), IKWARNING);
      break;
    }
  
}

void RunControl::LsGenMessHandler(IkLsGenMess* ikm)
	// making it slightly more usefull...
{
	if (ikm->StationId>2048) {
		IkLsWakeUp wakeup;
		wakeup.addresses.push_back(ikm->StationId);
		wakeup.mode="LIST";
		wakeup.send("Pm");
	} else if (ikm->msg == "GPS: LS in past" ||
		(strncmp(ikm->msg.c_str(),"GPS: LS in future",15)==0)) {
		IkLsReboot Reboot;
		Reboot.addresses.push_back(ikm->StationId);
		Reboot.mode="LIST";
		Reboot.send("Pm");
	}
}

void RunControl::LsOS9AckHandler(IkLsOS9Ack* ikm)
{ 
  IkLsReboot reboot; reboot.mode = "LIST";
  if (ikm->StationId<2048) 
    return;
  reboot.addresses.push_back(ikm->StationId);
  reboot.send("Pm");
  stringstream s; s<< "Reboot sent to "<< 
		    ikm->StationId; 
  InternalLog(s.str().c_str(), IKINFO); 
}

void RunControl::LsReadyHandler(IkLsReady *ikm)
{ 
  SdTank ready;
  // In an LsReadyMessage Pm assign the first received value (north) to X
  // the second (east) to Y and the last one (height) to Z. Of course
  // in reality Northing corresponds to Y and Easting to X but 
  // me must follow Pm conventions.
  ready.Id       = ikm->StationId;
  ready.Northing = ikm->X/100.0;
  ready.Easting  = ikm->Y/100.0;
  ready.Altitude = ikm->Z/100.0;
  ready.Name = ""; ready.Domain = "";
  ready.Valid = 0;

  int surveyIndex = SurveyIndex(ready.Id); 
  
  if (!surveyIndex ) {
    DbLoad();   // Will reload Dbs if it was'nt done recently 
    surveyIndex = SurveyIndex(ready.Id);
  }
  
  // Set up message that will eventually send.
  // IkLsOS9 bldConf;IkLsSaveConf flash; IkLsReboot reboot;
  // bldConf.mode = flash.mode = reboot.mode = "LIST";
  IkLsOS9 bldConf;
  bldConf.mode = "LIST";
  
  if (ready.Id < 2048 ) {
    // the station has an assigned logical Id,
    // We should find it both in the CDAS and Survey Db
    // However we may have to update the coordinates.
    if (!surveyIndex) {
      //This should not happen but some EA station may not yet be in the traveler
      // Or may be in with a different ID 
      IkWarningSend("You should create Survey data base entry for station Id %d",ready.Id);
      stringstream s;
      s << "Station " << ready.Id << " : Please create Survey data base entry";
      InternalLog(s.str().c_str(), IKERROR);
    } 
    else { // here we should have a survey entry make sure they match
      if (!XyzMatch(&(SurveyDb()[surveyIndex]), &ready)) {
	IkWarningSend("Station %d coordinates from LsReady do not match Survey Db. DO SOMETHING!!!",ready.Id);
	stringstream s;
	s << "Station " << ready.Id;
	s << " coordinates from LsReady do not match Survey Db. DO SOMETHING!!!"; 
	InternalLog(s.str().c_str(), IKERROR);
      }
    }
    return;
  }
  // We do not have a logical Id
  // We look for the closest coordinate match in the
  // survey Db. If a unique match is found we build the 
  // corresponding Cdas entry and update the station config passing the 
  // logical Id taken as the survey Id.
  SdTank *surveyTank = TankBestMatch(&ready, SurveyDb());
  SdTank *surveyTank2 = TankNextBestMatch(&ready, surveyTank, SurveyDb());
  if (surveyTank && surveyTank2 && TankDistance(surveyTank, surveyTank2) < 100 /* meters */) {
    // we have two tanks two close to a match
    // check the electronics
    if (surveyTank->Valid+2048==ready.Id) {
      IkWarningSend("Two tanks in Survey DB match tank Id %d. Using Electronics Id to desintangle...",ready.Id);
    } else if (surveyTank2->Valid+2048==ready.Id) {
      IkWarningSend("Two tanks in Survey DB match tank Id %d. Using Electronics Id to desintangle...",ready.Id);
      SdTank *surveyTank3=surveyTank2;
      surveyTank2=surveyTank;
      surveyTank=surveyTank3;
    } else {
      IkSevereSend("Two tanks in Survey DB match tank Id %d. You must do the configuation by hand! (Valid=%d/%d)" ,ready.Id,surveyTank->Valid,surveyTank2->Valid);
      stringstream s; 
      s << "Two tanks in Survey DB match tank Id " << 
	ready.Id << " You must do the configuation by hand!";
      InternalLog(s.str().c_str(), IKERROR);
      return;
    }
  }
  if (surveyTank && TankDistance(&ready, surveyTank) < 200 /* meters */ ) {
    stringstream s;
    s << "Building config for Station "<< ready.Id << ". ";
    s << "It will become \""<< surveyTank->Name << "\"";
    s << " with new Id " << surveyTank->Id; 
    InternalLog(s.str().c_str(), IKINFO);
    // need to know if it's UB or UUB. UUB are V2+*
    int version;
    sscanf(ikm->SoftVersion.c_str(),"V%d",&version);
    int uub=(version<2 ? 0 : 1);
    bldConf.command = BuildConfig(surveyTank, surveyTank->Id, ready.Id,uub);
    bldConf.addresses.push_back(ready.Id);
    bldConf.send("Pm");
  }
  else {// No match in the survey data base, entry has not been made yet. 
    IkSevereSend("Station %d not yet found in survey Db", ready.Id);
    stringstream s;
    s << "Station " << ready.Id << " not yet found in survey Db";
    s << " Please update traveler survey Db!";

    surveyTank = TankBestGroundMatch(&ready, SurveyDb());
    if(surveyTank!=NULL){
      s << "Closest tank: " << 
	surveyTank->Name << " (" << surveyTank->Id << ") at " << 
	TankDistance(&ready, surveyTank) << " m"; 
      
      s << ", Closest at ground: " << 
	surveyTank->Name << " (" << surveyTank->Id << ") at " << 
	TankGroundDistance(&ready, surveyTank) << " m"; 
    } else {
      s << "Position: (Northing,Easting)=(" << 
	ready.Northing << ","<<
	ready.Easting << ") is too far from whatever Station;";
    }
    InternalLog(s.str().c_str(), IKERROR);
  }
}
