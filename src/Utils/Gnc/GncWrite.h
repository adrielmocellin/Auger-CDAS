/******************************************************************************
 *
 *
 ******************************************************************************
 */

#ifndef __GncWrite__
#define __GncWrite__

#include "GncInit.h"
#include "GsProtocol.h"

int GncWrite(CDASSERVER *server, void *buff, int buffSize, longWord type,
	     int *bytesWritten);

#endif
