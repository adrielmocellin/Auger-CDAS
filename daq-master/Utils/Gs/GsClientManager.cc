/* ****************************************************************** */
/* GsClientManager.cc :                                                */
/*                                                                    */
/*     A ClientManager is launched for each incoming connection       */
/*                                                                    */
/* ****************************************************************** */

/*! 
 The Gs Engine launch a Light Weight Process (GsClientManager) for each 
 input/output path
 it is managing. Thus, a faulty client connection will block the corresponding
 thread only. Ik inputs and old "EndOfLoop" features are implemented the same 
 way to allow GsLoop to sleep waiting for "events" (condition), relinquish
 ressources usage.
 The GsClientManager splits itself into two separate threads, one for reading,
 one for writing.
 The reading thread is sleeping and awaked by "select", and awakes GsLoop 
 via the GsDataWaiting condition.
 The writing thread watch for the client queue and is awaked via the 
 GsClientOutputWaiting condition.
*/

#include "GsThreads.h"
#include "GsServer.h"
#include "GsClients.hxx"
#include "Gsp.h"
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>

#define FOREVER		for(;;) 

// TO BE CHECKED : just to have a reliable access to memory when 1 manager exit
GsClient MyClient[MAXDC];

/*---------------------------------------------------------------------------*/
/* exit handlers */
/*static*/ void closeFile(void * fd)
{
  int ret=(long int)fd;
#ifndef MENATWORK
fprintf(stderr,"closing socket fd = %d\n",ret);fflush(stderr);
#endif
  close(ret);
}

/*static*/ void emptyOutputQueue(void * voidClientP)
{
  GsClientOutMessP buf;
  GsClientP client;

#ifndef MENATWORK
  fprintf(stderr,"emptying queue\n");fflush(stderr);
#endif
  client = (GsClientP) voidClientP;
  GsClientOutputLock(client);
  buf = (volatile GsClientOutMessP) GsClientOutputQueue(client);
  GsClientOutputUnlock(client);
    
    while (buf)
    {
      GsClientOutputLock(client);
      // got a message, discard it from the queue and relinquish the queue
      GsClientSetFirstOutputMessage(client,GsClientNextMessage(buf));
      GsClientOutputUnlock(client);
      
      GsGetAccessToServer();
      
      Service[GsClientMessageService(buf)]
	->discardMessage(client,
			 CDAS_SUCCESS,
			 buf
			 );
      
      GsReleaseAccessToServer();
      free(buf);
      GsClientOutputLock(client);
      buf = (volatile GsClientOutMessP) GsClientOutputQueue(client);
      GsClientOutputUnlock(client);      
    }
}

/*static*/ void removeClient(void * voidClientP)
{
  /* forget this client */
  GsClientP client;
#ifndef MENATWORK
  fprintf(stderr,"removing Client \n");fflush(stderr);
#endif

  client = (GsClientP)voidClientP;

  GsClientOutputLock(client);
  GsNotifyIk(TOLOG,IK_INFO,
	    "removing data client %d (service %d)",
	     GsClientFd(client),
	     GsClientIdService(client)
	     );

  GsGetAccessToServer();
  if (Service[GsClientIdService(client)])
    Service[GsClientIdService(client)]
                  ->removeClient(client,NULL);

  GsReleaseAccessToServer();
  GsClientOutputUnlock(client); 
}

