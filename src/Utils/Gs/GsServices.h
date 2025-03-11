/*--------------------------------------------------------------------------------*/
/*
  GsServices.h

  ........... describe structures (objects) to handle each type of message

Basically, a GsService consists of :

         - (pointers to) functions to maintain a list of related Clients
	 - a (pointer to) a function to decode & process input
	 - a (pointer to) a function to build output messages (included list of destinations)
	 - a pointer to a function to be called once message have been sent to client


The available GsServices are referenced through the array "Service"

The array "Service" is initialized by ServicesInit() which has to be called 
prior using service concept; This function is specific to yhe server and has 
to be maintained together with new services implementation !!!! That's why the
array and the function are declared here as extern, and not defined in any way



*/
/*--------------------------------------------------------------------------------*/

#ifndef __GSSERVICES__
#define __GSSERVICES__

#include "Gs.h"
#include "GsClients.hxx"   /* get access to clients structures def. */


/* register a Client ; */
   /* receive a pointer to the Client and a pointer to the message */ 
   /* and a pointer to the parameter block; */ 
typedef int (addClientF)(GsClientP,GsStdHeader*,void*);
typedef int (*addClientFP)(GsClientP,GsStdHeader*,void*);

/* remove a Client ; */
   /* receive a pointer to the Client and a pointer to the message */
typedef int (removeClientF)(GsClientP,GsStdHeader*);
typedef int (*removeClientFP)(GsClientP,GsStdHeader*);

/* process a message */
   /* receive a pointer to the Client and a pointer to the message */
typedef int (processInputF)(GsClientP,GsStdHeader*);
typedef int (*processInputFP)(GsClientP,GsStdHeader*);

/* build some messages to clients */
   /* receive some data, and a type describing the parameter set */
typedef int (buildOutputF)(void *,int);
typedef int (*buildOutputFP)(void *,int);

/* take appropriate action once message was sent */
typedef int (discardMessageF)(GsClientP,int,GsClientOutMessP);
typedef int (*discardMessageFP)(GsClientP,int,GsClientOutMessP);

typedef struct
{
  addClientFP  addNewClient;
  removeClientFP removeClient;
  processInputFP process;
  buildOutputFP buildOutput;
  discardMessageFP discardMessage;
} GsService, *GsServiceP;


/******************************/

/* Global references */
extern GsServiceP Service[]; /* hold addresses of services structures. */
extern int ServicesInit(void);
extern int GsInternalServicesInit();




#endif






