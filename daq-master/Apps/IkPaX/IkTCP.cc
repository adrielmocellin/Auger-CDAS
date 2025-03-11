#include <stdio.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
 
#include <stdio.h>
#include <iostream>
#include <sstream>
using namespace std;

#include <vector>
#include <algorithm>

#include "Ik.h"
#include "IkC.h"
#include "IkTCP.h"

#define kTCPBUFSIZE 100000
int gConnectFd;
fd_set gFdSet;
vector<int> gFd;
vector<int> gFdLoop;

char gTCPBuf[kTCPBUFSIZE];

void TCPInit(int dataPort) {
  struct sockaddr_in hsta;

  if((gConnectFd=socket(AF_INET,SOCK_STREAM,0))==(-1)) {
    InternalLog("socket() error",IKFATAL);
    exit(0);
  }

  hsta.sin_addr.s_addr=INADDR_ANY;
  hsta.sin_family=AF_INET;
  hsta.sin_port=htons(dataPort);

  if((bind(gConnectFd,(struct sockaddr *)&hsta,sizeof(struct sockaddr_in)))==(-1)) {
    stringstream s;
    s << "bind() error for port " << dataPort;
    InternalLog(s.str().c_str(),IKFATAL);
    exit(0);
  }

  if ((listen(gConnectFd,0))==(-1)) {
    InternalLog("listen() error",IKFATAL);
    exit(0);
  }
  gFd.push_back(gConnectFd);
}

void TCPGetClient() {
  struct sockaddr addr;
  socklen_t addrlen=sizeof(addr);
  int fd=accept(gConnectFd, &addr,&addrlen);
  stringstream s;
  s << "New client, using fd " << fd;
  InternalLog(s.str().c_str(),IKINFO);
  gFd.push_back(fd);
}

void TCPListenClient(int fd,int verbose) {
  if (verbose) {
    stringstream s;
    s << "Client " << fd << " talked ";
    InternalLog(s.str().c_str(),IKINFO);
  }
  short size;
  int rb=read(fd,&size,sizeof(short));
  if (rb==0) {
    stringstream s;
    s << "0 bytes read on fd " << fd << ", assuming client deconnected";
    InternalLog(s.str().c_str(),IKINFO);
    close(fd);
    gFd.erase(find(gFd.begin(),gFd.end(),fd));
    return;
  }
  if (rb<0) {
    stringstream s;
    s << "Error in reading, deconnecting client fd " << fd;
    InternalLog(s.str().c_str(),IKERROR);
    close(fd);
    gFd.erase(find(gFd.begin(),gFd.end(),fd));
    return;
  }
  rb=read(fd,&gTCPBuf,htons(size));
  if (rb==0) {
    stringstream s;
    s << "0 bytes read on fd " << fd << ", assuming client deconnected";
    InternalLog(s.str().c_str(),IKINFO);
    close(fd);
    gFd.erase(find(gFd.begin(),gFd.end(),fd));
    return;
  }
  if (rb!=htons(size)) {
    stringstream s;
    s << rb << " bytes read on fd " << fd << " instead of " << htons(size) <<", deconnecting client";
    InternalLog(s.str().c_str(),IKERROR);
    close(fd);
    gFd.erase(find(gFd.begin(),gFd.end(),fd));
    return;
  }
  if (rb<0) {
    stringstream s;
    s << "Error in reading, deconnecting client fd " << fd;
    InternalLog(s.str().c_str(),IKERROR);
    close(fd);
    gFd.erase(find(gFd.begin(),gFd.end(),fd));
    return;
  }
  ProcessMessage(gTCPBuf,fd);
}

void TCPSelect(int verbose) {
  struct timeval timeout;
  timeout.tv_sec=60;
  timeout.tv_usec=0;
  sort(gFd.begin(),gFd.end());
  FD_ZERO(&gFdSet);
  for (unsigned int i=0;i<gFd.size();i++)
    FD_SET(gFd[i],&gFdSet);
  int sel=select(gFd[gFd.size()-1]+1,&gFdSet,NULL,NULL,&timeout);
  if (verbose) {
    stringstream s;
    s << "Select returns " << sel << " (" << gFd.size() << " fd watched)";
    InternalLog(s.str().c_str(),IKDEBUG);
  }
  gFdLoop = gFd;
  for (unsigned int i=0;i<gFdLoop.size();i++) {
    if (!FD_ISSET(gFdLoop[i],&gFdSet)) continue;
    if (verbose) {
      stringstream s;
      s << gFdLoop[i] << " Is set ";
      InternalLog(s.str().c_str(),IKDEBUG);
    }
    if (gFdLoop[i]==gConnectFd) TCPGetClient();
    else TCPListenClient(gFdLoop[i],verbose);
  }
}
