#ifndef __PmLib__
#define __PmLib__

// Basic system inclusion
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <errno.h>
#include <assert.h>

// Containers template
#include <iterator>
#include <vector>
#include <list>

// Miscs
#include <endian.h>
#include <algorithm>
#include <functional>

// RPC communication
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h> 
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

// CDAS inclusion
#include <UiErr.h>
#include <UiCmdLine.h>
#include <UiArgs.h>
#include <UiMessage.h>
#include "central_local.h"
#include "IkC.h"
#include "ClFrames.h"

// Ik messages inclusion
#include "IkT3.hxx"
#include "IkSuPing.hxx"
#include "IkSuPong.hxx"
#include "IkWarning.hxx"
#include "IkSevere.hxx"
#include "IkInfo.hxx"

#include "IkLsWakeUp.hxx"
#include "IkLsReboot.hxx"
#include "IkLsLoadConf.hxx"
#include "IkLsSaveConf.hxx"
#include "IkLsChangeState.hxx"
#include "IkLsMonReq.hxx"
#include "IkLsCalReq.hxx"
#include "IkLsGpsSet.hxx"
#include "IkLsConfig.hxx"
#include "IkLsReady.hxx"
#include "IkLsPause.hxx"
#include "IkLsPauseAck.hxx"
#include "IkLsStart.hxx"
#include "IkLsStartAck.hxx"
#include "IkLsStop.hxx"
#include "IkLsStopAck.hxx"
#include "IkLsCont.hxx"
#include "IkLsContAck.hxx"
#include "IkLsParam.hxx"
#include "IkLsParamAck.hxx"
#include "IkLsOS9.hxx"
#include "IkLsOS9Ack.hxx"
#include "IkLsGenMess.hxx"
#include "IkLsRecord.hxx"
#include "IkLsLog.hxx"
#include "IkLsLogReq.hxx"
#include "IkLsDownloadAck.hxx"
#include "IkLsDownloadCheck.hxx"

#include "IkPmMessage.hxx"

// Database Managment
#include "DbC.h"
#include "DbCArray.h"

// Generic macros
typedef unsigned char byte;         // 1 byte 
typedef unsigned short int word;    // 2 bytes
typedef unsigned int longWord; // 4 bytes
typedef signed int sLongWord;  // 4 bytes

#endif
