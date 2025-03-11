#include "Pm.h"
#include "Gnc.h"
#include <algorithm>
#include <iostream>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

#include "UiCmdLine.h"
#include "IkC.h"
#include "SDDownload.h"
#include <stdio.h>
#include <stdlib.h>

#define DATALTH 10

static  CDASSERVER *server_;

static int SDDPmInit(CDASSERVER *server) {
  longWord metaData[100];
  int size;
  int rtn;

  /* first we must create the data */
  metaData[0] = htonl(1);                  /* service version */
  metaData[1] = htonl(1);                  /* # services  */
  metaData[2] = htonl(PmDOWNLOADID);       /* service  */
  metaData[3] = htonl(0);                  /* #params  */
  rtn = GncWrite(server, metaData, 16, PmDOWNLOADID,&size);
  if (rtn == CDAS_SUCCESS) {
    /* read the return message */
    longWord fromPmData[DATALTH];
    //    GsStdHeader *header;

    if ((rtn =
           GncRead(server, (void *)fromPmData,
                   DATALTH*sizeof(longWord),&size)) != CDAS_SUCCESS)
      cerr << "problem with reading Pm reply" << endl;
  } else cerr << "problem with sending Pm connect" << endl;

  return(rtn);
}

int SDDPmConnect() {
  CDASSERVER *server=NULL;

  if ((GncInit((char *)"vide",PmDOWNLOADID,PmAddr, PmPort,
               (int (*)(CDASSERVER *srvr))SDDPmInit,
               //         (int (*)(CDASSERVER *srvr))XbPmProcess,
               NULL,
               &server)) != CDAS_SUCCESS) return 0;
  server_=server;
  return 1;
}

static int tmpwrite=0;

int SDDPmSendBlock(char * buf,int size,vector<int> *list) {
  int tmp;
  //int length=size%4+4+4+list->size()*4+4+size; // Padding
  int length=size%4+4+4+4+size; // Padding
  if (list->size()==1) length+=4;
  // Version
  // number of stations (or 0)
  //   each station
  // size of data block
  // datablock
  void *Data=malloc(length);
  int *lbuf=(int*)Data;
  *lbuf++=htonl(1); // Version
  // Pm should do it correctly now
  //#define GOOD_CODE_BUT_NOT_USED
#ifdef GOOD_CODE_BUT_NOT_USED
  // It is what we should do, but not what we do because
  // it was decided that Download messages were addressed
  // only on one station or in broadcast
  *lbuf++=htonl(list->size());  // 0 for broadcast
  for (unsigned int i=0;i<list->size();i++)
    *lbuf++=htonl((*list)[i]);
#else
  if (list->size()==1) {
    *lbuf++=htonl(1);  // 1 station
    *lbuf++=htonl((*list)[0]);
  } else {
    *lbuf++=htonl(0);  // Broadcast
  }
#endif
  *lbuf++=htonl(size);
  char *cbuf=(char*)lbuf;
  for (int i=0;i<size;i++)
    *cbuf++=buf[i];
  for (int i=0;i<size%4;i++)
    *cbuf++=0;  // Padding
  GncWrite(server_,Data,length,PmDOWNLOAD,&tmp);
  tmpwrite++;
  //if (tmpwrite%2) sleep(1);
  sleep(1);  // 2 packets every 3 seconds, just to be sure.
  //usleep(550000); // YEAH!!!!! HIGH SPEED TRANSFER !!!!!!!
  free(Data);
  return 0;
}

int SDDPmClose() {
  return GncClose(server_);
}

static int SDDPmSendFirstMessage(char *filename) {
  int fsize=strlen(filename);
  char *buf=(char*)malloc(10+fsize);
  short *sbuf=(short*)buf;
  *sbuf++=htons(DownloadId);
  *sbuf++=htons(0xffff);
  *sbuf++=ntohs(NBSLICE);
  *sbuf++=ntohs(BYTESPERMESS);
  *sbuf++=ntohs(fsize);
  memcpy((void*)sbuf,filename,fsize);
  SDDPmSendBlock(buf,10+fsize,&Participating);
  free(buf);
  return 0;
}

static int SDDPmSendNextMessage(int slice,char *buf, int size) {
  char *tmp=(char*)malloc(4+size);
  short *sbuf=(short*)tmp;
  *sbuf++=htons(DownloadId);
  *sbuf++=htons(slice);
  memcpy((void*)sbuf,buf,size);
  SDDPmSendBlock(tmp,4+size,&Participating);
  free(tmp);
  return 0;
}

void SDDPmSendSlice(int slice) {
  if (slice==HEADSLICE)
    SDDPmSendFirstMessage(fileName);
  else if (slice>NBSLICE-1)
    IkWarningSend("A station requested for slice n°%d of file %s which has %d slices (from 0 to %d)! What the hell am I supposed to do?",slice,fileName,NBSLICE,NBSLICE-1);
  else {
    FILE *f=fopen(file,"r");
    fseek(f,slice*BYTESPERMESS,SEEK_SET);
    char buf[BYTESPERMESS];
    int size=fread(buf,1,BYTESPERMESS,f);
    SDDPmSendNextMessage(slice,buf, size);
    fclose(f);
  }
  sleep(BSUFUCKDELAY);
  fprintf(stderr,"#");
  fflush(stderr);
}

