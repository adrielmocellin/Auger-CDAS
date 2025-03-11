#define _MORUTILITY_

#include <sys/stat.h>
#include <iostream>
#include <stdio.h>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <algorithm>

extern "C"
{
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/types.h>
}

#include "MrSignal.h"
#include "MrRoot.h"

#include "IkC.h"

static void ToDoBeforeFinishing(string signal) {
  cerr << signal << "signal received " << endl; 
  cerr << " close output file "  << endl; 
  CloseRootFile();
  IkSoundSend("loser.wav",10); 
  IkSoundSend("Goodbye.wav",10);
  string info = "Exiting on ";
  info += signal;
  info += " signal"; 
  IkFatalSend(info.c_str());
}

static void HandleSignalTerm(int code) {
    ToDoBeforeFinishing("SIGTERM");
    exit(1);
}

static void HandleSignalUsr1(int code) {
  ToDoBeforeFinishing("SIGUSR1");
  exit(1);
}

static void HandleSignalUsr2(int code) {
  ToDoBeforeFinishing("SIGUSR2");
  exit(1);
}

static void HandleSignalQuit(int code) {
  ToDoBeforeFinishing("SIGQUIT");
  exit(1);
}

static void HandleSignalInt(int code) {
  ToDoBeforeFinishing("SIGINT");
  exit(1);
}

static void HandleSignalHup(int code) {
  ToDoBeforeFinishing("SIGHUP");
  exit(1);
}

// ####################################
                    
void InitUnixSignals() {

  struct sigaction sga;
  
  sga.sa_handler = HandleSignalTerm;
  sigemptyset(&(sga.sa_mask));
  sga.sa_flags=0;
  
  sga.sa_handler = HandleSignalTerm;
  if(sigaction(SIGTERM,&sga,0)<0) {
    cerr << " error sigaction SIGTERM ::" << strerror(errno) << endl;
    exit(-1);
  }
  
  sga.sa_handler = HandleSignalUsr1;
  if(sigaction(SIGUSR1,&sga,0)<0) {
    cerr << " error sigaction SIGUSR1 ::" << strerror(errno) << endl;
  }

  sga.sa_handler = HandleSignalUsr2;
  if(sigaction(SIGUSR2,&sga,0)<0) {
    cerr << " error sigaction SIGUSR2 ::" << strerror(errno) << endl;
  }

  sga.sa_handler = HandleSignalQuit;
  if(sigaction(SIGQUIT,&sga,0)<0) {
    cerr << " error sigaction SIGQUIT ::" << strerror(errno) << endl;
  }

  sga.sa_handler = HandleSignalInt;
  if(sigaction(SIGINT,&sga,0)<0) {
    cerr << " error sigaction SIGINT ::" << strerror(errno) << endl;
  }

  sga.sa_handler = HandleSignalHup;
  if(sigaction(SIGHUP,&sga,0)<0) {
    cerr << " error sigaction SIGHUP ::" << strerror(errno) << endl;
  }
};
//------------------------------------------------------------------

