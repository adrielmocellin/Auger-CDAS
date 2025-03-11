#include <stdio.h> 
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

extern "C"
{
#include <errno.h>
}

#include "TROOT.h"
#include "TObject.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h" 

#include "IkC.h"
#include "IkInfo.hxx"
#include "IkWarning.hxx"
#include "IkSevere.hxx"
#include "IkFatal.hxx"

#include "MoRS_Summaryfile.h"

#include "MoIO_ClassDef.h"
#include "MoIO_MoRClassDef.h"
#include "MoIO_MoRSClassDef.h"

#define kMaxFileNameSize 1024

extern string gSummaryFile;
extern string gSFilePath;
extern string gTaskName;

int gSummaryFileOkValue = 0;

TFile *gRootFileSummary = NULL;
TTree *gSDSummaryMonTree = NULL;
TTree *gSDSummaryRawMonTree = NULL;
TTree *gSDSummaryMonTreeSpecial = NULL;
TTree *gSDSummaryRawMonTreeSpecial = NULL;
TBranch *gSDSummaryMonBranch = NULL;
TBranch *gSDSummaryRawMonBranch = NULL;
TBranch *gSDSummaryMonBranchSpecial = NULL;
TBranch *gSDSummaryRawMonBranchSpecial = NULL;

TSDSummaryMonData    *gSDSummaryMonData = NULL;
TSDSummaryMonRawData *gSDSummaryRawMonData = NULL;

TFileProcessedByMoRS *gFileAlreadySummarized = NULL;

/*-----------------------------------------------------------------*/
static void BuildSummaryFilename(string fileToBeSummarized) {
  int year1, month1, year2, month2, day, hour, min;
  char path[128], temp[64];
  sscanf(fileToBeSummarized.c_str(),"%[^2]%d/%d/mc_%d_%d_%d_%dh%d.root",path,&year1,&month1,&year2,&month2,&day,&hour,&min);
  if (strcmp(gTaskName.c_str(),"")==0) sprintf(temp,"/%d/%.2d/mc_summary_%d_%.2d.root",year1,month1,year1,month1);
  else {
    if (strcmp(gTaskName.c_str(),"LSSTATUS")==0) sprintf(temp,"/mc_summary_lsstatus.root");
    else sprintf(temp,"/%d/%.2d/mc_summary_%d_%.2d.root",year1,month1,year1,month1);
  }
  gSummaryFile = gSFilePath + temp;
}

/*-----------------------------------------------------------------*/
static int SummaryFileOk() {
  struct stat filestat;
  int rc; 

  if((rc=stat(gSummaryFile.c_str(),&filestat)) == 0 && gSummaryFileOkValue == 0) {
    gSummaryFileOkValue = 1;
    IkInfoSend("File %s status Ok, Last modification date: %s      Size : %d",gSummaryFile.c_str(),
               (char*)ctime(&filestat.st_ctime),(int)filestat.st_size);
  }
  else if (rc != 0 && gSummaryFileOkValue == 1) {
    gSummaryFileOkValue = 0;
    IkSevereSend("Wrong status for File %s :: %s",gSummaryFile.c_str(),strerror(errno));  
    }
  //cout << "SummaryFileOK = " << !rc << endl;
  return (!rc);
}

/*-------------------------------------------------------------------------*/
static int BookSumTrees(Int_t number) {

  Int_t bufsize = 1000000;
  Int_t split = 1;

  switch (number) {
  case 1:
    gSDSummaryMonTree = new TTree("SDSummaryMon","Summary of the monitoring data");
    gSDSummaryMonTree->Branch("SDSummaryMonBranch","TSDSummaryMonData",&gSDSummaryMonData,bufsize,split);
    break;
  case 2:
    gSDSummaryRawMonTree = new TTree("SDSummaryRawMon","Summary of the raw monitoring data");
    gSDSummaryRawMonTree->Branch("SDSummaryMonRawBranch","TSDSummaryMonRawData",&gSDSummaryRawMonData,bufsize,split);
    break;
  case 3:
    gSDSummaryMonTreeSpecial = new TTree("SDSummarySpecial","Special Temporary Tree");
    gSDSummaryMonTreeSpecial->Branch("SDSummaryMonBranchSpecial","TSDSummaryMonData",&gSDSummaryMonData,bufsize,split);
    break;
  case 4:
    gSDSummaryRawMonTreeSpecial = new TTree("SDSummaryRawMonSpecial","Special Temporary Tree for Raw Data");
    gSDSummaryRawMonTreeSpecial->Branch("SDSummaryMonRawBranchSpecial","TSDSummaryMonRawData",&gSDSummaryRawMonData,bufsize,split);
    break;
  }
  return 0;
}

