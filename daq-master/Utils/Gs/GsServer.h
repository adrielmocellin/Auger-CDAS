/*-----------------------------------------------------------------------------*/
/*
	GsServer.h
	         file to be included in Server applications
*/
/*-----------------------------------------------------------------------------*/
#ifndef __GsServer__
#define __GsServer__

#include "Gs.h"
#include "GsClients.hxx"
#include "GsServices.h"

/*-----------------------------------------------------------------------------*/
/* function prototypes and global variable. */
/* to be moved elsewhere */


extern void GsInitTCPIP(int dataPort, int queueLen, int *portfd);
extern int removeDataClient(int code,GsClientP client);
extern void GsTerminate(int errorCode);


#endif
