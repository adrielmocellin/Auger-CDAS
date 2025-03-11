/*-----------------------------------------------------------------------------*/
/*
  GsProtocol.h
*/
/*-----------------------------------------------------------------------------*/
#ifndef __GSPROTOCOL__
#define __GSPROTOCOL__

/* *************** Utilities *******************/


/* storage names */
typedef unsigned char gs_byte; /* 8 bits */

#ifndef ALPHA              /* to be checked for generalization */
typedef unsigned short int word; /* 2 bytes */
typedef unsigned int longWord; /* 4 bytes */
typedef signed int sLongWord; /* 4 bytes */
#endif

/*-----------------------------------------------------------------------------*/


/* some trick here */
#include <GsTools.h>

/* ***************Messages definitions *******************/


#define GsVERSION 1

/* fixed header part; will be maintained as stable as possible */
/*                       can be followed by a variable length, */
/*                       type/version specific header part     */

typedef struct
{
  longWord length; /* length of message in bytes, included header and trailer*/
  longWord protocolVersion; /* note the change from version 0 !!! */
  longWord type;   /* type of messages : triggers a specific processing */
  longWord hLength;/* complete header length (bytes)=16+variable header size */
} GsStdHeaderv1;

#define GsSTDHEADERLTHv1 16


/* message trailer */
/*                     for version 0 this is a fixed WORD */
#define GsMESSAGESENDMARKERv1 0x1ABCDEF2
#define GsMESSAGESENDMARKER GsLASTVERSION(GsMESSAGESENDMARKERv,GsVERSION)
typedef struct
{
  longWord id;
} GsTrailerv1;
#define GsTRAILERLTHv1 sizeof(GsTrailerv1)

/* definitions of the last version */
typedef GsLASTVERSION(GsStdHeaderv,GsVERSION) GsStdHeader;
/* that is : typedef GsStdHeaderv1 GsStdHeader; for version 1 */
#define GsSTDHEADERLTH GsSTDHEADERLTHv1
 
/* here that means that GsTrailer is GsTrailerv1 */
typedef GsLASTVERSION(GsTrailerv,GsVERSION) GsTrailer;
#define GsTRAILERLTH sizeof(GsTrailer)

/* ACKnoledge message */
typedef struct
{
  GsStdHeaderv1 header;
  longWord version;
  longWord errorCode;
  GsTrailerv1 trailer;
}GsAcknowledgeMessagev1;
#define GsACKMESSAGELENGTHv1 sizeof(GsAcknowledgeMessagev1)

typedef GsLASTVERSION(GsAcknowledgeMessagev,GsVERSION) GsAcknowledgeMessage;
#define GsACKMESSAGELENGTH sizeof(GsAcknowledgeMessage)



/* Known messages types ;  each type corresponds to a given service */
/*                         and is associated with a header definition */
/* they are defined in XyProtocol.h files where Xy is the name of the actual */
/* server package. here, we just reserve space for known servers */

typedef enum 
{
  GsNull = 0,          /* reserved */
  PmFirstId = 1,     /* first Id handled by Pm */
  PmLastId = 1000,     /* Last Id handled by Pm */
  MoFirstId = 1001,     /* first Id handled by Mo */
  MoLastId = 2000,     /* Last Id handled by Mo */
  FdFirstId = 2001,     /* first Id handled by Fd */
  FdLastId = 3000,     /* Last Id handled by Fd */
  /* Special record types. Examples */
  GsCONF=10000,
  GsBUFFAVAIL,
  GsMESS,
  GsLOG,
  GsOOB,
  GsACK = 100000 /* ACK/NACK messages */
}GsMessageType;


#include <UiErr.h>

/*-----------------------------------------------------------------------------*/
#endif