/*-----------------------------------------------------------------*/
int OpenSummaryFile(string fileToBeSummarized) {

  BuildSummaryFilename(fileToBeSummarized);

  gRootFileSummary = new TFile(gSummaryFile.c_str(),"UPDATE");

  if (gRootFileSummary == NULL) {
    IkFatalSend("File %s cannot be opened",gSummaryFile.c_str());
    cout << " FILE IS NULLLLLLLLLLLLLLLL" << endl;
    return -1;
  }

  gSDSummaryMonData    = new TSDSummaryMonData();
  gSDSummaryRawMonData = new TSDSummaryMonRawData();

  gSummaryFileOkValue = 1;

  if (CheckFileToBeSummarizedHasNotYetBeenSummarized(fileToBeSummarized)) {
    cout << "Done before ;-) -> File has already been summarized" << endl;
    gRootFileSummary->Close();
    //IkInfoSend("File %s has already been summarized",FileToBeSummarized);
    return -1;
  }
  IkInfoSend("Opening the summary file %s",gSummaryFile.c_str());

  gSDSummaryMonTree = (TTree*)gRootFileSummary->Get("SDSummaryMon");
  gSDSummaryRawMonTree = (TTree*)gRootFileSummary->Get("SDSummaryRawMon");

  if (gSDSummaryMonTree == NULL) {cout << "No Tree Found" << endl; BookSumTrees(1);}
  else {
    cout << "A Tree Found" << endl;
    gSDSummaryMonTree = (TTree*)gRootFileSummary->Get("SDSummaryMon");
    gSDSummaryMonBranch = gSDSummaryMonTree->GetBranch("SDSummaryMonBranch");
    gSDSummaryMonBranch->SetAddress(&gSDSummaryMonData);
  }

  if (gSDSummaryRawMonTree == NULL) {BookSumTrees(2);}
  else {
    gSDSummaryRawMonTree = (TTree*)gRootFileSummary->Get("SDSummaryRawMon");
    gSDSummaryRawMonBranch = gSDSummaryRawMonTree->GetBranch("SDSummaryMonRawBranch");
    gSDSummaryRawMonBranch->SetAddress(&gSDSummaryRawMonData);
  }

  //////////////////// FOR SPECIAL TREE !!!!!!!!!!!!!!!!!!!!!

  gSDSummaryMonTreeSpecial = (TTree*)gRootFileSummary->Get("SDSummaryMonSpecial");
  gSDSummaryRawMonTreeSpecial = (TTree*)gRootFileSummary->Get("SDSummaryRawMonSpecial");

  if (gSDSummaryMonTreeSpecial == NULL) {cout << "No Tree Found" << endl;BookSumTrees(3);}
  else {cout << "A Tree Found" << endl;
    gSDSummaryMonTreeSpecial = (TTree*)gRootFileSummary->Get("SDSummaryMonSpecial");
    gSDSummaryMonBranchSpecial = gSDSummaryMonTreeSpecial->GetBranch("SDSummaryMonBranchSpecial");
    gSDSummaryMonBranchSpecial->SetAddress(&gSDSummaryMonData);
  }

  if (gSDSummaryRawMonTreeSpecial == NULL) {BookSumTrees(4);}
  else {
    gSDSummaryRawMonTreeSpecial = (TTree*)gRootFileSummary->Get("SDSummaryRawMonSpecial");
    gSDSummaryRawMonBranchSpecial = gSDSummaryRawMonTreeSpecial->GetBranch("SDSummaryMonRawBranchSpecial");
    gSDSummaryRawMonBranchSpecial->SetAddress(gSDSummaryRawMonData);
  }

  Int_t nevent = (Int_t)gSDSummaryMonTree->GetEntries();

  cout << "Event already stored : "<< nevent << endl; 
  
  //for (Int_t i=0;i<nevent;i++) {
  //SDSummaryMonTree->GetEvent(i);
  //SDSummaryRawMonTree->GetEvent(i);
  //cout << "Id = " << sdsummarymondata->fSd_Id << "      Time = " << sdsummarymondata->fTime << endl;
  //}
  
  SummaryFileOk();
  return 0;
}


