#ifndef _IKTCP_
#define _IKTCP_
#include <string>

#define kIkServerDataPort 55561

void TCPInit(int dataPort=kIkServerDataPort);
void TCPSelect(int verbose=0);

#endif
