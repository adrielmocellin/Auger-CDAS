/*-----------------------------------------------------------------------------*/
/*
	GsIk.h
*/
/*-----------------------------------------------------------------------------*/
#ifndef __GsIk__
#define __GsIk__


#include <IkC.h>


/**************************/
/* To handle multiple copy of code during development phase*/
extern char GsIkTestSuffix[];


/**************************/
  /* is Ik there ? */
typedef enum 
{
  GSIKWITHIK,
  GSIKWITHOUTIK
}gsIkStatus;



/**************************/
  /* needed to interface with C code */
typedef struct IkMessage * IkMessageP;



/**************************/
/* pointer to a routine in the server to process inputs */
typedef int (*serverIkHandlerP)(IkMessageP ikMessage);

#ifdef GSIKMAIN
serverIkHandlerP ServerIkMessageHandler;
#else
extern serverIkHandlerP ServerIkMessageHandler;
#endif

/***************************/
/* input, handler for IkMessage */

int GsProcessIkMessage(IkMessageP mess);

/**************************/
/* some wrappers */
const char * GsIkSource(IkMessageP mess);
const char * GsIkDest(IkMessageP mess);
const char * GsIkData(IkMessageP mess);
int GsIkType(IkMessageP mess);

/**************************/
  // input function (TO BE CHANGED ?)
void GsIkStdInput(void);


/**************************/
  // periodically called function (check for message or try to connect to Ik)
  // (Is this viable ????; TO BE CHECKED )
#ifdef GSIKMAIN
void (*GsIkCheckFunc)(char * serverName);
#else
extern void (*GsIkCheckFunc)(char * serverName);
#endif

#define GsIkCheck (*GsIkCheckFunc)

/**************************/
  // Set-up the stuff (TO BE CHANGED ?)
void GsIkInit(char * serverName);
 
/**************************/
  // Check for Ik Message
void GsIkPoll(char * serverName);




#endif
