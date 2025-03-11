#ifndef _UIMESSAGE_H
#define _UIMESSAGE_H 1

#include <IkC.h>

/* typedef ELError IkMessage;
*/

typedef enum {
 FIRSTUIMSGCODE = LASTIKMSGCODE,    /* this one always needs to be first */
 RCCONTROLALL,
 RCCONTROLDONE,
 RCRELEASEOUT,
 LSWAKEUP,
 LSREADY,
 LSACK,
 LSRESET,
 LSREBOOT,
 INITIALIZE,
 INITIALIZED,
 LSSTART,
 LSSTOP,
 CONFIGREADY,
 CONFIGDONE,
 CDASSTART,
 BADSTATION,
 RCSTART,
 RCSTOP,
 RCPAUSE,
 RCCONTINUE,
 FDSTART,
 FDSTOP,
 T3,
 SENDCONFIG,
 GETMODATA,
 GETMODATAREPLY,
 LASTUIMSGCODE            /* this one always needs to be last */
}IkCode;

/* inter-tasks messages mnemonics */
/* XXX : int; type of message, defined in IkCodes */
/* XXXSTR : string; first word of data part */

/* Rc messages and replies */
#define RCCONTROLALLSTR "RC_CONTROL_ALL"
#define RCCONTROLDONESTR "RC_CONTROL_DONE"
#define RCRELEASEOUTSTR "RC_RELEASE_OUT"
#define LSWAKEUPSTR "LS_WAKEUP"
#define LSREADYSTR "LS_READY"
#define LSACKSTR "LS_ACK"
#define LSRESETSTR "LS_RESET"
#define LSREBOOTSTR "LS_REBOOT"
#define INITIALIZESTR "INITIALIZE"
#define INITIALIZEDSTR "INITIALIZED"
#define LSSTARTSTR "LS_START"
#define LSSTOPSTR "LS_STOP"
#define CONFIGREADYSTR "CONFIG_READY"
#define CONFIGDONESTR "CONFIG_DONE"
#define CDASSTARTSTR "RUN_START"
#define BADSTATIONSTR "BAD_STATIONS_LIST"
#define RCSTARTSTR "START"
#define RCSTOPSTR "STOP"
#define RCPAUSESTR "PAUSE"
#define RCCONTINUESTR "CONTINUE"
#define FDSTARTSTR "FDSTART"  
#define FDSTOPSTR "FDSTOP"
#define SENDCONFIGSTR "SEND_CONFIG"
#define GETMODATASTR "GET_MO_DATA"
#define GETMODATAREPLYSTR "GET_MO_DATA_REPLY"

#define SUPING IKSUPING
#define SUPINGSTR "PING"
#define SUPONG IKSUPONG
#define SUPONGSTR "PONG"
#define SUKILL 666
#define SUKILLSTR "STOP"

/* "addressing/list" modes */
/* second word of data part if needed */

/* LIST : simple list of integer values */
#define LIST "LIST"
/* FLIST : simple list of floating point values */
#define FLIST "FLIST"
/* SINGLE LIST : same data for all dest in the list */
/* format : SLIST #bytes message(1 integer / byte) ad1(integer) ... */
#define SLIST "SLIST"
/* MULTI LIST : a block of data for all address in the list, plus a */
/* fixed length block for each dest */
/* format : MLIST #bytes(gen) message #bytes(spec) ad1 mess1 ad2 ... */
#define MLIST "MLIST"
#ifndef ALL
/* ALL : no comment ; already defined in IkC*/
#define ALL "ALL"
#endif
/* BROADCAST : no comment */
#define BROADCAST "BROADCAST"
/* FORCED SINGLE LIST : same as slist but with special features*/
/* format : SFORCED #bytes message(1 integer / byte) ad1(integer) ... */
#define SFORCED "SFORCED"
/* FORCED  LIST : same as list but with special features*/
/* format : FORCED ad1(integer) ... */
#define FORCED "FORCED"


/* Application identifications */
#define IKMAXSUFFIX 10 /* max length of suffix string */
#define TOPM "Pm"
#define FROMPM "Pm"
#define TORC "Rc"
#define FROMRC "Rc"
#define TOALL "All"
#define TOEB "Eb"
#define TOCT "Ct"
#define PMPORT 10410

#define TOLOG "Log" /* just for logging puposes by Ik itself */
#define FROMSU "Su"
#define TOSU "Su"

typedef enum 
{
  TYPEBROADCAST,   /* same data to all local stations */
  TYPEALL,         /* different data to all local stations */
  TYPELIST,
  TYPESLIST,
  TYPEFLIST,
  TYPEMLIST,
  TYPEUNKNOWN
} UiIkAddressingModes;

typedef struct
{
  int mode;
  int type;
  int number;
  int commonPartSize;
  int specificPartSize;
  int inputStringSize;
  int *intList;
  int *commonPart;
  int *specificPart;
  char *inputString;
}UiIkListInfo, *UiIkListInfoP;

typedef int (UiIkMessageHandler)(char *from, char *to, int msgType, 
				 char *text);
typedef int (*UiIkMessageHandlerP)(char *from, char *to, int msgType,
				   char *text);

UiIkListInfoP UiDecodeIkData(char *data);


#endif

