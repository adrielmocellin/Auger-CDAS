#ifndef _SPMT_PM_H_
#define _SPMT_PM_H_

#include "Pm.h"
#include "Gnc.h"

class sPMT_Pm
{
private:
  int fPmSocket;
  CDASSERVER* fPmServer;
  int max_read_size;
public:
  sPMT_Pm(int maxsize);
  ~sPMT_Pm();
  cdasErrors ConnectToPm(int PmPort,char *PmAddr);
  int GetConn();
  cdasErrors ReadFromPm(int *buff,int *nbytes);
};

#endif
