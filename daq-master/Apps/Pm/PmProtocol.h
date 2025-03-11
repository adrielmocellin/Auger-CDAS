#ifndef __PmProtocol__
#define __PmProtocol__

#include <vector>
#include <netinet/in.h>
using namespace std;

#include "GsProtocol.h"
#include "PmConst.h"

// Known Message types
typedef enum
{
  PmBSID = 1          ,  /* BSUs to PM connection ; equal to 1*/
  PmCTID,                /* Ct to Pm connection */
  PmEBID,                /* Eb to Pm connection */
  PmRCID,                /* Rc to Pm connection */
  PmMOID,                /* Mo to Pm connection */
  PmCONSID,              /* Generic client (consumer) connection */
  PmLSBLOCKS,            /* LS messages aggregated by a BSU */
  PmBSLOST,              /* BSUs lost some messages */
  PmBROADCAST,           /* Messages from CS to LSs */
  PmBSCONFIG,            /* Messages from Pm to BSs */
  PmT2,                  /* T2 buffer record */
  PmEVENT,               /* shower data records. */
  PmCALIB,               /* Calib records. */
  PmMoSdRaw,             /* raw monitoring records (LS->)Pm->client */
  PmMoSdSend,            /* raw monitoring config client->Pm(->LS) */
  PmMON,                 /* Monitoring records. */
  PmCTRL,                /* General Notification record from LS to CS */
  PmCOMMAND,             /* send data to LS */
  PmLTRASH,              /* raw data from LS */
  PmBTRASH,              /* raw data from BSU */
  PmDOWNLOADID,          /* get the link to download files to the LSs */
  PmDOWNLOAD,            /* download software to the local stations */
  PmRADIO,               /* write radio data on Raid */
  PmCBID,                /* Cb to Pm connection */
  PmT2Fast,              /* Fast T2 buffer record */
  PmSpmt,                /* raw data from LS for small PMT */
  PmLAST                 /* last service type */
}PmMessageType;

// PmLsMessage structure
typedef struct  
{
  longWord length;
  longWord type;
  longWord version;
  char data [1];
}PmLsMessage, *PmLsMessageP;

// pourquoi ce tiroir supplémentaire ? voir compatibilité avec mo et eb
typedef struct
{
  longWord lsId;
  PmLsMessage mess;
}PmLsRawMessage, *PmLsRawMessageP;

// Pm standard header structure
class PmStdHeader
{
 public:
  PmStdHeader () {}
  ~PmStdHeader () {}
  PmStdHeader ( longWord len, int typeMess ) { // RR - new member
    length = htonl (PmSTDHEADERLENGTH + len + sizeof (longWord));
    protocolVersion = htonl (GsVERSION);
    type = htonl (typeMess);
    hLength = htonl (PmSTDHEADERLENGTH);
  }

  longWord length;
  longWord protocolVersion;
  longWord type;
  longWord hLength;
};

// Pm standard trailer structure
class PmTrailer
{
 public:
  PmTrailer () {}
  ~PmTrailer () {}
  PmTrailer ( longWord trailer ) { id = htonl (trailer); }

  longWord id;
};

// Pm standard data block structure
class PmStdConnection
{
 public:
  PmStdConnection () {}
  ~PmStdConnection () {}

  PmStdHeader pmStdHeader;
  longWord serviceVersion;
  longWord numberOfServices;
  vector<longWord> service;
  longWord numberOfParameterBytes;
  longWord id;
  PmTrailer trailer;
	
};

// Pm message acknowledge structure
class PmMessageAck
{
 public:
  PmMessageAck () {}
  ~PmMessageAck () {}

  PmStdHeader pmStdHeader;
  longWord version;
  longWord errorCode;
  PmTrailer trailer;
};
#define PmACKMESSLENGTH sizeof(PmMessageAck)

#endif
