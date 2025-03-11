#ifndef XBPM_H
#define XBPM_H
#include "XbT2.h"
#include "PmProtocol.h"

int XbPmConnect(char *name,PmMessageType service=PmT2);
int XbPmSendStatus(const char* dest);
int XbPmGetT2(struct t2 *input, int official=1, struct t2* scaler=NULL, int * nscp=NULL); // if official will write t2_* and scaler_* files and complain about missing stations

#endif
