/******************************************************************************
 *
 *
 ******************************************************************************
 */

#ifndef __GncRead__
#define __GncRead__

#include "GncInit.h"
#include "GsProtocol.h"

#define METASIZE  4
#define METABYTES METASIZE*sizeof(longWord)

int GncRead(CDASSERVER *server, void *buff, int buffSize, int *dbytes);
int GncReadAck(CDASSERVER *server);
GsStdHeader *GncGetHeaderLastRead(void);
void *GncGetDataLastRead(void);
int GncGetLenLastRead(void);

#endif
