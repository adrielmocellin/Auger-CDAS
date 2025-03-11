/*-----------------------------------------------------------------------------*/
/*
	Gs.h
*/
/*-----------------------------------------------------------------------------*/
#ifndef __Gs__
#define __Gs__



/*-----------------------------------------------------------------------------*/
/* #include <dmalloc.h>*/
#include "GsProtocol.h"
#include "GsClients.hxx"
#include "GsServices.h"
#include "GsMess.h"

#include "GsIk.h"
#include "GsNotify.h"

// stupid Gs...
#define IK_INFO 0
#define IK_WARNING 1
#define IK_SEVERE 2
#define IK_FATAL 3


/*-----------------------------------------------------------------------------*/
/* function prototypes and global variable. */
/* to be moved elsewhere */

#define MMAXLEN         1024
#ifdef GSMAIN
/* miscellanea */
char ServerName[MMAXLEN+IKMAXSUFFIX]={0};
#else
extern char ServerName[];
#endif


/*-----------------------------------------------------------------------------*/

#endif

