#ifndef __PmDefs__
#define __PmDefs__

#include "PmLib.h"
#include "PmProtocol.h"

/*-----------------------------------------------------------------------*/
/* T2 Structures                                                         */
/*-----------------------------------------------------------------------*/
typedef struct t2lsstruct
#if  __BYTE_ORDER == __BIG_ENDIAN
{
  unsigned char energie:4;
  unsigned char timeHigh:4;
  unsigned char timeMid;
  unsigned char timeLow;
}
T2Ls, *T2LsP;
#else
{
  unsigned char timeHigh:4;
  unsigned char energie:4;
  unsigned char timeMid;
  unsigned char timeLow;
}
T2Ls, *T2LsP;
#endif

typedef struct t2lsBuff
{
  longWord second;
  longWord availableSize;
  T2LsP data;
  int number;
  int status;			/* see codes above */
  unsigned int id;
}
T2LsBuffer, *T2LsBufferP;
#define NULLT2LSBUFFER {0,0,NULL,0,0,0}

typedef struct localStation
{
  T2LsBuffer t2buffer[6];
  int status;			/* see codes above */
  unsigned int id;
}
Ls, *LsP;


/*-----------------------------------------------------------------------*/
/* LSMESS object structure                                               */
/*-----------------------------------------------------------------------*/
class LsMessReference
{
public:
  LsMessReference ();
  LsMessReference (int &, int, const LsRawMessage &);
  LsMessReference (const LsMessReference &);
  ~LsMessReference () {}
  LsMessReference & operator = (const LsMessReference &);

  /* donnees pour gerer le message */
  int lsId;
  int bsId;
  unsigned int type;
  unsigned int version;
  unsigned int length;
  unsigned short slice;
  unsigned short messNumber;
  unsigned char completion;
  /* data */
  char *rawData;
};

/*-----------------------------------------------------------------------*/
/* UPLOAD.object structure                                               */
/*-----------------------------------------------------------------------*/
/* LS receiving message managment                                        */
class PmUPLOADMessage
{
private:
  vector < int >status;
  EnumPmBoolean _status;

public:
  PmUPLOADMessage () { currentTime = lastTime = 0; }
  PmUPLOADMessage (const PmUPLOADMessage &);
  PmUPLOADMessage & operator = (const PmUPLOADMessage &);
  ~PmUPLOADMessage () {}

  void SetNewUpload (IkMessage *, int, int);
  void RemoveLS (int);
  void SetStatus (int);
  int DiscardMissingStations ();
  void NextPart ();
  int FindLSByLsId (int);
  int ReadyForNextPart ();
  EnumPmBoolean GetStatus ();

  time_t currentTime, lastTime;
  vector < int >ls;
  vector < int >inTransfer;	// because of mode BROADCAST
  char filename[__MAXFILENAME__];
  char mode[10];
  longWord initPos;
  longWord length;
};

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* DOWNLOAD.object structure                                             */
/*-----------------------------------------------------------------------*/
/* LS sending message managment                                          */
typedef enum
{
  pEVTSVR,
  pCONTROL,
  pMONIT,
  pGPS,
  pLAST
}EnumPriority;

class PmDownloadMessage 
{
 public:
  longWord destNumber;
  vector<int> addresses;
  longWord dataLength;
  char* data;

  PmDownloadMessage () {}
  PmDownloadMessage (const PmDownloadMessage& copy) 
		: destNumber(copy.destNumber), dataLength(copy.dataLength), data(copy.data) // RR - Initialization
    {
      for (unsigned int i=0; i<copy.addresses.size(); i++) addresses.push_back (copy.addresses[i]);
    }
  PmDownloadMessage (char* input);
  ~PmDownloadMessage () {}
};

/*-----------------------------------------------------------------------*/
/* Pm Message object structure                                           */
/*-----------------------------------------------------------------------*/
class PmMessReference
{
public:
  PmMessReference () {}
  PmMessReference (IkMessage *, char *, int);
  PmMessReference (const PmUPLOADMessage &);
  PmMessReference (const PmMessReference &);
  ~PmMessReference () {}
  PmMessReference & operator = (const PmMessReference &);