/*static*/ void stopManager(void * manager)
{
#ifndef MENATWORK
 fprintf(stderr,"stopping manager %p\n",manager);fflush(stderr);
#endif

  GsThreadDestroy((GsThread)manager);
}
/*--------------------------------------------------------------------------------*/
int exactRead(unsigned int fd, 
	      char ** buff)
{
 /* fd is the file descriptor we are working with */
 /* buff holds the address of the buffer that will be allocated by */
 /* exactread.*/
 /* Calling releaseInputBuffer with the same params will free the ressources*/
 /* allocated by exactread */
  int n;
  char * tmpP;
  unsigned int size,blockSize;

  if (!buff)
  {
    GsNotifyIk(TOLOG,IKSEVERE,
	       "BUG:exactRead called with a NULL buffer for %d!",fd);
#ifndef MENATWORK
    GsThreadExit(EXIT_FAILURE);
#else
    return(EXIT_FAILURE);
#endif
  }
  
  /* get number of char of the block */
  size=sizeof(longWord);
  tmpP = (char*)&blockSize;
  while(size>0)
  { 
    GsThreadContinue();
    n = read(fd,(char*)&blockSize,sizeof(longWord)); 
    GsThreadContinue();
    switch(n) 
    {
    case -1 :
      GsNotifyIk(TOLOG,IKSEVERE,
		 "exactRead() size,error : %s on port %d ",strerror(errno),fd);
#ifndef MENATWORK
    GsThreadExit(CDAS_READ_ERROR);
#else
    return(CDAS_READ_ERROR);
#endif
      break;
    case 0 :
      GsNotifyIk (TOLOG,IK_INFO,
		  " unexpected end of file on %d",fd);
#ifndef MENATWORK
    GsThreadExit(CDAS_CONNECT_CLOSED);
#else
    return(CDAS_CONNECT_CLOSED);
#endif
      break;
    default :
      tmpP+=n;
      size-=n;
      break;
    } /* switch(n) */
  }/* while (size) */

    /* get number of bytes to read */
  size = ntohl(blockSize);

  /* allocate space enough for the message */
  if ( (tmpP = (char*)malloc(size)) == NULL )
  {
    GsNotifyIk(TOLOG,IK_FATAL,
	       "insufficient space for %u chars from %d! ",size,fd);
#ifndef MENATWORK
    GsThreadExit(EXIT_FAILURE);
#else
    return(EXIT_FAILURE);
#endif
  }

  /* keep the Gs header, so copy the length */
  memcpy(tmpP,(char*)&blockSize,sizeof(longWord));
  /* return a pointer to the header */
  *buff = tmpP;

/* Now read the remaining of the block */
  tmpP+=sizeof(longWord);
  size-=sizeof(longWord);
  while (size>0)
  {
    GsThreadContinue();
    n = read(fd,tmpP,size);
    GsThreadContinue();
    switch(n) 
    {
    case -1 :
      /* TO BE CHANGED :errno ? */
      GsNotifyIk(TOLOG,IKSEVERE,
		 "exactRead() error : %s on port %d ",strerror(errno), fd);
#ifndef MENATWORK
    GsThreadExit(CDAS_READ_ERROR);
#else
    return(CDAS_READ_ERROR);
#endif
      break;
    case 0 :
      GsNotifyIk (TOLOG,IK_INFO,
		  " exactRead:unexpected end of file on %d",fd);
#ifndef MENATWORK
    GsThreadExit(CDAS_CONNECT_CLOSED);
#else
    return(CDAS_CONNECT_CLOSED);
#endif
      break;
    default :
      tmpP+=n;
      size-=n;
      break;
    } /* switch n (block)*/
  } /* while(size>0) block */

  return(CDAS_SUCCESS);
}
void releaseInputBuffer(unsigned int fd, 
			char ** buff)
{
 free(*buff);
}
/*--------------------------------------------------------------------------------*/

static int exactWrite(unsigned int fd, GsStdHeader *  buff)
{
  int
    n,size;
  char * tmpP;

  size = ntohl(buff->length); /* output header */
  tmpP = (char *) (buff);
  while(size>0)
  {
    GsThreadContinue();
    n=write(fd,tmpP,size);
    GsThreadContinue();
    switch(n) {
    case -1 :
      GsNotifyIk(TOLOG,IKSEVERE,
		 "exactWrite() error : %s on %d ",strerror(errno),fd);
      // TO BE CHANGED : in case of timeout, should not exit
      GsThreadExit(CDAS_WRITE_ERROR);
    case 0 :
      GsNotifyIk(TOLOG,IKSEVERE,
		   "exactWrite() : unexpected eof (output) on %d",fd);
      GsThreadExit(CDAS_CONNECT_CLOSED);
default :
      tmpP+=n;
      size-=n;
      break;
    }/* switch n */
  } /* while size > 0 */
  
  return(CDAS_SUCCESS);
}


/*--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------*/
/* GsSendACK */
/* TO BE CHANGED TO ALLOW VARIABLE SIZE STATUS (redirection) */
static /*inline*/ int GsSendAck(unsigned int fd , int errorCode)
{
  GsAcknowledgeMessage mess;
 
  mess.header.length = htonl(GsACKMESSAGELENGTH);
  mess.header.protocolVersion = htonl(GsVERSION);
  mess.header.type = htonl(GsACK);
  mess.header.hLength = htonl(GsSTDHEADERLTH);
  mess.version = htonl(1); /* to be changed ? */
  mess.errorCode = htonl(errorCode);
  mess.trailer.id = htonl(GsMESSAGESENDMARKER);
  return(exactWrite(fd,&(mess.header)));
}


