/*--------------------------------------------------------------------------------*/
/*
  GsClients.hxx
  ...............describes structures (objects) associated to clients handling.
	   
*/
/*--------------------------------------------------------------------------------*/
#ifndef __GSCLIENTS__
#define __GSCLIENTS__

#include <pthread.h>
#include <malloc.h>

#include "GsThreads.h"
#include "GsNotify.h"



typedef struct clientOut
{
  int priority; /* used by services to insert message in the queue */
  int tag; /* services own data part of messages; used to dispose of them */
  struct clientOut * next;
  struct clientOut * previous;
  longWord type; /* set by service to its id */
  int dataLength;
  char  * data;
} PrivateClientOutMess, * PrivateClientOutMessP;

#define PRIVATENULLCLIENTOUTMESS {0,0,NULL,NULL,0,0,NULL}


  /* group all the thread-related stuff in one struct */
typedef struct 
{
  GsThread clientManager;
  GsLocker outputQueueLocker;
  GsSignal outputQueueSignal;
} GsClientSynchro , * GsClientSynchroP;

#define NULLGSCLIENTSYNCHRO { \
                             0, \
                             GSLOCKERINITIALIZER, \
                             GSSIGNALINITIALIZER, \
                            }

typedef struct clientS
{
  int fd ;
  int version;
  int idService ; /* identification service the client was registered by */
  PrivateClientOutMessP firstOutputMessage;
  int onRequest; 
  int requested; /* in case of OnRequest regulation */
  int samplingClient; /* in case of */
  void * serviceData;
  GsClientSynchro synchro;
} PrivateClient, * PrivateClientP;

#define PRIVATENULLCLIENT {0,0,0,NULL,0,1,0,NULL,NULLGSCLIENTSYNCHRO}


/* messages to be sent to Clients */
typedef PrivateClientOutMess GsClientOutMess;
typedef PrivateClientOutMessP GsClientOutMessP;

#define  GsClientMessagePriority(object) (object)->priority
#define  GsClientMessageType(object) (object)->type
#define  GsClientMessageTag(object) (object)->tag
#define  GsClientMessageService(object) (object)->type
#define  GsClientNextMessage(object) (object)->next
#define  GsClientPreviousMessage(object) (object)->previous
#define  GsClientMessageLength(object)  (object)->dataLength
#define  GsClientMessageData(object)  (object)->data

#define GsClientMessageSetPriority(object,value) ((object)->priority = (value))
#define GsClientMessageSetType(object,value) ((object)->type = (value))
#define GsClientMessageSetTag(object,value) ((object)->tag = (value))
#define GsClientMessageSetService(object,value) ((object)->type = (value))
#define GsClientMessageSetNext(object,value) ((object)->next = (value))
#define GsClientMessageSetPrevious(object,value) ((object)->previous = (value))
#define GsClientMessageSetLength(object,value) ((object)->dataLength = (value))
#define GsClientMessageSetData(object,value) ((object)->data = (value))


/* Client structure*/

typedef PrivateClient GsClient;
typedef PrivateClientP GsClientP;

#define GsClientFd(object)  (object)->fd
#define GsClientIdService(object) (object)->idService
#define GsClientVersion(object) (object)->version
#define GsClientOutputQueue(object) (object)->firstOutputMessage
#define GsClientIsOnRequest(object) (object)->onRequest
#define GsClientBufferCapacity(object) (object)->requested
#define GsClientReadyForOutput(object)  ((object)->requested>0)
#define GsClientIsSampling(object) (object)->samplingClient
#define GsClientServiceData(object) (object)->serviceData
#define GsClientSynchro(object) (object)->synchro
#define GsClientClientManager(object) GsClientSynchro(object).clientManager


/* Services must be able to configure clients : access "functions") */
#define GsClientSetFirstOutputMessage(object,mess) \
                           ((object)->firstOutputMessage = (buf))
