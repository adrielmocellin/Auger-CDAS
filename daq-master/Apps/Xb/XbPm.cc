/*! \file
    \brief Interaction with Pm: connection and data receiving
*/


#include "Pm.h"
#include "PmDefs.h"
#include "Gnc.h"
#include <algorithm>
#include <netinet/in.h>
#include <errno.h>
#include <sstream>

#include "UiCmdLine.h"
#include "IkC.h"
#include "IkSDSummary.hxx"
#include "IkSDUpdate.hxx"

#include "XbIk.h"
#include "XbArray.h"

#define SCALERVALUE 7

#define DATALTH 10
#define MODE (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)

static  CDASSERVER *server_;
unsigned int second_=0,waitpm=5;
static char *PresentStations=NULL;
static char *ThisSecondStations=NULL;
PmMessageType service_;

void XbPmSendStatus(const char *dest) {
  IkSDSummary ikm;
  ikm.second=second_;
  for (int i=0;i<ROW;i++)
    if(PresentStations[i]==1) ikm.stations.push_back(i);
  ikm.send(dest);
}

static int XbPmInit(CDASSERVER *server) {
  longWord metaData[100];
  int size;
  int rtn;

  if (PresentStations==NULL) PresentStations=(char*)malloc(ROW);
  if (ThisSecondStations==NULL) ThisSecondStations=(char*)malloc(ROW);
  for (int i=0;i<ROW;i++)
    PresentStations[i]=ThisSecondStations[i]=0;
  /* first we must create the data */
  metaData[0] = htonl(1);                  /* service version */
  metaData[1] = htonl(1);                  /* # services  */
  metaData[2] = htonl(service_);            /* service  */
  metaData[3] = htonl(0);                  /* #params  */
  rtn = GncWrite(server, metaData, 16, service_,&size);
  if (rtn == CDAS_SUCCESS) {
    /* read the return message */
    longWord fromPmData[DATALTH];
    if ((rtn = GncRead(server, (void *)fromPmData,
                       DATALTH*sizeof(longWord),&size)) != CDAS_SUCCESS) {
      stringstream s;
      s << "problem with reading Ok for T2 reply";
      InternalLog(s.str().c_str(),IKERROR);
    }
  } else {
    stringstream s;
    s << "problem with sending Ok for T2";
    InternalLog(s.str().c_str(),IKERROR);
  }
  return(rtn);
}

int XbPmConnect(char *name,PmMessageType service) {
  service_=service;
  CDASSERVER *server=NULL;
  if ((GncInit(name,service_,PmAddr, PmPort, (int (*)(CDASSERVER *srvr))XbPmInit, NULL, &server)) != CDAS_SUCCESS) return 0;
  server_=server;
  return 1;
}

