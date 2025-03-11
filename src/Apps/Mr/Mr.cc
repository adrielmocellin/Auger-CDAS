// For Debugging :
// run -Name Mr -PmAddr 192.168.1.110 -MoAddr 192.168.1.104 -FdAddr 192.168.2.105 -PmPort 10600 -MoPort 10400 -FdPort 10500 -IkSuffix __Cyril

#include <iostream>
#include <time.h>
#include <sys/time.h>

#include <IkC.h>
#include <IkFatal.hxx>
#include <IkSuPing.hxx>
#include <IkSuPong.hxx>
#include <IkMoRBookTree.hxx>


#include <DbC.h>
#include <DbCConfig.h>

#include <UiErr.h>
#include <UiCmdLine.h>
#include <UiArgs.h>   // had to be added with respect to c version
#include <UiMessage.h>

#include "MrManager.h"
#include "MrRoot.h"
#include "MrSignal.h"

string gPmName;
string gMrName;

string gExecPath;
string gRootFilePath;
string gSuffix;

extern time_t gLastBookingSecond;

void MrIkHandler(IkMessage *message) {
  //
  // Handle Ik message we receive.
   //

  IkSuPong mpong;

  switch(message->get_type()) {
    case IKSUPING:
      mpong.PongValue = ((IkSuPing*)message)->PingValue;
      mpong.send(message->get_sender().c_str());
      break;

    case IKMORBOOKTREE:
      if (((IkMoRBookTree*)message)->fBookTreeNow == 1) 
	gLastBookingSecond = 0;
      if (((IkMoRBookTree*)message)->fBookTreeTimeValue != 0)
	cout << "For the moment Mr doesn't change the Booking Time Value " << endl;
      break;
  }
}

cdasErrors MrConfig() {
  // to do 
  return CDAS_SUCCESS;
}

void Info() {

}

int main(int argc, char **argv) {
  //
  // Main loop of the Mr package.
  //

  TMrManager manager;

  InitUnixSignals(); // Trap the final signal to close the rootfile properly

  /* parse command line */
  if ( UiParseCommandLine (argc,argv) != CDAS_SUCCESS ) {
    cerr << "Error in command line" << endl;
    return 0;
  }

  /* Initialisation to CDAS  */
  gPmName = PM;
  gMrName = "Mr";
  if (TaskIkSuffix) {
    gMrName += TaskIkSuffix;
    gPmName += TaskIkSuffix;     
  }
  CDASInit(gMrName.c_str());
  
  gExecPath=argv[0];
  //gExecPath.erase(gExecPath.find("Mr.exe"),7);   FIXME doesn't work. We don't care as MoRS is not used anymore

  gSuffix="";

  if (TaskName) {
    if (TaskIkSuffix) {
      gSuffix = TaskIkSuffix;
      gRootFilePath = "/Raid/tmp/";
      gRootFilePath += TaskIkSuffix;
      gRootFilePath += "/monit/";
    } else gRootFilePath = kMrFilePath; 
  } else gRootFilePath = kMrFilePath;

#ifdef DEBUGGING
  cout << endl << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl<< endl;
  cout << "     Mr HAS BEEN LAUNCHED IN DEBUGGING MODE : BE CAREFULL    " << endl;
  cout << "      Change the requirement file for normal compilation    " << endl;
  cout << endl << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl<< endl;
#endif

  /* Asking for Ik Messages */
  char listen[100];
  sprintf (listen,"destination is \"%s\" or type is %d ",gMrName.c_str(),IKMORBOOKTREE);
  IkMonitor(MrIkHandler,listen);

  /* Init connection to Pm */
  if (manager.ConnectToPm(PmPort, PmAddr) != CDAS_SUCCESS) {
    IkFatalSend("ERROR : Can't connect to Pm server. Bye");
    return 0;
  }
  
  /* read config file */
  if ( MrConfig() != CDAS_SUCCESS ) IkWarningSend("Can't read config");

  /* Root stuff initialization */
  OpenRootFile();
  BookTree();

  /* Main loop  */
  while(1) {

    // process interrupts
    manager.Interrupts();

    // Reception of the data
    manager.ListenPm();

    // Request calibration block if needed

    manager.RequestCalibFromSD();
  }

  CloseRootFile();

  return 0;
}
