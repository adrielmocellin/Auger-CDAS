#include "IkDebug.hxx"
#include "IkInfo.hxx"
#include "IkWarning.hxx"
#include "IkSevere.hxx"
#include "IkFatal.hxx"
#include "IkSound.hxx"
#include "IkSpeech.hxx"

#include "IkC.h"
#include <cstdarg>
#include <cstdio>

#define __bufSize__ 1024

int IkDebugSend(const char *fmt,...)
{va_list ap; char buf[__bufSize__ + 2]; int n; 
IkDebug debug;
va_start(ap, fmt); n = vsnprintf (buf, __bufSize__, fmt, ap); va_end(ap);
debug.msg = buf;
if (n < 0 || n > __bufSize__) 
  {
  debug.send(LOG);
  return  IkWarningSend("IkDebugSend : Could not build complete message");
  }
else return debug.send(LOG);
}

int IkInfoSend(const char *fmt,...)
{va_list ap; char buf[__bufSize__ + 2]; int n; IkInfo info;
va_start(ap, fmt); n = vsnprintf (buf, __bufSize__, fmt, ap); va_end(ap);
info.msg = buf;
if (n < 0 || n > __bufSize__) 
  {
  info.send(LOG);
  return  IkWarningSend("IkInfoSend : Could not build complete message");
  }
else return info.send(LOG);
}

              
int IkWarningSend(const char *fmt,...)
{va_list ap; char buf[__bufSize__ + 2]; int n; IkWarning warning;
va_start(ap, fmt); n = vsnprintf (buf, __bufSize__, fmt, ap); va_end(ap);
warning.msg = buf;
if (n < 0 || n > __bufSize__) 
  {
  warning.send(LOG);
  return  IkWarningSend("IkWarningSend : Could not build complete message");
  }
else return warning.send(LOG);
}

int IkSevereSend(const char *fmt,...)
{va_list ap; char buf[__bufSize__ + 2]; int n; IkSevere severe;
va_start(ap, fmt); n = vsnprintf (buf, __bufSize__, fmt, ap); va_end(ap);
severe.msg = buf;
if (n < 0 || n > __bufSize__) 
  {
  severe.send(LOG);
  return  IkWarningSend("IkSevereSend : Could not build complete message");
  }
else return severe.send(LOG);
}

int IkFatalSend(const char *fmt,...)
{va_list ap; char buf[__bufSize__ + 2]; int n; IkFatal fatal;
va_start(ap, fmt); n = vsnprintf (buf, __bufSize__, fmt, ap); va_end(ap);
fatal.msg = buf;
if (n < 0 || n > __bufSize__) 
  {
  fatal.send(LOG);
  return  IkWarningSend("IkFatalSend : Could not build complete message");
  }
else return fatal.send(LOG);
}


int IkSoundSend(const char *file, int volume)
{IkSound sound; char buf[__bufSize__ + 2], *nameP; int n = 0;
 if (!strchr(file,'/')) 
   {
   n = snprintf (buf, __bufSize__, "/Raid/opt/sound/%s", file);
   nameP = buf;
   }
 else nameP = (char *) file;
 if (n < 0 || n > __bufSize__) 
  {
  return  IkWarningSend("IkSoundSend : Could not build sound file name");
  }
sound.file = nameP;
sound.volume = volume;
return  sound.send("IkSound");
}

int IkSpeechSend(const char *fmt,...)
{va_list ap; char buf[__bufSize__ + 2]; int n; IkSpeech speech;
va_start(ap, fmt); n = vsnprintf (buf, __bufSize__, fmt, ap); va_end(ap);
speech.msg = buf;
if (n < 0 || n > __bufSize__)
  {
  speech.send("IkSpeech");
  return  IkWarningSend("IkSpeechSend : Could not build complete message");
  }
else return speech.send("IkSpeech");
}                  
