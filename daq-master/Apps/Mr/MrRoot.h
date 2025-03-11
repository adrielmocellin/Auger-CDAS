#ifndef _MR_ROOT_H_
#define _MR_ROOT_H_

int OpenRootFile();
int CloseRootFile();
int FileSize();
int FileOk();
int BookTree();
int FillSDMonCal();
#ifdef OLD_MOR_VERSION
int FillSDMon();
int FillSDMonRaw();
int FillSDCalib();
#endif
bool CloseFileTest();
bool BookingTreeTest();
int CallMoRSForSummary();

#endif
