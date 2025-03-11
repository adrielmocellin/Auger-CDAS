#ifndef __PmConnection__
#define __PmConnection__

#include "PmServices.h"
#include "PmLSX.h"

/*
 * Structure passed to the thread during download process
 */
#if USING_THREAD
#include <pthread.h>

struct structthread {
		char data[__MAXBYTESTOWRITETOBSU__];
		int length_data;
		int socket;
		int bsId;
};
#endif

/*
 * Pm generic connection object
 */
class PmConnection
{
protected:
  virtual cdasErrors _CheckTrailerFromSocket (int &);
  virtual PmStdHeader *_ReadPmStdHeaderFromSocket (int &);

  PmServices *_pmService[__MAXSERVICESBYCONNECTION__];
  int _whichService;

  // Pm object message properties
  int _name;
  int _socket;
  int _version;
  int _idService;
  int _onRequest; // ???
  int _requested; // ???
  int _samplingClient; // ?????

public:
  PmConnection ();
  virtual ~ PmConnection () {}

  virtual void RemoveClient () = 0;
  virtual cdasErrors GetInput () = 0;
  virtual void GetInput (IkMessage *) {}
  virtual void Process () = 0;
  virtual cdasErrors SendOutput () = 0;
  virtual void DiscardMess (int &, int &);
  virtual longWord GetSocket () = 0;
		
  // VT - Packet handler function prototypes
  void InitialisePacketHandler ();

  friend unsigned int CRC_32bit (gs_byte*, word);
  friend int SendBSUPacket (const int, const char, const gs_byte *, const int, const int);

  // VT - Packet handler variables
  struct PacketStructure BSUPacket;
};


/*
 * BSU connection object
 */
class PmBSUConnection:public PmConnection
{
private:
  // BSU properties
  longWord _id;

  EnumPmBoolean _uploadRunning;
  time_t _current_time, _last_time;
  longWord _count_radio;

  PmLSBLOCKSServices *_lsBlocksServiceP;   // LS -> CS
  PmBROADCASTServices *_broadcastServiceP; // CS -> all LS
  //PmBSCONFIGServices *_bsconfigServiceP;
  //PmBSLOSTServices *_bslostServiceP;
	
  int _ProcessLsFrame (LsFrameP Data);
  cdasErrors _SendData ();
  void TrashBadLsBlock (char *);

public:
  PmBSUConnection () {}
  PmBSUConnection (PmStdConnection &, int);
  ~PmBSUConnection () {}

  void RemoveClient ();
  cdasErrors GetInput ();
  cdasErrors GetInputTest ();
  void GetInput (IkMessage *, char *, int);
  void Process ();
  cdasErrors SendOutput ();

  longWord GetBsId () { return _id; }
  longWord GetSocket () { return _socket; }

  // uLSx Routines
  cdasErrors HandleBSUPacket (const int, const unsigned char *);
  int BSUPacketInterpreter (struct PacketStructure *);
};


/*
 * CDAS connection object
 */
class PmCDASConnection:public PmConnection
{
private:
  cdasErrors _SendData (int, int);

public:
  PmCDASConnection () {}
  PmCDASConnection (PmStdConnection &, int);
  ~PmCDASConnection () {}

  cdasErrors GetInput () { return CDAS_SUCCESS; }
  void GetInput (LsMessReference &) {}
  void Process () {}

  cdasErrors SendOutput ();
  void RemoveClient ();

  longWord GetSocket () { return _socket; }
};

/*
 * DOWNLOAD connection object
 */
class PmDOWNLOADConnection:public PmConnection
{

private:
  cdasErrors _SendData ();
  cdasErrors _LinkLsWithBSuAndSocket (char *);

  PmDOWNLOADServices *_downloadServicesP;
  vector < int >_sockets;
  time_t _current_time, _last_time;

#if USING_THREAD
  pthread_t v_threadID[__MAXBSUONLINE__];
  struct structthread sthread;
#endif

public:
  PmDOWNLOADConnection () {}
  PmDOWNLOADConnection (PmStdConnection &, int);
  ~PmDOWNLOADConnection ();

  void RemoveClient ();
  cdasErrors GetInput ();
  void Process () {}
  cdasErrors SendOutput ();

  longWord GetSocket () { return _socket; } // RR - filling member function
};

#endif