  /* donnees pour gerer le message */
  unsigned char completion;
  unsigned char priority;
  unsigned char slice;
  unsigned char messNumber;
  unsigned char type;
  char *mode;
  unsigned short pointerData;
  unsigned int length;		// = champ length du LSRAWMESSAGE
  vector < int >ls;
  char *rawData;
};

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Gestion of link between local stations and base subscriber unities    */
/*-----------------------------------------------------------------------*/
class BaseSubscriberUnity
{
public:
  BaseSubscriberUnity () {}
  BaseSubscriberUnity (int id, int sock):bsId (id), socket (sock) {}
  ~BaseSubscriberUnity () {}

  int bsId;
  int socket;
  vector < int >list_ls;
};


class LocalStation
{
  //  short _ls_id;
  short _bs_id;
  short _socket;
  unsigned char _requestedByTrash;

public:
  LocalStation ()
  {
    _bs_id = -1;
    _socket = -1;
    _requestedByTrash = 0;
  }
  ~LocalStation () {}

  short GetBsId () { return _bs_id; }
  void SetBsId (short value) { _bs_id = value; }
  short GetSocket () { return _socket; }
  void SetSocket (short value) { _socket = value; }
  unsigned char IsRequestedByTrashService () { return _requestedByTrash; }
  void RequestedByTrashService (int state) { _requestedByTrash = (unsigned char) state; }
};


class BaseSubscriber
{
private:
  BaseSubscriber () {}
  static BaseSubscriber *_instance;
public:
  static BaseSubscriber *Access ();
  BaseSubscriber (const BaseSubscriber &)
  {
    cerr << "dans le constructeur de copie de BaseSubscriver\n";
  }
  ~BaseSubscriber ()
  {
    cerr << "dans le destructeur de BaseSubscriver\n";
  }

  vector < BaseSubscriberUnity > list_BSu;
  LocalStation local_station[__MAXLSID__];

  void AddBSu (int, int);
  void RemoveBSu (int, int);
  int FindBSuFromId (int, int);
  cdasErrors AddLsInBSU (int, int, int);
  void RemoveLsInBSU (int, int, int);
  int LinkLsWithBSu (int, int, int);
  vector < int >GetListOfSockets ();
  int GetSocketOfBsByLsId (int);
  vector < int >GetStationsInBSU (int, int);
  int IsRequestedByTrashService (int);
  void RequestedByTrashService (int, int);
  IkMessage *ReBuildIkMessageWithListOfLsInThisBSu (IkMessage *, int &, int);

};
BaseSubscriber *theStationDataBase ();

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Internal messages                                                     */
/*-----------------------------------------------------------------------*/
class _PmLsMessage
{
public:
  _PmLsMessage () {}
  _PmLsMessage (const LsMessReference &);
  _PmLsMessage (const _PmLsMessage &);
  _PmLsMessage & operator = (const _PmLsMessage &);

  //  longWord lsId;
  longWord length;
  longWord type;
  longWord version;
  char *data;
};

class _PmLsRawMessage
{
public:
  _PmLsRawMessage () {}
  _PmLsRawMessage (const LsMessReference &);
  _PmLsRawMessage (const _PmLsRawMessage &) {}
  _PmLsRawMessage & operator = (const _PmLsRawMessage &) { return *this; }

  longWord lsId;
  _PmLsMessage mess;
};

class PmServicesOutput
{
public:
  PmServicesOutput ();
  PmServicesOutput (int, char *);
  PmServicesOutput (const PmServicesOutput &);
  PmServicesOutput & operator = (const PmServicesOutput &);
  ~PmServicesOutput () {}

  int type;
  int priority;
  int length;
  char *output;
};

/*-----------------------------------------------------------------------*/

#endif
