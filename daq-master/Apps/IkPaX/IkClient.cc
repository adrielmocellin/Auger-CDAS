#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>

#define _MAIN_IKCLIENT_H_
#include "IkClient.h"
#include "IkTCP.h"
#include "IkC.h"

#include <IkCTypes.h>

#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;
int IkClientFd;

// programs need to have access to this... moved to IkClient.h
//char *gAppName;
void (*gWrapper)(IkMessage *ikm);

// split IkConnect into IkJustConnect and IkJustRequest
// plus I like IkJustConnect better now
int IkJustConnect(char *ip,char *name)
{
  struct hostent *IkServer;
  struct protoent *ipproto;
  int sockIkClientFd;
  
  if ((IkServer=gethostbyname(ip)) == NULL) {
    herror("IkJustConnect: gethostbyname");
    exit(1);
  }
  cout << "IkServer " << IkServer->h_name;
  cout << " on port " << kIkServerDataPort << endl;
  cout << "is at " << inet_ntoa(*((struct in_addr *)IkServer->h_addr));
  cout << endl;
  
  if ((ipproto = getprotobyname("ip")) == NULL)
    {
      cerr << "IkInit: getprotobyname() error" << endl;
      exit(1);
    }
  sockIkClientFd = socket(AF_INET, SOCK_STREAM,ipproto->p_proto);
  
  struct sockaddr_in dest_addr;
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(kIkServerDataPort);
  dest_addr.sin_addr = *((struct in_addr *) IkServer->h_addr);
  bzero(&(dest_addr.sin_zero),8);
  
  if (connect(sockIkClientFd, (struct sockaddr *) &dest_addr,
	      sizeof(struct sockaddr)) == -1)
    {
      perror("IkJustConnect: connect");
      exit(1);
    }
  IkClientFd = sockIkClientFd;
  gAppName = strdup(name);
  return 0;
}

int IkJustRequest(char *name, char *type, int any, void (*f)(IkMessage *ikm))
{
  string s="@";
  s += name;
  s += "|";
  if (any) s+="Any|";
  if (type) {
    s+="Type|";
    s+=type;
    s+="|";
  }
  short nb=htons(strlen(s.c_str()));
  write(IkClientFd,&nb,sizeof(short));
  write(IkClientFd,(const void *)(s.c_str()), htons(nb));
  if (f) gWrapper=f;
  return 0;
}

void IkConnect(char *ip, char *name, char *type, int any, void (*f)(IkMessage *ikm)) {
  IkJustConnect(ip, name);
  IkJustRequest(name, type, any, f);
}

char gBuffer[32768];
char *gPointer=NULL;
int msgtoread=0;
int bitstoread=0;

void IkMessageCheck(int verbose) {
  static struct timeval tv={0,0};
  struct timeval newtv;
  gettimeofday(&newtv,NULL);
  if ((newtv.tv_sec-tv.tv_sec)*1000000+(newtv.tv_usec-tv.tv_usec)<333333) return; // max 3 checks / second
  tv=newtv;
  string s="%";
  s+=gAppName;
  s+="|";
  short nb=htons(strlen(s.c_str()));
  write(IkClientFd,&nb,sizeof(short));
  write(IkClientFd,(const void *)(s.c_str()),htons(nb));
  fd_set FdSet;
  FD_ZERO(&FdSet);
  FD_SET(IkClientFd,&FdSet);
  struct timeval timeout;
  timeout.tv_sec=0;
  timeout.tv_usec=100000;
  select(IkClientFd+1,&FdSet,NULL,NULL,&timeout);
  if (!FD_ISSET(IkClientFd,&FdSet)) {
   InternalLog("No answer from Ik...",IKWARNING);
   return;
  }
  // if we are not with pending messages...
  if (!msgtoread) {
    read(IkClientFd,(void *)&nb,sizeof(short));
    msgtoread=htons(nb);
  }
  if (verbose && msgtoread) cerr << msgtoread << " messages to read" << endl;
  while (msgtoread) {
    // if we are not in the middle of a message
    if (!bitstoread) {
      short size;
      read(IkClientFd,(void *)&size,sizeof(short));
      bitstoread=htons(size);
    }
    if (!gPointer) gPointer=gBuffer;
    if (verbose) cerr << "message size left: " << bitstoread << endl;
    int nbread=read(IkClientFd,(void *)gPointer,bitstoread);
    bitstoread-=nbread;
    if (bitstoread) { // we coudn't finish reading a message
      gPointer+=nbread;
      return;
    }
    // we finished one message
    msgtoread--;
    bitstoread=0;
    gPointer=gBuffer;
    // trying to IkMessage it
    IkMessage mess;
    if (gWrapper && mess.FromBuffer(gBuffer)) {
      // IkCTypify  is the autotypification program.
      // We'd PREFER to remove it once programs are used to dealing with
      // raw IkMessages rather than typed IkMessages.
      // For now it stays.
      IkMessage *IkCTypifiedMess = IkCTypify(mess);
      gWrapper(IkCTypifiedMess);
      
      // IkCTypifiedMess is declared - it needed to escape the bounds
      // of the function above
      delete IkCTypifiedMess;
    }
  }
}

void IkMessageSend(IkMessage &ikm) {
  string s=ikm.FullMessage();
  short nb=htons(strlen(s.c_str()));
  write(IkClientFd,&nb,sizeof(short));
  write(IkClientFd,(const void *)(s.c_str()),htons(nb));
}
