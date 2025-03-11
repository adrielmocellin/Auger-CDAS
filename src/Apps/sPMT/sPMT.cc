#include <iostream>
#include <time.h>
#include <sys/time.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


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

#include <string.h>
#include <errno.h>

#include "sPMT_Pm.h"
#include "SPMTData.h"

#define BUFF_SIZE 102400


//struct spmt_debug
//{
//  int index;
//  int max_files;
//  char fname[100];
//  FILE *arq;
//};

void SPMTIkHandler(IkMessage *message) {
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
	cout << "Feature not implemented "<<endl;
      if (((IkMoRBookTree*)message)->fBookTreeTimeValue != 0)
	cout << "For the moment sPMT doesn't change the Booking Time Value " << endl;
      break;
  }
}

int main(int argc, char **argv)
{
  string PmName;
  string SpmtName;
  string ExecPath;
  string RootFilePath;

  sPMT_Pm *pmconn;
  int fd_pm;
  union {
    int ibuf[BUFF_SIZE];
    char cbuf[BUFF_SIZE*sizeof(int)];
  } buff;

  int nbuff;
  SPMTData *data_store;
  fd_set rdfds,rdfdsaux;
  struct timeval timeout;
  int select_flag,ok,err;

  //struct spmt_debug debug;
  //debug.index=0;
  //debug.max_files=10;


  /* parse command line  */
  if ( UiParseCommandLine (argc,argv) != CDAS_SUCCESS ) {
    cerr << "Error in command line" << endl;
    return 0;
  }
  /* Initialisation to CDAS  */
  PmName = PM;
  SpmtName = "sPMT";
  if (TaskIkSuffix!=NULL){

    SpmtName += TaskIkSuffix;
    PmName += TaskIkSuffix;

    RootFilePath = "/Raid/tmp/";
    RootFilePath += SpmtName + TaskIkSuffix;
    RootFilePath += "/monit/";
  } else {
    RootFilePath = kSpmtFilePath; //defined at CMakeList.txt
  }

  /* connect to Ik and Pm */

  CDASInit(SpmtName.c_str());

  ExecPath=argv[0];

  char listen[100];
  sprintf (listen,"destination is \"%s\" or type is %d ",
	   SpmtName.c_str(),IKMORBOOKTREE);
  IkMonitor(SPMTIkHandler,listen);


  pmconn=new sPMT_Pm(BUFF_SIZE);

  if(pmconn->ConnectToPm(PmPort,PmAddr) != CDAS_SUCCESS ){
    IkFatalSend("ERROR : Can't connect to Pm server. Bye");
    return(0);
  }

  FD_ZERO(&rdfds);

  fd_pm=pmconn->GetConn();

  FD_SET(fd_pm,&rdfds);
  
  data_store=new SPMTData();
  data_store->SetOutPath(RootFilePath.c_str());

  timeout.tv_sec=2;
  timeout.tv_usec=0;
  ok=1;
  while(ok) {
    rdfdsaux=rdfds;
    select_flag=select(fd_pm+1, &rdfdsaux, NULL,NULL,&timeout);
    if(0<select_flag){
      if(FD_ISSET(fd_pm,&rdfdsaux)){
	if(pmconn->ReadFromPm(buff.ibuf,&nbuff)==CDAS_SUCCESS){
	  //if(debug.index < debug.max_files){
	  //  sprintf(debug.fname,"spmt_out_%02d",debug.index);
	  //  debug.arq=fopen(debug.fname,"w");
	  //  if(debug.arq!=NULL){
	  //    printf("Writting %d\n",nbuff);
	  //    fwrite(buff.cbuf,1,nbuff,debug.arq);
	  //    fclose(debug.arq);
	  //  }
	  //} else {
	  //  printf("New buffer %d; size=%d\n",debug.index,nbuff);
	  //}
	  //debug.index++;
	  err=data_store->SetData(buff.cbuf);
	} else {
	  if(errno!=EINTR){ //the select error is not due to a signal
	    printf("Error while reading data from Pm. Bye ...\n");
	    IkMonitor(SPMTIkHandler,"Error reading Pm data. Bye ");
	    ok=0;
	  } else {
	    printf("select. receive signal\n");
	  }
	}
      }
    } else {
      if(select_flag==0){
	IkMessageCheck();
	timeout.tv_sec=2;
	timeout.tv_usec=0;
      } else {
	IkMonitor(SPMTIkHandler,"Some error with select. Bye.");
	printf("select error ...: %d. Error: %s (%d)\n",select_flag,strerror(errno),errno);
	ok=0;
      }
    }
  }
  printf("spmt- some error - ok=%d\n",ok);
  delete(data_store);//it will close automatically the open root file.
  return 0;
}