/*---------------------------------------------------------------------------*/
void * GsClientOutputManager(void *data) // output thread
{
  GsClientP client;
  unsigned int fd;
  GsClientOutMessP buf;

  int length;

  GsStdHeader * outrec;

  if (!data) return(NULL);
  
  GsThreadInit();

  client = (GsClientP)data;
  fd = GsClientFd(client);
  // in case we exit, stop the clientManager too
  GsThreadAddExitHandler(stopManager,(void*)GsClientClientManager(client));
  // empty the output queue on exit
  GsThreadAddExitHandler(emptyOutputQueue,(void*)(client));

  FOREVER
  {
    // test the queue
    GsClientOutputLock(client);
    // TO BE CHANGED ? : no warning if cond is true with queue empty !
    while (!(buf = (volatile GsClientOutMessP) GsClientOutputQueue(client)))
    {
      GsClientWaitForSomeOutput(client);
    }

    // got a message, discard it from the queue and relinquish the queue
    GsClientSetFirstOutputMessage(client,GsClientNextMessage(buf));
    GsClientOutputUnlock(client);

    switch (GsClientVersion(client))
    {
    case 1:
      length = GsSTDHEADERLTHv1 + 
	       GsTRAILERLTHv1 + 
	       GsClientMessageLength(buf);
      // padding 
      if(length & 0x1) length++ ;
      if(length & 0x2) length += 2;
      if (!(outrec=(GsStdHeader*)malloc(length)))
      {
	GsNotifyIk(TOLOG,IKSEVERE,
		   "malloc failed on output for %d, size : %d",
		   fd,length);
	GsThreadExit(CDAS_ERROR);
      }
      outrec->length = htonl(length);
      outrec->type = htonl(GsClientMessageType(buf));
      /* TO BE CHANGED */
      outrec->protocolVersion = htonl(GsClientVersion(client));
      outrec->hLength = htonl(GsSTDHEADERLTHv1);
      memcpy(((char*)outrec)+GsSTDHEADERLTHv1,
		     GsClientMessageData(buf), GsClientMessageLength(buf));
      /* TO BE CHANGED */
      *(longWord*)(((char*)outrec)
			   +length
			   -sizeof(GsTrailerv1)) 
	               = htonl(GsMESSAGESENDMARKERv1);
      break;
    default: /* bad client, reports and delete it */
      GsNotifyIk(TOLOG,IKSEVERE,
		 "cannot manage client %d, Gs Version %d", 
		 fd,GsClientVersion(client) );
      GsThreadExit(CDAS_INVALID);
      break;
    } /* switch version */
    exactWrite(fd,outrec);
    free(outrec);
    // tell the server we are done
    GsGetAccessToServer();

    Service[GsClientMessageService(buf)]
      ->discardMessage(client,
		       CDAS_SUCCESS,
		       buf
		       );

    GsReleaseAccessToServer();
    // TO BE CHECKED (who free it ?)    
    free(buf);
  }/* FOREVER */

  GsThreadRemoveExitHandler(emptyOutputQueue,(client));
  GsThreadRemoveExitHandler(stopManager,GsClientClientManager(client));


}

/*--------------------------------------------------------------------------*/

