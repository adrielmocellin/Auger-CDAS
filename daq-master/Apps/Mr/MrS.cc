#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef DPA
#include "IkC.h"
#include "IkInfo.hxx"
#include "IkWarning.hxx"
#include "IkSevere.hxx"
#include "IkFatal.hxx"
#endif

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h" 

#include "MoIO_ClassDef.h"

#include "MrSMessage.h"
#include "MrSSummaryFile.h"
#include "MrSClassDef.h"

#define kMaxFileNameSize 1024
#define kDelay 0 // sleep delay before launching the summary, not usefull now

#ifndef DEBUGGING
#define kOffset 3600 // Compression = one value/hour
#else
#define kOffset 360  // Compression = one value/6 minutes for debugging
#endif

string gSummaryFile;
string gTaskName;
string gSFilePath;

int gLowerIndexForFile=0, gUpperIndexForFile=0;

TROOT MrS("MrS","Monitoring and Calibration Summary"); 

extern TFile *gRootFileSummary;

TFile *gRootFileToBeSummarized;

TSDMonCal *gSDMonCal;

TTree   *gSDMonCalTree;
TBranch *gSDMonCalBranch;

static void MrSUsage() {
  cout << endl << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl << endl;
  cout << " To launch MrS : you must specify a taskname !!!" << endl;
  cout << " MrS -taskname __Cyril -sumfilepath myfilepath -file mor_2001...root" << endl;
  cout << " If no sumfilepath is specified default is /Raid/tmp/Taskname/ " << endl;
  cout << " In the CDAS, MrS is runned with : " << endl;
  cout << " MrS -taskname -file mor_2001...root" << endl;
  cout << " A real example : " << endl;
  cout << " MrS -taskname CYRIL -sumfilepath /Raid/tmp/__Cyril -file /Raid/monit/2001/12/mor_2001_12_07_23h21.root" << endl;
  cout << endl << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl << endl;
}


static int OpenMrFile(string mrFileToOpen) {

  gSDMonCal = new TSDMonCal();
  
  gRootFileToBeSummarized = new TFile(mrFileToOpen.c_str(),"READ");
  
  if(gRootFileToBeSummarized == NULL) return -1;
  string message = "Opening file for summary : " + mrFileToOpen;
  SendMessage(message.c_str(),EIkInfo);

  gSDMonCalTree = (TTree*)gRootFileToBeSummarized->Get("SDMonCal");
  
 if (gSDMonCalTree != NULL) {
   gSDMonCalBranch = gSDMonCalTree->GetBranch("SDMonCalBranch");//->SetAddress(&sdmondata);
   if (gSDMonCalBranch != NULL) gSDMonCalBranch->SetAddress(&gSDMonCal);
 }
 else if (((gRootFileToBeSummarized->Get("SDMON"))!= NULL) || (gRootFileToBeSummarized->Get("SDMon")!= NULL)) {
   SendMessage("You are trying to summarize an old monitoring file. Use MoRS instead.",EIkInfo);
 }
 
 return 0;
}

static int CloseMrFile() {
  gRootFileToBeSummarized->Close();
  delete gSDMonCal;    gSDMonCal = NULL;
  delete gRootFileToBeSummarized; gRootFileToBeSummarized = NULL;
  return 0;
}

static int MrSParseCommandLine(int argc, char **argv) {
  //cout << "Command line " << endl;
  //cout << argc << " elements" << endl;
  //for (int i = 0; i < argc; i++) cout << argv[i] << endl;
  
  // Command line must be something like
  // MrS.exe -taskname __Cyril -file f1.root f2.root ...
  // MrS.exe -taskname -file f1.root f2.root ... for the real CDAS application

  if (argc <= 3) 
    MrSUsage();
  
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i],"-taskname") == 0) {
      if (strcmp(argv[i+1],"-file") == 0) {
	gTaskName=""; 
	gSFilePath = kMrFilePath;
      }
      else {
	gTaskName=argv[i+1];
	if (strcmp(gTaskName.c_str(),"LSStatus")==0) cout << " We have been called by LSSTATUS " << endl;
	else gSFilePath = "/Raid/tmp/"+gTaskName+"/";
      }
      //cout << "TaskName " << TaskName << endl;
    }
    
    if (strcmp(argv[i],"-sumfilepath") == 0)
      gSFilePath=argv[i+1];
    
    if (strcmp(argv[i],"-file") == 0 && argc >= i+1) {
      gLowerIndexForFile = i+1;
      gUpperIndexForFile = argc;
    }
  }
  
  if (gLowerIndexForFile == 0) {
    cerr << "No file to summary -> no summary has been done." << endl;
    return 1;
  }
  
  struct stat filestat;
  int rc; 
  
  if((rc=stat(gSFilePath.c_str(),&filestat)) == -1) {
    IkFatalSend("File %s cannot be opened",gSummaryFile.c_str());
    cout << " FILE IS NULL" << endl;
    return -1;
  }
  
  return 0;
}

