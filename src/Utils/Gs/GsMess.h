/*-----------------------------------------------------------------------------*/
/*
  GsMess.h
  ...............describes the interface to messages
*/
/*-----------------------------------------------------------------------------*/


#ifndef __GSMESS__
#define __GSMESS__

#include "GsProtocol.h"


/* format of the messages including service information. i put the numServices
   in the union because if this value is 0, then there is nothing in the message
   after it to union with the block. */
typedef struct gsmessage {
  GsStdHeader header;
  longWord formatVersion;
  union {
    longWord numServices;
    int block; /* Alain : changed from void* to avoid misinterpretation i! */
  } msg;
} GSMESSAGE;

/* structure within the msg block for each service */
typedef struct serviceParams {
  longWord serviceId;
  longWord paramBytes;
} SERVICEPARAMS;

/* supported parameter types */
typedef enum paramtype {
  GSLINT = 0,            /* unsigned long integer */
  GSSINT,
  GSDBL
} PARAMTYPE;

#define SERVICEPARAMSLEN sizeof(SERVICEPARAMS)
#define NUMSERVICESLEN sizeof(longWord)

longWord GsMessNumServ(GsStdHeader *header);
longWord GsMessServInfo(GsStdHeader *header, longWord numServ, longWord **params);
void *GsMessParam(void *parameters, PARAMTYPE paramType, void **nextParam);
int GsMessParamSet(void *param, void *parameters, PARAMTYPE paramType, 
		   void **nextParam);



#endif