void * GsClientManager(void *data) // main manager and read
{
  // data holds the file descriptor we have to watched for


  GsClientP client;

  int
    nfd, // TO BE CHANGED ? only 1 fd to watch for now
    fd,
    type;

  int error;
  
  char * ir;

  GsStdHeader header;  /* !!!! will be used to store data in internal format*/
  GsStdHeader * inputHeader;


  fd_set
    crfds;
 
  struct timeval tv;

  GsThread clientOutputManager;

  // "standard" GsThread init 
  // e.g., mask signals trapped by the "main" thread
  GsThreadInit();

  /* get fd we are watching for */
  fd = (long int) data;


  // At this point, we have to close fd before exiting)
  GsThreadAddExitHandler(closeFile,data);

  /* get block */
  /* TO BE CHECKED : exactread always return success)*/
  if((error=exactRead(fd,&ir))!=CDAS_SUCCESS) GsThreadExit(error);

  if (fd > MAXDC )
  {
    GsNotifyIk(TOLOG,IKSEVERE,
	       "tooMuchClients(%d)",
	       fd);
    GsSendAck(fd, CDAS_TOO_MUCH_CLIENTS);
    GsThreadExit(CDAS_TOO_MUCH_CLIENTS);
  } /* not enough ressources */


  client = &MyClient[fd];
  GsClientCreate(client);
  GsClientSetFd(client,fd);
  GsClientSetManager(client,GsThreadWAI());

  inputHeader = (GsStdHeader*)ir;
  header.length = ntohl(inputHeader->length);
  header.type = ntohl(inputHeader->type);
  header.protocolVersion = 
    ntohl(inputHeader->protocolVersion);
  header.hLength = ntohl(inputHeader->hLength);
  
  // (roughly) check protocol
  switch(header.protocolVersion)
  {
  case 0 :
  case 1 :
    error = 
      ntohl(*(longWord*)(((char*)inputHeader)
			 + header.length-sizeof(GsTrailerv1)))
      !=GsMESSAGESENDMARKER;
    break;
  default:
    error = -10;
    break;
  }

  if(error)
  {
    GsNotifyIk(TOLOG,IK_WARNING,
	       "badDataClient(%d), error %d",
	       fd, error);
    /* Bad Data Client : don't try to talk to it */
    GsThreadExit(error);
  } /* if (error on Protocol) */

  // Register
  if (Service[header.type]==NULL)
  {
    GsSendAck(fd,CDAS_NO_SUCH_SERVICE);
    GsNotifyIk(TOLOG,IK_WARNING,
	       " Unknown service : %d, requested ",header.type);
    GsThreadExit(CDAS_NO_SUCH_SERVICE);
  }
  
  // Ok, we can try.
  GsClientIdService(client) = header.type;
  GsClientVersion(client) = header.protocolVersion;
  
  if ((error = Service[header.type]->process(client,inputHeader))
      != CDAS_SUCCESS ) 
  {
    GsSendAck(fd,error);
    GsNotifyIk(TOLOG,IK_INFO,
	       "bad news : process returned %d",error);
    GsThreadExit(error);
  } /* if process(message) returned error */
  /* now, we are done with input data, so release ressources 
     allocated by exactread*/
  releaseInputBuffer(fd,&ir);
  // here we go
  // Now, client is registered, so we need to discard it before exiting
  GsThreadAddExitHandler(removeClient,(void *) client);

  /* accept request */
  if ((error = GsSendAck(fd,CDAS_SUCCESS)) != CDAS_SUCCESS) 
  {
    /* very short connection ! */
    GsThreadExit(error);
  }

  /* start the writing thread */
    GsThreadLaunch (&clientOutputManager,
		  GsClientOutputManager,
		  (void*)client);
  // now, we have to kill the writing thread before exiting
  GsThreadAddExitHandler(stopManager,(void*)clientOutputManager);

  /* report connection */
  GsNotifyIk(TOLOG, IK_INFO,
	     "addNew (%d) at %d",
	     header.type,fd);
  /* now watch for input */


  FOREVER
    {
      FD_SET (fd, &crfds);
      /* TO BE CHECKED : don't sleep forever in order to exit properly */
      tv.tv_sec=2;
      tv.tv_usec=700;  /* how long should we wait for */

      GsThreadContinue();
      nfd=select(fd+1,&crfds,NULL,NULL,&tv); /* somebody here ? */
      GsThreadContinue();

      switch(nfd) 
      {
      case -1 : /* select didn't work : that's bad ! */
	/* TO BE CHANGED : errno is not thread compatible */
	GsNotifyIk(TOLOG,IK_FATAL,
		   "select() error : %s on fd %d ",strerror(errno), fd);
	GsThreadExit(EXIT_FAILURE);
	break;
      case 0 : /* nobody's talking : so what ? */
#ifdef VERBOSE
	GsNotifyIk(TOLOG,IK_INFO,
		   " nobody's here ");
#endif
	break;
      default :
#ifdef VERBOSE
	GsNotifyIk(TOLOG,IK_INFO,
		   " something received from %d fd(s)",nfd);
#endif
	break;
      } /* switch nfd */
      
      if(FD_ISSET(fd,&crfds)) 
      {
	nfd--;
	/* TO BE CHECKED : exactread always return success)*/
	if((error=exactRead(fd,&ir))!=CDAS_SUCCESS) 
	 {
	  GsThreadExit(error);
	 }

	// TO BE DONE ? : check protocol

	type = ntohl(((GsStdHeader*)ir)->type);

	// TO BE DONE : check if type is ok; how ?

	// is there a service for this type of message ?
	if (!Service[type])
	{
	  GsNotifyIk(TOLOG,IKSEVERE,
		     " client %d sent message of unknown type %d",fd,type);
	  GsThreadExit(CDAS_NO_SUCH_SERVICE);
	}

	// protect against thread "unaware" server routines

	GsGetAccessToServer();
	// and process, OUF !
	Service[type]
            ->process(client, ((GsStdHeader*)ir));
	GsReleaseAccessToServer();
	/* now, we are done with input data, so release ressources 
	   allocated by exactread */
	releaseInputBuffer(fd,&ir);
      } /* if FDISSET ( clients) */
      else /* is it still alived ? */
      { 
#ifdef TOBECHECKED
	if (read(fd,0)!=0) closeClientManager(GsReadError,i);
#endif
      } /* else FD-ISSET   */
      
      if(nfd!=0) /* did we process all inputs ? */
      {
	GsNotifyIk( TOLOG, IKSEVERE,
		    "select for %d saw something else than its own fd",fd); 
	GsThreadExit(EXIT_FAILURE); 
      }
    } /* FOREVER */
  GsThreadRemoveExitHandler(stopManager,clientOutputManager);
  GsThreadRemoveExitHandler(removeClient,client);
  GsThreadRemoveExitHandler(closeFile,fd);
  
  return ((void*)NULL); // just to be consistent with declaration
}

  



