#ifndef XBIK_H
#define XBIK_H
#include "XbT2.h"

void XbIk(IkMessage *ikm);
unsigned short XbIkGetId();
void XbIkInitId(char*filename="/Raid/var/Xb", int t3idmin=0, int t3idmax=4090);
void XbIkT3Emit(t3 in);

#endif
