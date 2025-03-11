#ifndef FD_H
#define FD_H

// statistical veto values
#define PHYSRATE (1./30.)
#define DEADTIME 0.01

// Basic parameters
//FIX increased coincidence window to +/- 120 usec . ALS 09/12/2001
//Decreased coincidence window to +/- 20 usec . ALS 24/09/2003
//Increased coincidence window to +/- 40 usec . ALS 30/09/2003
//After loosing probably a few events, moved back to 120. XB 26/10/2003
//Time is now ok. Going to 20 XB 19/03/2005
#define WINDOW 20

// central offset, not clear it is needed...
#define CENTRALOFFSET 120

#define TRACELENGTH 13
// 512 bins of 25 ns == 12800 ns, roughly 13us

int FdIk(IkMessage *ikm);

extern int use_phi; // to know if we use the phi information from Fd
extern int verbose,debug;

extern string PmName;

void FdIkPong();

void FdTriggerArrayInit();

int FdTriggerRead(unsigned int *site,unsigned short *t3id,unsigned int *second,unsigned int *tzero,double *phi);
int FdTriggerT3Emit(unsigned int site,unsigned short t3id,unsigned int second,unsigned int tzero,double phi);

#include "T3Notify.h"
int IsValidT3(T3Notify *t3);

#endif