static void LoopOnTree() {

  Bool_t this_is_the_end = 1;
  Bool_t is_this_the_first_slice_in_time = 1;
  Bool_t is_this_the_last_slice_in_time = 0;

  UInt_t lowertime, uppertime, someseconds, atime;

  Int_t nevent = (Int_t)gSDMonCalTree->GetEntries();

  UInt_t max_time = (UInt_t)gSDMonCalTree->GetMaximum("fTime");

  while (this_is_the_end) {

    TSDArray *arrayofLS = new TSDArray();

    if (is_this_the_first_slice_in_time == 1) {
      cout << endl << "- First Slice -" << endl;
      // First slice must do things special...
      // Have to integrate data coming from a previous file
      lowertime = (UInt_t)gSDMonCalTree->GetMinimum("fTime");
      someseconds = lowertime % 3600;
      lowertime = lowertime - someseconds;
      uppertime = lowertime + kOffset;

      arrayofLS->FillArrayWithExistingValues(lowertime,uppertime);

      gRootFileSummary->Delete("SDSummaryMonSpecial;*");
      gRootFileSummary->Delete("SDSummaryRawMonSpecial;*");

      is_this_the_first_slice_in_time = 0;
    }

    for (Int_t i=0;i<nevent;i++) {
      gSDMonCalTree->GetEvent(i);
      atime = lowertime+1800;
      if (gSDMonCal->fTime >= lowertime && gSDMonCal->fTime < uppertime)
	arrayofLS->FillOneLSWithMoRValues(gSDMonCal,atime);
    }

    if (uppertime > max_time) is_this_the_last_slice_in_time = 1;

    if (is_this_the_last_slice_in_time == 1) {
      //cout << endl << "- Last Slice -" << endl;
      // Last slice must do things special...
      // Write summary in special tree, not in the true summary tree
      //First REBOOK the Special Trees...
      BookSpecialTrees();
      arrayofLS->StoreData(2);
      this_is_the_end = 0; // exit the loop on tree...
    } else {
      //cout << endl << "- Other Slice -" << endl;
      arrayofLS->ComputeAverage();
      arrayofLS->StoreData(1);
      lowertime = uppertime;
      uppertime = uppertime + kOffset;
    }
    delete arrayofLS;
  }
}

/**************************************************************************
***************************************************************************/

int main(int argc , char** argv) {

  cout << "\n\n ************************************************************************** \n\n" << endl;
  cout << "        MMMMM     MMMM                  SSSSSSS" << endl;
  cout << "        MMMMMM   MMMMM                SSSSSS" << endl;
  cout << "        MMMM MM MM MMM              SSSSSS" << endl;
  cout << "        MMMM  MMM  MMM               SSSSSS" << endl;
  cout << "        MMMM       MMM   ooo  ooooo    SSSSS" << endl;
  cout << "        MMMM       MMM   ooo oooo       SSSSSS" << endl;
  cout << "        MMMM       MMM   ooooo            SSSSSS" << endl;
  cout << "        MMMM       MMM   ooo              SSSSSS" << endl;
  cout << "        MMMM       MMM   ooo           SSSSSSS" << endl;
  cout << "        MMMM       MMM   ooo         SSSSSSSS\n\n" << endl;
  cout << " ************************************************************************** \n\n" << endl;

  // Command line must be something like
  // MoRS.exe -taskname __Cyril -file f1.root f2.root ...
  
  if (MrSParseCommandLine(argc,argv) != 0) {
    cerr << "Problem : MrS not running. Dying now !!!" << endl;
    exit(1);
  }

#ifndef DPA
  string IkTaskRegister;
  IkTaskRegister="MrS"+gTaskName;
  CDASMultipleInit(IkTaskRegister.c_str());
  IkMonitor(HandleMessage,"destination is \"%s\"",IkTaskRegister.c_str());
#endif

  sleep(kDelay);
  
  string fileToBeSummarized;
  
  for (int i = gLowerIndexForFile; i < gUpperIndexForFile; i++) {
    fileToBeSummarized = argv[i];
    // Now start to open the files...
    if (OpenMrFile(fileToBeSummarized)==-1) {
      SendMessage("No monitoring root file opened",EIkSevere);
      break;
    }
    if (OpenSummaryFile(fileToBeSummarized)==0) {
      LoopOnTree();
      CloseRootSummaryFile();
    }
    CloseMrFile();
  }

  CheckPointers();

  SendMessage("MrS task is completed. MrS will come back later. Ciao.",EIkInfo);
  SendMessage("MrS task is completed. MrS will come back later. Ciao.",EIkSpeech);

  //IkSoundSend("loser.wav",10); IkSoundSend("Goodbye.wav",10);
  SendMessage("Exiting from MrS...");
  return 0;
}
