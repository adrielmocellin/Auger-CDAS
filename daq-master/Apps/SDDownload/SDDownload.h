#ifndef SDD_H
#define SDD_H

#include <string>

extern string PmName;
extern char *file;
extern char *fileName;
extern int fileSize;
extern int broadcast_init;
#define CHECKEVERY 64
#define BYTESPERMESS 110 /* because of %#$! LSX */

#define HEADSLICE 65535 // 0xffff (unsigned short)

#define NBSLICE (fileSize-1)/BYTESPERMESS+1

#define NETWORKDELAY 10
#define NUMBEROFACK 3
#define TOOMANYSLICESMISSING 50
#define BSUFUCKDELAY 0

extern int DownloadId;
extern vector<int> Participating;

#endif
