#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h" 
//#include "TChain.h" 
//#include "TApplication.h"
//#include "TCanvas.h"

#include "TSDMonCal.h"    // Def n MoIO

#include "MrRoot.h"

#include "IkC.h"

#include "IkInfo.hxx"
#include "IkWarning.hxx"
#include "IkSevere.hxx"
#include "IkFatal.hxx"

extern "C"
{
#include <errno.h>
}

// Root variables:
TROOT Mr("Mr","SD Monitoring and Calibration (Pierre Auger Observatory)");
TFile*     gRootFile;

TTree*     gSDMonCalTree;
TSDMonCal* gSDMonCal;

#define kMaxFileNameSize 1024

#ifndef DEBUGGING
#define kBookingTime     1200 // Booking the trees every 20 minutes (1200 seconds)
#else
#define kBookingTime     100 // Booking the trees every 100 seconds (DEBUGGING mode)
#endif

extern string gExecPath;
extern string gRootFilePath;
extern string gSuffix;

static string gRootFileName;
static string gRootFileNameForRename;
string gRootFileId;

int gFileOkValue = 0;
static struct tm* gOpenFileTime;
static int gOpenFileDay;
time_t gLastBookingSecond = 0;

/*-----------------------------------------------------------------*/
int FileOk() {
  struct stat filestat;
  int rc;

  if((rc=stat(gRootFileName.c_str(),&filestat)) == 0 && gFileOkValue == 0) {
    gFileOkValue = 1;
    IkInfoSend("File %s status Ok, Last modification date: %s      Size : %d", gRootFileName.c_str(),
               (char*)ctime(&filestat.st_ctime),(int)filestat.st_size);
  }
  else if (rc != 0 && gFileOkValue == 1) {
    gFileOkValue = 0;
    IkSevereSend("Wrong status for File %s :: %s",gRootFileName.c_str(),strerror(errno));  
  }
return (!rc);
}
/*-----------------------------------------------------------------*/


/*-----------------------------------------------------------------*/
int FileSize() {
  struct stat filestat;
  int rc;
  if (!gFileOkValue) return 0;
  if((rc=stat(gRootFileName.c_str(),&filestat)) == 0) return filestat.st_size;
  else {
    cerr << "error in stat ::" << strerror(errno)<< endl;
    return 0;
  }
}

/*-----------------------------------------------------------------*/
static void BuildFileName() {
  time_t currenttime;
  struct tm *ptms;
  char filename[kMaxFileNameSize];
  char temp[kMaxFileNameSize+128];

  currenttime=time((time_t *)NULL);
  gOpenFileTime = gmtime(&currenttime);
  gOpenFileDay = gOpenFileTime->tm_mday;
#ifdef DEBUGGING
  gOpenFileDay = gOpenFileTime->tm_hour;
#endif
  ptms = gOpenFileTime;
  if(strlen(gRootFilePath.c_str())!=0) { 
    snprintf(filename,kMaxFileNameSize,"mc_%d_%.2d_%.2d_%.2dh%.2d.root",
	     ptms->tm_year+1900,ptms->tm_mon+1,ptms->tm_mday,ptms->tm_hour,ptms->tm_min);
    gRootFileId = filename;
    snprintf(temp,kMaxFileNameSize+128,"%s%d/%.2d/%s",gRootFilePath.c_str(),ptms->tm_year+1900,ptms->tm_mon+1,filename);
    gRootFileNameForRename = temp;
    gRootFileName = gRootFileNameForRename + ".nobackup";
  }
}

/*-----------------------------------------------------------------*/
bool CloseFileTest() {
  time_t currenttime; struct tm * ptms; 
  currenttime=time((time_t *)NULL);
  ptms=gmtime(&currenttime);
  
#ifdef DEBUGGING
  return (ptms->tm_hour != gOpenFileDay);
#endif
  return (ptms->tm_mday != gOpenFileDay);
}

/*-----------------------------------------------------------------*/
bool BookingTreeTest() {
  time_t currenttime;
  currenttime=time((time_t *)NULL);
  
  if (abs(currenttime-gLastBookingSecond) > kBookingTime) gLastBookingSecond = currenttime;
  return (gLastBookingSecond == currenttime);
}

/*-------------------------------------------------------------------------*/
int OpenRootFile() {
  char * tmp;
  char * buf;
  BuildFileName();
  
  // Create the directory in which Mr files are stored if it does not exist
  tmp=strdup(gRootFileName.c_str());
  buf=tmp;
  while(++tmp=strchr(tmp,'/')) {
    *tmp='\0';
    mkdir(buf,7*8*8+7*8+7);
    *tmp='/';
  }
  free(tmp);

  gRootFile = new TFile(gRootFileName.c_str(),"RECREATE");
  gRootFile->SetCompressionLevel(2);
  
  if(gRootFile==NULL) {
    IkSevereSend("No root filename declared in MoR");
    return -1;
  }
  gFileOkValue = 1;
  IkInfoSend("Opening file %s", gRootFileName.c_str());
  return FileOk();
}
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
int CloseRootFile() {
  if (!gFileOkValue) return -1;

  gSDMonCalTree->AutoSave();
  gSDMonCalTree->Print();
  gSDMonCalTree->Delete();

  gRootFile->Write();
  gRootFile->Map();
  gRootFile->Close();
  if (gRootFile != NULL) delete gRootFile;
  rename(gRootFileName.c_str(),gRootFileNameForRename.c_str());
  IkInfoSend("%s successfully closed.",gRootFileNameForRename.c_str());
  return 0;
}
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
int BookTree() {
  Int_t bufsize = 16000;
  Int_t split = 2;

  gSDMonCalTree = new TTree("SDMonCal","Monitoring and Calibration (Pierre Auger Observatory)");
  gSDMonCalTree->Branch("SDMonCalBranch","TSDMonCal",&gSDMonCal,bufsize,split);
  return 0;
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
int FillSDMonCal() {
  if (FileOk()) gSDMonCalTree->Fill();
  return 0;
}
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
int CallMoRSForSummary() {
  string commandline;
  commandline =  "nice -19 ";
  commandline += gExecPath;
  commandline += "MoRS.exe";
  commandline += " -taskname ";
  commandline += gSuffix;
  commandline += " -file ";
  commandline += gRootFileNameForRename;
  //commandline += " &";
  cout << commandline << endl;
  const char* command = commandline.c_str();
  system(command);
  return 0;
}
