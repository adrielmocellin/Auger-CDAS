/***********************************************************************/
/* UiCmdLine.c                                                         */
/*   Handles Global variables and routines definitions  for            */
/*   for "standard" command line parsing.                              */
/*                                                                     */
/***********************************************************************/

#include <UiCmdLine.h>
#include <UiArgs.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>

char *TaskName;
char *TaskIkSuffix;
char *FdAddr;
int FdPort;
char *MoAddr;
int MoPort;
char *PmAddr;
int PmPort;

int UiParseCommandLine(int argc, char **argv)
{
  int ret = 0, portfd;
  char tmpAddr[20];
  
  /* set-up the arg table for Ui */
  UiArgsInfo argsTable[] = {
    {NULL, UI_ARGS_HELP, NULL, NULL, 
     "specify the following parameters \n\n"}, 
    {"-MoAddr", UI_ARGS_STRING, NULL, NULL, 
     "This is the IP address or host of Mo."},
    {"-MoPort", UI_ARGS_INT, NULL, NULL, 
     "This is the port number Mo will use to accept connections."},
    {"-PmAddr", UI_ARGS_STRING, NULL, NULL, 
     "This is the IP address or host of Pm."},
    {"-PmPort", UI_ARGS_INT, NULL, NULL, 
     "This is the port number of Pm."},
    {"-FdAddr", UI_ARGS_STRING, NULL, NULL, 
     "This is the IP address or host of Fd."},
    {"-FdPort", UI_ARGS_INT, NULL, NULL, 
     "This is the port number of Fd."},
    {"-Name", UI_ARGS_STRING, NULL, NULL, 
     "identifier for Ik Message (Task Name)"},   
    {"-IkSuffix", UI_ARGS_STRING, NULL, NULL, 
     "Suffix to be added to identifiers in Ik messages"},   
    {(char *)NULL, UI_ARGS_END, NULL, NULL, (char *)NULL}
  };
  
  argsTable[1].dst = &MoAddr;
  argsTable[2].dst = &MoPort;
  argsTable[3].dst = &PmAddr;
  argsTable[4].dst = &PmPort;
  argsTable[5].dst = &FdAddr;
  argsTable[6].dst = &FdPort;
  argsTable[7].dst = &TaskName;
  argsTable[8].dst = &TaskIkSuffix;
  
  /* first initialize fields */
  MoAddr="mo";
  PmAddr="pm";
  FdAddr="fd";
  TaskName=NULL;
  TaskIkSuffix=NULL;
  MoPort=0;
  PmPort=0;
  FdPort=0;
  
  if ((ret = UiParseArgs(&argc,argv, argsTable, UI_ARGS_NO_DEFAULTS ))
      == CDAS_SUCCESS) 
  {
    struct hostent *host;
    /* check for uninitialized fields */
    do
    {
      host=gethostbyname(MoAddr);
    } while (h_errno == TRY_AGAIN);
    if (host)
    {
      sprintf(tmpAddr,"%s",inet_ntoa(*(struct in_addr*)host->h_addr));
      if (MoPort==0)
      {
	sscanf(tmpAddr,"%*d.%*d.%*d.%d",&MoPort);
	MoPort*=100;
      }
    }
    else
    {
      MoAddr="127.0.0.1";
      if (MoPort==0) MoPort = 25000;
      if (!TaskIkSuffix) TaskIkSuffix = "test";
    }   
  


    do
    {
      host=gethostbyname(FdAddr);
    } while (h_errno == TRY_AGAIN);
    if (host)
    {
      sprintf(tmpAddr,"%s",inet_ntoa(*(struct in_addr*)host->h_addr));
      if (FdPort==0)
      {
	sscanf(tmpAddr,"%*d.%*d.%*d.%d",&FdPort);
	FdPort*=100;
      }
    }
    else
    {
      FdAddr="127.0.0.1";
      if (FdPort==0) FdPort = 25100;
      if (!TaskIkSuffix) TaskIkSuffix = "test";
    }

    do
    {
      host=gethostbyname(PmAddr);
    } while (h_errno == TRY_AGAIN);
    if (host)
    {
      sprintf(tmpAddr,"%s",inet_ntoa(*(struct in_addr*)host->h_addr));
      if (PmPort==0)
      {
	sscanf(tmpAddr,"%*d.%*d.%*d.%d",&PmPort);
	PmPort*=100;
      }
    }
    else
    {
      PmAddr="127.0.0.1";
      if (PmPort==0) PmPort = 25200;
      if (!TaskIkSuffix) TaskIkSuffix = "test";
    }   
    
  }
  else 
  {
    /* could not parse the input successfully */
    UiErrOutput();
    UiErrClear();
    /*    GsNotifyIk(UiGetHelp(argsTable, UI_ARGS_NO_LEFTOVERS, "XX", 5, 
	  "USAGE: ", "", "\n"));
    */
  }
  return(ret);
}



