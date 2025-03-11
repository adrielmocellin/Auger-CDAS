#ifndef _RC_H_
#define _RC_H_

#include <unistd.h>

#include <iostream>
#include <string>

#include <IkC.h>
#include <DbC.h>

#include <IkLsReady.hxx>
#include <IkLsGenMess.hxx>

#define MAXID 4096
#define MAXTANK 2048

class RunControl {

 private:

  RunControl ();
  ~RunControl();

  static RunControl* _instance;
  //
  struct {SdTank Survey[MAXTANK+1]; int nSurvey;
	  time_t lastLoad; int SurveyIndex[MAXID];} _Db;

 public:
 
  static RunControl*	Instance 	();
  	 void   	Init        	(string);
  	 void   	Info        	();
	 void 		LsReadyHandler 	(IkLsReady *);
	 void 		LsGenMessHandler(IkLsGenMess *);
	 void 		LsOS9AckHandler(IkLsOS9Ack *);

  	 int    	DbLoad      	();
  static void   	RcIkHandler 	(IkMessage *);
  
  	 SdTank*	SurveyDb   	(int reload = 0);
	 int 		SurveyIndex	(int);
	 
	 SdTank*	TankBestMatch	 (SdTank *, SdTank *);
	 SdTank*	TankBestGroundMatch	 (SdTank *, SdTank *);
         SdTank*        TankNextBestMatch(SdTank *ref, SdTank *match, SdTank *List);
	 double 	TankDistance	 (SdTank*, SdTank*);
	 double 	TankGroundDistance	 (SdTank*, SdTank*);
	 bool           XyzMatch	 (SdTank*, SdTank*);
   const char*          BuildConfig	 (SdTank*, int Id=0, int CpuId=0, int uub=0);
};
 
RunControl* theRunControl ();
 
#endif
