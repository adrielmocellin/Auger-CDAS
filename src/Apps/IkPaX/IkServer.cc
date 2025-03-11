#include <list>
#include <string>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>


#include "IkTCP.h"
#include "IkC.h"
#include "IkMessage.h"

// seconds without doing an IkMessageCheck => 60 to have time to reset
#define MAXINACTIVITY 60

#define MODE (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)

int verbose=0;

using namespace std;
 
class Queue {
public:
  string ClientName;
  string WantedType;
  bool Any;
  time_t LastSecond;
  list <IkMessage> Messages;
};

list <Queue> QueueList;

FILE *gLogFile=NULL;
int gDay=0;

int AddClient(string clientname,int fd) {
  list<Queue>::iterator q=QueueList.begin();
  while (q!=QueueList.end()) {
    if ((*q).ClientName==clientname) return 0;
    q++;
  }
  Queue *nq=new Queue();
  nq->Any=false;
  nq->WantedType="";
  nq->ClientName=clientname;
  nq->LastSecond=time(NULL);
  QueueList.push_back(*nq);
  stringstream s;
  s << "Added client " << clientname << " on fd " << fd;
  InternalLog(s.str().c_str(),IKINFO);
  return 1;
}

void QueueMessage(IkMessage &message) {
  list<Queue>::iterator q=QueueList.begin();
  time_t t=time(NULL);
  while (q!=QueueList.end()) {
    if ((*q).Any || (*q).ClientName==message.Dest || (*q).WantedType==message.Type) {
      (*q).Messages.push_back(message);
      if (t>(*q).LastSecond+MAXINACTIVITY) {
	stringstream s;
        s << "Cleaning " << (*q).ClientName << " queue and stopping it, inactivity time too long";
	InternalLog(s.str().c_str(),IKWARNING);
        (*q).Messages.clear();
        (*q).LastSecond=t;
	stringstream s2;
        s2 << "/Raid/usr/local/bin/cdasurvey stop " << (*q).ClientName << " >> /Raid/tmp/cdas/ikout 2>> /Raid/tmp/cdas/ikerr &";
        system(s2.str().c_str());
      }
    }
    q++;
  }
}

void ProcessIkRequest(char *buf,int fd) {
  // IkRequest format: Sender|Type|<typewanted>| or Sender|Any| or Sender|
  char *tmp=strchr(buf,'|');
  if (!tmp) {
    InternalLog("Badly formated IkRequest",IKERROR);
    return;
  }
  *tmp='\0';
  list<Queue>::iterator q=QueueList.begin();
  while (q!=QueueList.end()) {
    if (strcmp((*q).ClientName.c_str(),buf)==0)
      break;
    q++;
  }
  if (q==QueueList.end()) {// New client
    string s(buf);
    AddClient(s,fd);
  }
  char *tmp2=strchr(tmp+1,'|');
  if (!tmp2) { // no special request
    return;
  }
  *tmp2='\0';
  q=QueueList.begin();
  while (q!=QueueList.end()) {
    if (strcmp((*q).ClientName.c_str(),buf)==0) { // found it
      if (strcmp(tmp+1,"Any")==0) {
        (*q).Any=true;
	stringstream s;
        s << "Adding 'Any' request for " << (*q).ClientName;
	InternalLog(s.str().c_str(),IKINFO);
      } else {
        if (strcmp(tmp+1,"Type")!=0) {
	  InternalLog("Badly formated IkRequest",IKERROR);
	  return;
        }
	char *tmp3=strchr(tmp2+1,'|');
	if (!tmp3) {
	  InternalLog("Badly formated IkRequest",IKERROR);
	  return;
	}
	*tmp3='\0';
        string wt(tmp2+1);
        (*q).WantedType=wt;
	stringstream s;
        s << "Adding Type " << wt << " request for " << (*q).ClientName;
	InternalLog(s.str().c_str(),IKINFO);
	*tmp3='|';
      }
    }
    q++;
  }
}

void ProcessIkMessage(char *buf) {
  IkMessage message;
  if (message.FromBuffer(buf)) {
    LogMessage(message.FullMessage().c_str());
    QueueMessage(message);
  }
}

void FlushMessage(char *buf,int fd) {
  char *tmp=strchr(buf,'|');
  if (!tmp) {
    InternalLog("Badly formated Flush request",IKERROR);
    return;
  }
  *tmp='\0';
  list<Queue>::iterator q=QueueList.begin();
  if (q==QueueList.end()) {  // no queue, what is happening?
    short nb=htons(0);
    write(fd,(const void *)&nb,sizeof(short));
  }
  while (q!=QueueList.end()) {
    if (strcmp((*q).ClientName.c_str(),buf)==0) {
      (*q).LastSecond=time(NULL);
      if (verbose && (*q).Messages.size()){
        stringstream s;
        s << "Flushing " <<  (*q).Messages.size() << " messages for " << buf;
        InternalLog(s.str().c_str(),IKDEBUG);
      }
      short nb=htons((*q).Messages.size());
      write(fd,(const void *)&nb,sizeof(short));
      list<IkMessage>::iterator m=(*q).Messages.begin();
      while (m!=(*q).Messages.end()) {
        string s=(*m).FullMessage();
        short nb=htons(strlen(s.c_str()));
	write(fd,(const void *)&nb,sizeof(short));
	write(fd,(const void *)(s.c_str()),htons(nb));
        m++;
      }
      (*q).Messages.clear();
    }
    q++;
  }
}

void ProcessMessage(char *buf,int fd) {
  switch (buf[0]) {
    case '@':
      ProcessIkRequest(buf+1,fd);
      break;
    case '|':
      ProcessIkMessage(buf);
      break;
    case '%':
      FlushMessage(buf+1,fd);
      break;
    default:
      InternalLog("Unknown message received, not parsing",IKERROR);
      return;
  }
}

int main() {
  InternalLog("Ik Server launched",IKINFO);
  TCPInit(kIkServerDataPort); 
  InternalLog("Ik Server ready",IKINFO);
  while (1) TCPSelect();
  return 0;
}
