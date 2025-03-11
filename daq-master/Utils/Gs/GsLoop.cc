/*--------------------------------------------------------------------------------*/
/*
  GsLoop.c
  ...........handles the main loop and ethernet related  management
*/
/*--------------------------------------------------------------------------------*/

/*! 
  The Gs Ip Server wait for connections on the server port. Once a connection
  is accepted, it "forks" a new manager (GsClientManager) for the incoming 
  client. 
  Every time the server stops waiting (because of an incoming connection or
  as a result of a timeout, "ancillary" processing (e.g. Ik input management) 
  is performed.
*/



/* tcp/ip related stuff */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

/* miscellanea */
#include <signal.h>

/* Gs related Stuff */
#define GSNOTIFYMAIN  /* TO BE CHANGED, global var in c++ */
#define GSIKMAIN
#define GSMAIN
#include "GsServer.h"
#include "GsServices.h"
#include "GsClients.hxx"

#include "Gsp.h"
#include "GsThreads.h"

#include <UiCmdLine.h> 
#define max(i0,i1) (i0<i1?i1:i0)
#define FOREVER		for(;;)    

/*---------------------------------------------------------------------------*/


  // find a way to clean everything !
  // TO BE DONE : how to maintain a list of ACTIVE managers ?
GsThread ClientManager[MAXDC];

/*--------------------------------------------------------------------------------*/
  // TO BE MOVED ELSEWHERE
int removeDataClient(int code,GsClientP client)
{
  unsigned int fd;
  fd=GsClientFd(client);
  GsNotifyIk(TOLOG,IK_INFO,
            "removeDataClient(%d)",fd);

  GsThreadDestroy(ClientManager[fd]);
  return(code);
}


/*--------------------------------------------------------------------------------*/
/* GsTerminate, to start with ;-) */ 

void GsTerminate(int errorCode)
{
  int i;
  /* close all client connections */
  GsNotifyIk(TOLOG,IK_FATAL," Terminating with code %d",errorCode);
  for (i=0;i<MAXDC;i++)
  {
    GsThreadDestroy(ClientManager[i]);
  }
    exit(errorCode);
}
/*--------------------------------------------------------------------------------*/

static int trailerOk(int * trailerP, int version)
{
  if (version < 100) return (*trailerP == GsMESSAGESENDMARKER);
  return(-1);
}
/*--------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
void GsLoop(/* TO BE CHANGED */ int dataPort, int data)
{
  //! GsLoop is called with the file descriptor to watch for
  
  int nfd;
  socklen_t 
    rSize;
  unsigned long int
    newClient;
  
  struct sockaddr_in
    hsta;

  fd_set
    crfds;
 



  struct timeval tv;
 

  /*********************************************************************/
  /* try to exit without blocking TCP ports */
  GsSetSignalHandling(GsTerminate);



  GsNotifyIk(TOLOG,IK_INFO,
	     "entering Gsloop");

  FOREVER
  {
    
    FD_SET (data, &crfds);
    /* don't sleep forever in order to exit properly */
    tv.tv_sec=0;
    tv.tv_usec=400; /*TO BE CHECKED how long should we wait for (Ik vs load) */
    
    nfd=select(data+1,&crfds,NULL,NULL,&tv); /* somebody here ? */
    switch(nfd) 
    {
    case -1 : /* select didn't work : that's bad ! */
      GsNotifyIk(TOLOG,IK_SEVERE,
		 "select() error : %s on fd %d ",strerror(errno), data);
      // TO BE DONE : reset TCP/IP config
      break;
    case 0 : /* nobody's comming : GOOD, I'm lazzy  */
      break;
    default : /* somebody's connecting */
      break;
    } /* switch nfd */
    
    if(FD_ISSET(data,&crfds)) // new connection
    {
#ifdef VERBOSE
      // TO BE DONE : extract the IP address
      GsNotifyIk(TOLOG,IK_INFO,
		 " new connection on %d fd from ??",data);
#endif
      nfd--;
      rSize=sizeof(struct sockaddr_in);
      if((newClient = accept(data,(struct sockaddr *)&hsta,&rSize))==(-1)) 
      {
	GsNotifyIk(TOLOG,IK_SEVERE,
		   "accept() error : %s",strerror(errno));
      }
      else  /* accept worked */
      {
	//       launch manager
	GsThreadLaunch(&(ClientManager[newClient]),
		       GsClientManager,
		       (void*)newClient);
      } /* else (accept worked )*/

      // for debugging puposes : 
      if(nfd!=0) /* did we process all inputs ? */
      {
	GsNotifyIk( TOLOG, IK_SEVERE,
		    "GsLoop :select for saw %d unexpected things",nfd); 
	GsTerminate(EXIT_FAILURE); 
      }
    } /* if (FDISSET data) */
    
  standardInput: /* piped from Ik input task in CDAS config */

    // TO BE DONE : #ifdef AVIRERPOURUNTHREADSEPARE
    if(FD_ISSET(0,&crfds)) 
    {
      nfd--;
      FD_CLR(0,&crfds);
      GsIkStdInput();
    }

    /* TO BE MOVED ELSEWHERE */
    GsIkCheck(ServerName); // check for Ik messages or try to reconnect to Ik;
    
    GsEndOfLoop();
  } /* FOREVER */
} 

  
/*--------------------------------------------------------------------------------*/