/*-------------------------------------------------------------------------*/
void BookSpecialTrees() {
  BookSumTrees(3);
  BookSumTrees(4);
  return;
}

/*-------------------------------------------------------------------------*/
//void DeleteSpecialTrees() {
  //if (SDSummaryMonTreeSpecial != NULL) {SDSummaryMonTreeSpecial->Delete();cout << "deleted ! ->" << endl; }
//if (SDSummaryMonBranchSpecial)     delete SDSummaryMonBranchSpecial;
  //if (SDSummaryRawMonTreeSpecial != NULL) SDSummaryRawMonTreeSpecial->Delete();
//if (SDSummaryRawMonBranchSpecial)  delete SDSummaryRawMonBranchSpecial;
//}

/*-------------------------------------------------------------------------*/
int FillSDSummary(Int_t number) {
  if (SummaryFileOk()) {
    switch (number) {
    case 1:
      //gSDSummaryMonTree->GetBranch("SD_SUMMARYMON_Branch")->SetAddress(&sdsummarymondata);
      gSDSummaryMonTree->Fill();
      break;
    case 2:
      //gSDSummaryRawMonTree->GetBranch("SD_SUMMARYMON_RAW_Branch")->SetAddress(&sdsummaryrawmondata);
      gSDSummaryRawMonTree->Fill();
      break;
    case 3:
      //gSDSummaryMonTreeSpecial->GetBranch("SD_SUMMARYMON_BranchSpecial")->SetAddress(&sdsummarymondata);
      gSDSummaryMonTreeSpecial->Fill();
      break;
    case 4:
      //gSDSummaryRawMonTreeSpecial->GetBranch("SD_SUMMARYMON_RAW_BranchSpecial")->SetAddress(&sdsummaryrawmondata);
      gSDSummaryRawMonTreeSpecial->Fill();
      break;
    }
  }
  return 0;
}

/*-------------------------------------------------------------------------*/
int CloseRootSummaryFile() {
  if (!gSummaryFileOkValue) return -1;

  gSDSummaryMonTree->AutoSave();
  gSDSummaryMonTree->Print();

  gSDSummaryRawMonTree->AutoSave();
  gSDSummaryRawMonTree->Print();

  gSDSummaryMonTreeSpecial->AutoSave();
  gSDSummaryMonTreeSpecial->Print();
  gSDSummaryRawMonTreeSpecial->AutoSave();
  gSDSummaryRawMonTreeSpecial->Print();

  gRootFileSummary->Write();
  gRootFileSummary->Map();
  gRootFileSummary->Close();

  gSDSummaryMonTree->Delete(); gSDSummaryMonTree = NULL;
  gSDSummaryRawMonTree->Delete(); gSDSummaryRawMonTree = NULL;
  gSDSummaryMonTreeSpecial->Delete(); gSDSummaryMonTreeSpecial = NULL;
  gSDSummaryRawMonTreeSpecial->Delete(); gSDSummaryRawMonTreeSpecial = NULL;

  if (gRootFileSummary != NULL)     {delete gRootFileSummary; gRootFileSummary = NULL;}
  if (gSDSummaryMonData != NULL)     {delete gSDSummaryMonData; gSDSummaryMonData = NULL;}
  if (gSDSummaryRawMonData != NULL)  {delete gSDSummaryRawMonData; gSDSummaryRawMonData = NULL;}

  cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;

  IkInfoSend("%s successfully closed.",gSummaryFile.c_str());
  return 0;
}

