#ifndef _CALMON_PACK_H_
#define _CALMON_PACK_H_

#include "msg_pack_unpack.h" //defined in Utils/CL

#define CALMON_PACK_MAX 7168 

enum ecalmon {
      CALMON_PACK_MONIT=1,
      CALMON_PACK_CALIB,
      CALMON_PACK_RD,
      CALMON_PACK_SYSTEM,
      CALMON_PACK_EXTRA,
      CALMON_PACK_LAST
};

#endif
