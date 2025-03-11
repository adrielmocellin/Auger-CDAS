#include <iostream>
#include <list>

#include "Eb.h"
#include "EbEvent.h"
#include "EbManager.h"

#include <IkC.h>
#include <IkFatal.hxx>
#include <IkDbConfig.hxx>
#include <IkSDSummary.hxx>
#include <IkSDUpdate.hxx>
#include <DbC.h>
#include <DbCConfig.h>
#include <Pm.h>
#include <UiCmdLine.h>
#include <UiArgs.h> // had to be added with respect to c version
#include <UiErr.h> 
#include <UiMessage.h>

string gPmName;
string gEbName;
string gXbName;
int gDebug = 1, gVerbose = 1, gSuperVerbose = 0;

void info ()
{

}

void EbIkHandler(IkMessage *ikm) 
{
  switch (ikm->get_type()) 
    {
    case IKT3:
      if ((TaskIkSuffix && strstr(ikm->get_sender().c_str(),TaskIkSuffix)) ||
	  (TaskIkSuffix==NULL && strncmp(ikm->get_sender().c_str(),"Ct",2)==0) ||
	  (TaskIkSuffix==NULL && strncmp(ikm->get_sender().c_str(),"Xb",2)==0) ||
	  (TaskIkSuffix==NULL && strncmp(ikm->get_sender().c_str(),"Fd",2)==0))
        if ( strcmp (((IkT3*)ikm)->algo.c_str(), "IGNORE") != 0 )
          TheEbManager ()->AddIkT3 ((IkT3 *)ikm);
      break;
    case IKSUPING:
      TheEbManager ()->IkPingValue = ((IkSuPing*)ikm)->PingValue;
      //TheEbManager()->GetPingSender(ikm->get_sender());
      break;
//     case IKSDSUMMARY:
//       if ((TaskIkSuffix && strstr(ikm->get_sender().c_str(),TaskIkSuffix)) ||
// 	  (TaskIkSuffix==NULL && strncmp(ikm->get_sender().c_str(),"Ct",2)==0) ||
// 	  (TaskIkSuffix==NULL && strncmp(ikm->get_sender().c_str(),"Xb",2)==0))
// 	TheEbManager ()->GetSDStatus ((IkSDSummary*)ikm);
//       break;
//   case IKSDUPDATE:
//     if ((TaskIkSuffix && strstr(ikm->get_sender().c_str(),TaskIkSuffix)) ||
// 	(TaskIkSuffix==NULL && strncmp(ikm->get_sender().c_str(),"Ct",2)==0) ||
// 	(TaskIkSuffix==NULL && strncmp(ikm->get_sender().c_str(),"Xb",2)==0))
//       TheEbManager ()->GetSDStatusUpdate ((IkSDUpdate*)ikm);
//     break;
    case IKDBCONFIG:
      if ( strstr ((((IkDbConfig *)ikm)->apps).c_str(), "Eb") )
	{
	  TheEbManager()->ResetIo();
	  IkInfoSend ("New array configuration read.");
	}
      break;
    case IKSUKILL:
      break;
    default:
      IkWarningSend ("Received unknown message: %s", ikm->to_text().c_str());
    }
}

int EbMain (int argc, char ** argv) 
{

  /* parse command line */
  if ( UiParseCommandLine (argc,argv) != CDAS_SUCCESS )
    {
      stringstream s;
      s << "Error in command line";
      InternalLog(s.str().c_str(), IKFATAL);
      exit (1);
    }
  if(TaskName==NULL){
      stringstream s;
      s << "TaskName not defined. Use command option \"-Name <taskname>";
      InternalLog(s.str().c_str(), IKFATAL);
      exit (1);
  }
  /* Initialisation to CDAS  */
  gPmName = PM;
  gEbName = TaskName; // Using TaskName
  gXbName = "Xb";
  if (TaskIkSuffix) 
    {
      gEbName += TaskIkSuffix;
      gPmName += TaskIkSuffix;     
      gXbName += TaskIkSuffix;     
    }
  CDASInit (gEbName.c_str());

  /* Asking for Ik Messages */
  char listen[100];
  sprintf (listen, "destination is \"%s\" or type is %d", gEbName.c_str(), IKT3);
  IkMonitor (EbIkHandler, listen);

  /* Init connection to Pm */
  if ( TheEbManager () -> ConnectToPm (PmPort, PmAddr) != CDAS_SUCCESS ) {
    IkFatalSend("ERROR : Can't connect to Pm server. Bye");
    exit(0);
  }

  /* read config file */
  //  if ( EbConfig () != CDAS_SUCCESS )
  //    {
  //      IkWarningSend ("Can't read config, default one is verbose and debug");
  //    }

  /* Read the last identifier event number */
  /* Builds old events if necessary */
  TheEbManager () -> Init ();

  /* Main loop  */
  struct timeval tv1,tv2;
  while(1) 
    {
      /* Reception of the trigger or of a ping value */
      TheEbManager()->CheckNewT3FromCt(); 

      /* Reception of the data */
      gettimeofday(&tv1,NULL);
      while ( (TheEbManager () -> CheckNewT3FromPm ()) == CDAS_SUCCESS ) 
	{
	  /* pushes data in event's list */
	  TheEbManager () -> PushDataInEventsList ();
	}
      /* Writes on disk finished events and cleans the event's list */
      TheEbManager () -> SearchEventsFinished (); 
      gettimeofday(&tv2,NULL);
      if((tv1.tv_sec-tv2.tv_sec)*1000000+(tv1.tv_usec-tv2.tv_usec)>100000) {
        InternalLog("pb pm...",IKINFO);
        cerr<<"pb-pm:"<<(tv1.tv_sec-tv2.tv_sec)*1000000+(tv1.tv_usec-tv2.tv_usec)<<endl;
      }
      /* if necessary */
      TheEbManager () -> IkPong ();
    }
  
  return 0;
}
