#ifndef _IKC_H
#define _IKC_H

#include <string>
#include <iostream>
#include <sstream>

#include "IkCTypes.h"
#include "IkClient.h"

#define LOG "Log"
#define RC "Rc"
#define PM "Pm"
#define EB "Eb"
#define SU "Su"
#define CT "Xb"

int CDASInit(const char *name, const char *IkServerHostname="ns");
int IkMonitor(void (*function)(IkMessage *ikm), const char *fmt, ...);
int IkDebugSend(const char *fmt,...);
int IkInfoSend(const char *fmt,...);
int IkWarningSend(const char *fmt,...);
int IkFatalSend(const char *fmt,...);
int IkSevereSend(const char *fmt,...);
int IkInfoSend(const char *fmt,...);
int IkSoundSend(const char *file, int volume);
int IkSpeechSend(const char *fmt,...);

// Logging stuff
void InternalLog(const char *text,string err); // Use this one with one of the followinf #define
#define IKFATAL "Fatal"
#define IKERROR "Error"
#define IKWARNING "Warning"
#define IKINFO "Info"
#define IKDEBUG "Debug"
// example:
//
//  {
//    stringstream s;
//    s << "Added client " << clientname;
//    InternalLog(s.str().c_str(),IKINFO);
//  }


void LogMessage(const char *message); // don't use this one (for IkServer only)
#endif