/*-------------------------------------------------------------------------*/
int CheckFileToBeSummarizedHasNotYetBeenSummarized(string fileToBeSummarized) {
  string afilename = fileToBeSummarized;
  // Must see if the file to be summarized has already been summarized
  gFileAlreadySummarized = (TFileProcessedByMoRS*)gRootFileSummary->Get("SUMMARIZEDFILENAME");

  if (gFileAlreadySummarized == NULL) {
    gFileAlreadySummarized = new TFileProcessedByMoRS();
    gFileAlreadySummarized->AddFileName(afilename);
    gFileAlreadySummarized->Write("SUMMARIZEDFILENAME");
    gRootFileSummary->Write();
    gRootFileSummary->ls();
    delete gFileAlreadySummarized; gFileAlreadySummarized = NULL;
    return 0;
  }

  //  cout << "Files existing in summary file before adding" << endl;
  //filealreadysummarized->PrintFileNames();

  //  cout << "A summary file has been found checking if the file to be summarized exist" << endl;
  if (gFileAlreadySummarized->CompareFileName(afilename)) return 1;

  //cout << "This file has not been summarized writing its name in the file" << endl;
  gFileAlreadySummarized->AddFileName(afilename);
  gRootFileSummary->Delete("SUMMARIZEDFILENAME;*");
  //filealreadysummarized->Write("",TObject::kOverwrite);

  gFileAlreadySummarized->Write("SUMMARIZEDFILENAME");
  cout << "Files existing in summary file after adding" << endl;
  gFileAlreadySummarized->PrintFileNames();

  gRootFileSummary->Write();
  gRootFileSummary->ls();

  delete gFileAlreadySummarized; gFileAlreadySummarized = NULL;
return 0;
}

void CheckPointers() {

  if (gRootFileSummary != NULL) {cout << "rootfile_summary = " <<gRootFileSummary << endl;}
  if (gSDSummaryMonTree != NULL) {cout << "SDSummaryMonTree = " << gSDSummaryMonTree << endl;}
  if (gSDSummaryRawMonTree != NULL) {cout << "SDSummaryRawMonTree = " << gSDSummaryRawMonTree<< endl;}
  if (gSDSummaryMonTreeSpecial != NULL) {cout << "SDSummaryMonTreeSpecial = " << gSDSummaryMonTreeSpecial<< endl;}
  if (gSDSummaryRawMonTreeSpecial != NULL) {cout << "SDSummaryRawMonTreeSpecial = " << gSDSummaryRawMonTreeSpecial << endl;}
  if (gSDSummaryMonBranch != NULL) {cout << "SDSummaryMonBranch = " << gSDSummaryMonBranch << endl;}
  if (gSDSummaryRawMonBranch != NULL) {cout << "SDSummaryRawMonBranch = " << gSDSummaryRawMonBranch << endl;}
  if (gSDSummaryMonBranchSpecial != NULL) {cout << "SDSummaryMonBranchSpecial = " << gSDSummaryMonBranchSpecial << endl;}
  if (gSDSummaryRawMonBranchSpecial != NULL) {cout << "SDSummaryRawMonBranchSpecial = " << gSDSummaryRawMonBranchSpecial << endl;}
  if (gSDSummaryMonData != NULL) {cout << "sdsummarymondata = " << gSDSummaryMonData << endl;}
  if (gSDSummaryRawMonData != NULL) {cout << "sdsummaryrawmondata = " << gSDSummaryRawMonData << endl;}
  if (gFileAlreadySummarized != NULL) {cout << "filealreadysummarized = " << gFileAlreadySummarized<< endl;}
}
