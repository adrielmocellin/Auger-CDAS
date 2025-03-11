#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include "SPMTRoot.h"

using namespace std;

SPMTRoot::SPMTRoot()
{
  fFile=NULL;
  fOutDirPath_fmt="";
  fFileName_fmt="";
  fFileName_nobackup="";
  fFileName="";
  fNewFileInterval = 24 * 3600;
}


SPMTRoot::~SPMTRoot()
{
  if(fFile!=NULL){
    CloseFile();
  }
}

void SPMTRoot::SetOutPath(const char *path)
{
  //path is the root path where the data will be stored.
  time_t tt;

  //the format is to converted directly from the strftime function.
  fOutDirPath_fmt = path;
  fOutDirPath_fmt += "/%Y/%m"; 
  fFileName_fmt=fOutDirPath_fmt + "/spmt_%Y_%m_%d_%Hh%M.root";

  fFile=NULL;

  tt=time(NULL);
  NewFile(tt);
}

int SPMTRoot::NewFile(time_t curtime)
{
  //create the new file
  // return 1: in case of error.
  //        0: success.

  string dir_path;
  char caux[256],*cpt;

  struct tm *ttm;
  struct stat file_st;
  if(fFile!=NULL){ //the file looks to be still opened.
    return(1);
  }

  ttm=gmtime(&curtime);

  //create the directories and subdirectories if necessary.
  strftime(caux,255,fOutDirPath_fmt.c_str(),ttm);
  cpt=caux+1;
  while( (cpt=strchr(cpt,'/')) != NULL){
    *cpt='\0';
    if(stat(caux,&file_st)!=0){
      mkdir(caux, S_IRWXU |S_IRWXG | S_IRWXO);
    }
    *cpt='/';
    cpt++;
  }
  if(stat(caux,&file_st)!=0){
    mkdir(caux, S_IRWXU |S_IRWXG | S_IRWXO);
  }

  //create the file.
  strftime(caux,255,fFileName_fmt.c_str(),ttm);
  fFileName=caux;
  fFileName_nobackup=fFileName+".nobackup";
  printf("File create: %s\n",fFileName_nobackup.c_str());
  fFile=new TFile(fFileName_nobackup.c_str(),"RECREATE");
  if(fFile->IsZombie()){
    delete(fFile);
    fFile=NULL;
    fFileTimeNext=0;
    return(1);
  }
  fFile->SetCompressionLevel(2);

  fTree = new TTree("spmt_monit","events for spmt calib");
  fTree->Branch("data",&fData);
  //it would auto save the tree every time the amount of data is
  // bigger than 10Mb
  fTree->SetAutoSave(-10000000);

  //set the time which the file should be closed and new file open.
  fFileTimeNext  = curtime + fNewFileInterval;
  fFileTimeNext -=  fFileTimeNext % fNewFileInterval;

  return(0);
}

void SPMTRoot::CloseFile()
{
  size_t sufix_pos;
  if(fFile!=NULL){
    fFile->cd();
    fFile->Write();
    delete(fTree);
    fFile->Close();
    delete(fFile);
    fFile=NULL;
    /*rename the file spmt_...root.nobackup to spmt_...root */
    if(fFileName_nobackup.find(".nobackup")!=string::npos){
      rename(fFileName_nobackup.c_str(),fFileName.c_str());
    }
    fFileName="";
    fFileName_nobackup="";
  }
}

int SPMTRoot::AddData(spmtIO *dat)
{
  //
  time_t curtime;
  curtime=time(NULL);

  if(fFile==NULL || fFileTimeNext <= curtime ){
    if(fFile != NULL){
      CloseFile();
    }
    if( NewFile(curtime) ){
      return(1);
    }
  }

  fFile->cd();
  fData.ResetData();

  fData.LsId   = dat->LsId;
  fData.evt    = dat->evt;
  fData.muon   = dat->muon;
  fData.thres  = dat->thres;
  fTree->Fill();
  return(0);
}
