#ifndef _PmConst_
#define _PmConst_

#define __QUEUELEN__ 32 // Maximum queued connection during listening mode

#define __MAXBYTESPERMESS__ 50000	// fix me
#define __MAXNUMBEROFMESSPERSEC__ 1
#define __MAXBYTESTOWRITETOBSU__ 248	// Max data value supported by LSx
#define __MAXBYTESTOWRITE__ 200000	// WATCH ME !!!!!!!!!!!!!
#define __MAXSERVICESBYCONNECTION__ 5

#define __MAXLENGTHFORUPLOAD__ 1024

#define PMDEF_max_(x1,x2) ((x1) > (x2)) ? (x1) : (x2)

// Filenames & directories
#define __MAXFILENAME__ 256
#define PMLOGDIR "/Raid/var/Pm/"
#define MAXLOGSIZE 3e5 // in bytes

#define PMTRASHDIR "/Raid/var/trash/"
#define PMBADDATA PMTRASHDIR"bad_data"

// Pm message managment macros
#define PmSTDHEADERLENGTH 16
#define PmMESSAGESENDMARKER 0x1ABCDEF2
#define PMLSMESSAGEHEADERLENGTH 12
#define PMLSRAWMESSAGEHEADERLENGTH 4
#define LTHEXCDATABLOCK (6 * sizeof(longWord)) // block length + Gs version + message type + header length + message version + trailer length

// Frame mode destination CS -> LS
#define mLIST "LIST"
#define mANTILIST "ANTILIST"
#define mBROADCAST "BROADCAST"
#define mSLIST "SLIST"

// Maximum size of entities
#define __MAXBSUONLINE__ 60
#define __MAXLSID__ (2048 + 3100) // RR
#define __MAXCDAS__ 20

// max number of messages supported in the same time for 1 local station and 1 stream
#define __MAXINDEX__ 3
#define __MAXSTREAM__ 5

#define __MAXTIMETOGETUPLOAD__ 600 // increased in case there is an event
#define __MINRECEIVEDSLICESTORESET__ 58

typedef enum {
  PmNO = 0,
  PmYES
} EnumPmBoolean;

typedef enum
{
  T2Stream = 0,
  EventStream,
  ControlStream,
  GPSStream,
  CalmonStream
} EnumStreams;

// Debugging variables
#define PMDEBUG 0
#define PMDEBUG_VERBOSE 0

// Thread managment
#define USING_THREAD 0

#endif
