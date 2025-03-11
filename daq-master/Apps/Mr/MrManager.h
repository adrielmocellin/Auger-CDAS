#ifndef __MrManager__
#define __MrManager__

#include <sys/time.h>

#include <Pm.h>
#include <Gnc.h>

#include "CbRawData.h"
#include "MoRawData.h"
#include "TSDSystem.h"

//#define kLS 1702
#define kLS (2047+2048)

class TMrManager {
 public:
  TMrManager();
  ~TMrManager() {};

  cdasErrors ConnectToPm(int, char*);
  cdasErrors ListenPm();

  void Interrupts() { IkMessageCheck(); };

  void FillTree(int);
  void RequestCalibFromSD();

 private:
  cdasErrors ListenOnPort();

  TCbRawData *fCalibData; // Tables where calibration data are stored waiting to be written on disk
  TMoRawData *fMonitData; // Tables where monitoring data are stored waiting to be written on disk

  int         fSocketFd;
  int         fListen;

  fd_set      fGlobalFd;
  CDASSERVER* fPmServer;
  int         fPmSocket;

  time_t fLastRequestCalibSecond; // Stores the last time (in second) we have requested the calibration
};

#endif