#define GsClientSetSampling(object) ((object)->samplingClient = 1)
#define GsClientSetNonSampling(object) ((object)->samplingClient = 0)
#define GsClientSetOnRequest(object) ((object)->onRequest=1)
#define GsClientSetUnregulated(object) ((object)->onRequest=0)
#define GsClientSetBufferCapacity(object,numMess) ((object)->requested = numMess)
#define GsClientSetServiceData(object,pointer) ((object)->serviceData = pointer)
#define GsClientSetManager(object,manager) \
                                 (GsClientSynchro(object).clientManager \
                                 = (manager))
static inline void GsClientSetFd(GsClientP object,unsigned int fd)
{
  object->fd = fd;
}

/* Synchronization */

/* TO BE CHECKED */
// here we have the problem with the use of GetAccess MACROS !
// see GsThreads.h

#ifdef SEPARABLECLEANUPFUNCTIONS
static inline int GsClientOutputTryLock(GsClientP object)
{
  return GsTryAccessTo(&(object->synchro.outputQueueLocker));
}

static inline int GsClientOutputLock(GsClientP object)
{
  return GsGetAccessTo(&(object->synchro.outputQueueLocker));
}

static inline int GsClientOutputUnlock(GsClientP object)
{
  return GsReleaseAccessTo(&(object->synchro.outputQueueLocker));
}
#else // SEPARABLECLEANUPFUNCTIONS
#define GsClientOutputLock( object) \
               GsGetAccessTo(&((object)->synchro.outputQueueLocker)); 

#define GsClientOutputUnlock( object) \
             GsReleaseAccessTo(&(object->synchro.outputQueueLocker)); 

#endif // SEPARABLECLEANUPFUNCTIONS


static inline int GsClientSignalOutput(GsClientP object)
{
  return pthread_cond_signal(&(object->synchro.outputQueueSignal));
}

static inline int GsClientWaitForSomeOutput(GsClientP object)
{
  return GsWaitFor(&(object->synchro.outputQueueSignal),
		   &(object->synchro.outputQueueLocker));
}


/* initialization */
#define NULLCLIENT PRIVATENULLCLIENT

static inline GsClientP GsClientCreate(GsClientP where)
{
  GsClient nullClient = NULLCLIENT;
  GsClientP client;
  if (!where)
    client = (GsClientP) malloc(sizeof(GsClient));
  else
    client = where;
  *client = nullClient;
  
  return (client);
}
static inline int GsClientInsertMessageInQueue(GsClientP client,
					       void * buffer,
					       int messType,
					       int length,
					       int priority,
					       int tag,
					       char * funcName)
{
  GsClientOutMessP buf,tmp,newmess;
  if (client && buffer)
  { 
    if(!(newmess=(GsClientOutMessP)calloc(1,sizeof(GsClientOutMess))))
    {
      GsNotifyIk(TOLOG,IKSEVERE,
		 "%s : no space available for queueing output/n",
		 funcName);
      return(CDAS_MALLOC_ERROR);
    }
    GsClientMessageSetPriority(newmess,priority);
    GsClientMessageSetType(newmess,messType);
    GsClientMessageSetTag(newmess,tag);
    GsClientMessageSetLength(newmess,length);
    GsClientMessageSetData(newmess,(char*)buffer);
    /* We don't know where the call is coming from (mainly a service) 
       But either the server is single threaded , and then there is only 1 service
       accessing the ressource, either the the server is multithreaded and we are 
       allowed to block the service. Dark Corner : ensure that the writing thread 
       never lock this ressources for a long time.
    */

    GsClientOutputLock(client);
    tmp = buf = GsClientOutputQueue(client);
    while ( 
	   (buf)&&
	   (GsClientMessagePriority(buf)>=GsClientMessagePriority(newmess))
	   )
    {
      tmp=buf;
      buf = GsClientNextMessage(buf);
    }
    GsClientMessageSetNext(newmess,buf);
    GsClientMessageSetPrevious(newmess,tmp);
    if (tmp)
    {
      GsClientMessageSetNext(tmp,newmess);
    }
    else
    {
      GsClientSetFirstOutputMessage(client,newmess);
    }
    GsClientOutputUnlock(client);
    GsClientSignalOutput(client);
  } /* if client*/
  return(CDAS_SUCCESS);
}






#endif


