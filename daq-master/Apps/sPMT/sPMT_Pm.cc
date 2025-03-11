#include "sPMT_Pm.h"
#include "PmProtocol.h"
#include "UiErr.h"

#include <cstdio>
#include <cstdlib>
#define kDATALTH 10
int SPMT_PmInit(CDASSERVER *server)
{
  longWord metaData[100];
  int nbytes;
  int rtn;

  printf("Start PM conn. \n");
  /* first we must create the data */
  metaData[0] = htonl(1);                  /* service version */
  metaData[1] = htonl(1);                  /* # services  */
  metaData[2] = htonl(PmSpmt);
  metaData[3] = htonl(0);

  rtn = GncWrite(server, metaData, 4*sizeof(longWord),
		 PmSpmt,&nbytes);
  if (rtn == CDAS_SUCCESS) {
    int err;

    longWord fromPmData[kDATALTH];
    GsStdHeader *header;
    err = GncRead(server, (void *)fromPmData,
		  kDATALTH*sizeof(longWord),&nbytes);

    if (err==CDAS_SUCCESS) {
      header = GncGetHeaderLastRead();
    }
  }
  return rtn;
}



sPMT_Pm::sPMT_Pm(int maxsize)
{
  max_read_size=maxsize; /*maximum data size buffer which will read from PM */
  fPmServer=NULL;
  fPmSocket=-1;
}

sPMT_Pm::~sPMT_Pm()
{
  if(0<fPmSocket){
    close(fPmSocket);
  }
}
cdasErrors sPMT_Pm::ConnectToPm(int port,char *addr)
{
  if ( GncInit("Pm", PmSpmt, addr, port,
	       (int (*)(CDASSERVER *srvr))SPMT_PmInit,
               NULL, &fPmServer) != CDAS_SUCCESS){
    return CDAS_CONNECT_ERROR;
  }
  fPmSocket = fPmServer->fd;
  return(CDAS_SUCCESS);
}

int sPMT_Pm::GetConn()
{
  return fPmSocket;
}

cdasErrors sPMT_Pm::ReadFromPm(int *buff,int *nbytes)
{
  int err,i,j;
  unsigned char *pt;
  printf("Start Read from Pm\n");
  if(fPmSocket<0)
    return(CDAS_READ_ERROR);
  err = GncRead(fPmServer, buff, max_read_size,nbytes);
  if (err != CDAS_SUCCESS) {
    printf("pm read error\n");
    GncClose(fPmServer);
    return(CDAS_READ_ERROR);
  }
  printf("Finish Read from Pm %d\n",*nbytes);
  return(CDAS_SUCCESS);
}