int XbPmGetT2(struct t2 *input, int official, struct t2 *scaler, int *nscp) {
  static int _IkmInOutnotSent = 1000;
  int rtn, size;//bytes, lwords
#define  bufsize  150000
  GsStdHeader * header;
  int nt2=0,nstat=0,config=0,nsc=0;

  union {
    int ibuf[bufsize];
    char cbuf[bufsize*sizeof(int)];
  } buff;

  fd_set fdset;
  int fdesc=server_->fd;
  int sel;
  struct timeval timeout;
  FILE *f=NULL,*fscal=NULL;

  for(unsigned int i=0;i<waitpm;i++) {
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    FD_ZERO(&fdset);
    FD_SET(server_->fd,&fdset);
    sel = select(fdesc+1,&fdset,NULL,NULL,&timeout);
    if (nt2==0 && sel==-1 && errno==EBADF) {
      IkFatalSend("Pm seems dead. Giving up. Goodbye");
      exit(1);
    }
    if((sel == 1 && FD_ISSET(server_->fd ,&fdset)) && (timeout.tv_sec == 0)) break;
    // To answer Su even when Pm has no T2...
    IkMessageCheck();
  }
  if(!(sel == 1 && FD_ISSET(server_->fd ,&fdset)) && (timeout.tv_sec == 0)) {
    IkWarningSend("No T2 received from Pm for %d seconds !",2*waitpm-5); // !!
    nt2=-1;
    waitpm*=2;
  } else waitpm=5;

  if(nt2==0 && (rtn = GncRead(server_, buff.ibuf, bufsize,&size)) != CDAS_SUCCESS) {
    /* error during the read, close the connection */
    stringstream s;
    s << " error during the read";
    InternalLog(s.str().c_str(),IKERROR);
    GncClose(server_);
    nt2=-1;
  } else if (nt2==0) {
    /* check if it was a T2 message */
    header = GncGetHeaderLastRead();
    if (!header) {
      stringstream s;
      s << " error getting last header: BUG!!! ";
      InternalLog(s.str().c_str(),IKERROR);
    } else if (header->type != service_ ) {
      stringstream s;
      s << " got message type : " <<  header->type;
      InternalLog(s.str().c_str(),IKERROR);
    } else {
      // Skip the version number of the message
      //char * cP = buff.cbuf + sizeof(longWord);
      unsigned int version = ntohl(*(int*)(buff.cbuf));
      unsigned int tmpsecond;
      char * cP;
      switch (version) {
      case 1 :
        tmpsecond = 0;
        cP = buff.cbuf + sizeof(longWord);
        break;
      case 2:
        tmpsecond = ntohl(*(int*)(buff.cbuf+sizeof(longWord)));
        cP = buff.cbuf + 2*sizeof(longWord);
        break;
      default :
	stringstream s;
        s << " CtPm bad T2 version number : " <<  version;
	InternalLog(s.str().c_str(),IKERROR);
        exit(1);
      }
      unsigned int firstt2=0;
      short * sP;
      int id, number;
      int msec;
      nt2=0;
      nsc=0;
      unsigned int oldsecond=second_;
      int pos=0;
      longWord *lP;
      int readsecond;
      while (cP < buff.cbuf+size-6) { // the "-6" is due to four byte padding
        sP = (short*) cP;
        id = ntohs(*sP++);
        number = ntohs(*sP++);
        lP = (longWord *) sP;
        readsecond = ntohl(*lP++);
        cP=(char*) lP;
        if (number!=0) {
          second_=readsecond;
          if (id<ROW) ThisSecondStations[id]=1;
          if (!firstt2) {
            time_t t = second_;
            t += 315964800;
            struct tm * stm=gmtime(&t);
            if (f) fclose(f);
            if (fscal) fclose(fscal);
            stringstream dirnm;
            stringstream fileend;
            stringstream s1;
            stringstream s2;
            dirnm << "/Raid/var/Ct/";
            mkdir(dirnm.str().c_str(),MODE);
            dirnm << stm->tm_year+1900 << "/";
            mkdir(dirnm.str().c_str(),MODE);
            if (stm->tm_mon<9) dirnm << "0";
            dirnm << stm->tm_mon+1 << "/";
            mkdir(dirnm.str().c_str(),MODE);
            fileend << "_" << stm->tm_year+1900 << "_";
            if (stm->tm_mon<9) fileend << "0";
            fileend << stm->tm_mon+1 << "_";
            if (stm->tm_mday<10) fileend << "0";
            fileend << stm->tm_mday << "_00h00.dat";
            s1 << dirnm.str() << "second" << fileend.str();
            s2 << dirnm.str() << "scaler" << fileend.str();
            if (official) {
              f=fopen(s1.str().c_str(),"a");
              fscal=fopen(s2.str().c_str(),"a");
              fprintf(f,"%u ",second_);
              fprintf(fscal,"%u ",second_);
            }
            firstt2=1;
          }
          if (second_!=tmpsecond)  {
            IkWarningSend("Pm sent in the same block messages from different seconds");
	    stringstream s;
            s << "BUG:" << id << " " << number << " " << pos << " old:"
            << tmpsecond << " new:" << second_ ;
	    InternalLog(s.str().c_str(),IKERROR);
          }
          pos++;
          cP=(char*) lP;
          if (debug) {
	    stringstream s;
            s << " Ls " << id << " sent " << number << " T2" << " for second " << second_ ;
	    InternalLog(s.str().c_str(),IKDEBUG);
          }
          if (f!=NULL) fprintf(f,"%d %d ",id,number);
          nstat++;
          for (int i=0;i<number;i++) { // Is station in neighbour?
            T2LsP stat= (T2LsP) cP;
            msec=((unsigned int)(stat->timeLow))
                 +((unsigned int)(stat->timeMid))*256
                 +((unsigned int)(stat->timeHigh))*256*256;
            cP += sizeof(T2Ls);
            if (id<ROW && second_==tmpsecond) {//Is station in neighbour array?(else it will segfault) and is time good ?
              if (stat->energie==SCALERVALUE) { // scalers
                if (scaler) scaler[nsc].ID=id;
                if (scaler) scaler[nsc].time=msec;
		nsc++;
                if (fscal!=NULL) fprintf(fscal,"%d %d ",id,msec);
              } else { // WARNING!!!! HERE INFILL WAS THROWN AWAY!!!! NOW NO MORE!!!!!! IS THROWN AWAY LATER ON!!!!
                input[nt2].ID=id;
                input[nt2].time=msec;
                input[nt2].energy=stat->energie;
                nt2++;
                if (debug) {
	          stringstream s;
		  s << "++ station:" << id << " T2: nrj:" << input[nt2].energy << " time:" << msec ;
	          InternalLog(s.str().c_str(),IKDEBUG);
                }
              }
            }
            if ((id<=2048) && (id>=ROW || exists[id]==0)) {
              if (second_%60==0 && !config) {
                XbArrayInit(1);  // reconfiguring to add the station once per minute
                config=1;
                if (verbose) {
		  stringstream s;
                  s << "Unknown station:" << id ;
	          InternalLog(s.str().c_str(),IKWARNING);
                }
              }
            }
          }
        }
        // Because of parity check... (T2 are two byte padded)
        if ((number*sizeof(T2Ls)) & 0x1) cP++;
        if ((number*sizeof(T2Ls)) & 0x10) cP+=2;
      }
      if (second_!=tmpsecond && tmpsecond!=0) second_=tmpsecond;
      if (second_<=oldsecond) {
        IkWarningSend("Pm sent informations from the past (current=%u) (Pm sent=%u)",oldsecond,second_);
        second_=oldsecond;
        if (f!=NULL) {
          fprintf(f,"-1 -1\n");
          fclose(f);
        }
        if (fscal!=NULL) {
          fprintf(fscal,"-1 -1\n");
          fclose(fscal);
        }
        return 0;
      } else if (oldsecond!=0 && second_-oldsecond!=1) IkWarningSend("Pm didn't send informations beetwen second %u and %u" ,oldsecond,second_);
    }
  }
  if (f!=NULL) {
    fprintf(f,"-1 -1\n");
    fclose(f);
  }
  if (fscal!=NULL) {
    fprintf(fscal,"-1 -1\n");
    fclose(fscal);
  }
  if ((_IkmInOutnotSent > 60) && official) {
    IkSDUpdate ikm;
    for (int i=0;i<ROW;i++) {
      if (ThisSecondStations[i]>0 && PresentStations[i]==0) {
        ikm.in.push_back(i);
        PresentStations[i]=1;
      } else if ((nt2==-1 || ThisSecondStations[i]==0) && PresentStations[i]==1) {
        ikm.out.push_back(i);
        PresentStations[i]=0;
      }
      ThisSecondStations[i]=0;
    }
    if (ikm.in.size()>0 || ikm.out.size()>0) {
      ikm.second=second_;
      ikm.send(LOG);
      _IkmInOutnotSent = 0;
    }
  }
  _IkmInOutnotSent++;
  if (verbose) {
    stringstream s;
    s << nstat << " stations sending T2s" ;
    InternalLog(s.str().c_str(),IKINFO);
  }
  if (nscp) *nscp=nsc;
  return nt2;
}
