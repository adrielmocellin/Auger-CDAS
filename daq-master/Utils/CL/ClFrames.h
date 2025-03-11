/*--------------------------------------------------------------------------------*/
/*
  ClFrames.h
  ........... data structures describing frames.
*/
/*--------------------------------------------------------------------------------*/

#ifndef __CLFRAMES__
#define __CLFRAMES__


/*********************************/
/* description of LS->CS frames */
#define MAXLSFRAMELENGTH 150

/* Warning : to avoid beeing confusing, we added frame size field in
   the header description; to be changed ? */
#define LSFRAMEHEADERLENGTH 7
#define LSFRAMEDATALENGTH (MAXLSFRAMELENGTH - LSFRAMEHEADERLENGTH)
typedef struct
{
  unsigned char length;
  unsigned char frameNumber;
  unsigned char reserved1;
  unsigned char reserved2;
  unsigned short id;
  unsigned char numberOfMessages;
  unsigned char data[LSFRAMEDATALENGTH]; /* 1200 bits - header */
} LsFrame, *LsFrameP;
#define NULLLSFRAME {0,0,0,0,0,0,{[0 ... LSFRAMEDATALENGTH-1] = 0}}


/**************************/
/* description of CS->LS frames */

#define MAXCSFRAMELENGTH 150

#define CSFRAMEHEADERLENGTH 6
#define CSFRAMEDATALENGTH MAXCSFRAMELENGTH - CSFRAMEHEADERLENGTH
typedef struct
{
   unsigned char mode;
   unsigned char number;
}FrameAddressField;
typedef struct
{
  unsigned char length;
  unsigned char reserved0;
  unsigned char reserved1;
  unsigned char reserved2;
  union
  {
    unsigned short single;
    FrameAddressField list;
  } address;
  unsigned char data[CSFRAMEDATALENGTH]; /* 1200 bits - header */
} CsFrame, *CsFrameP;
#define NULLCSFRAME {0,0,0,0,0,{[0 ... CSFRAMEDATALENGTH-1]=0}}

/************************************/
/* description of messages */
/* Warning : to avoid beeing confusing, we added message size field in
   the header description; to be changed ? */
 
#define LSRAWMESSAGEHEADERLENGTH 4
#define LSRAWMESSAGEDATALENGTH (LSFRAMEDATALENGTH - LSRAWMESSAGEHEADERLENGTH)
typedef struct lsrawmessagestruct
{
  unsigned char size;

#if  __BYTE_ORDER == __BIG_ENDIAN
   unsigned char completion:2;
   unsigned char slice:6;
#endif
#if  __BYTE_ORDER == __LITTLE_ENDIAN
   unsigned char slice:6;
   unsigned char completion:2;
#endif

  unsigned char type;

#if  __BYTE_ORDER == __BIG_ENDIAN
   unsigned char mess:6;
   unsigned char version:2;
#endif
#if  __BYTE_ORDER == __LITTLE_ENDIAN
   unsigned char version:2;
   unsigned char mess:6;
#endif

  unsigned char data[LSRAWMESSAGEDATALENGTH]; /* 144 - 4 bytes (header) */
}LsRawMessage, * LsRawMessageP;

#define NULLLSRAWMESSAGE {0,0,0,0,{[0 ... LSRAWMESSAGEDATALENGTH-1] = 0}}

typedef LsRawMessage CsRawMessage, *CsRawMessageP;


/*************************************/
/* new addressing modes */
#define CLFRAMESINGLEADDRESS 0x00
#define CLFRAMEBROADCAST 0xC0
#define CLFRAMELIST 0x40
#define CLFRAMEANTILIST 0x80

/* temporary? */
#define FRAMESINGLEADDRESS CLFRAMESINGLEADDRESS
#define FRAMEBROADCAST CLFRAMEBROADCAST
#define FRAMELIST CLFRAMELIST
#define FRAMEANTILIST CLFRAMEANTILIST

/* new completions */

#define CompletionMess(completion) ((completion)>>6) /* TO BE CHANGED */
#define MessCompletion(completion) ((completion)<<6) /* TO BE CHANGED */
/*******************************/

#endif












