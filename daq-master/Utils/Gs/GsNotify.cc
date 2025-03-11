/*--------------------------------------------------------------------------------*/
/*
  GsNotify.cc
  ...........just the GsNotify routines;
*/
/*--------------------------------------------------------------------------------*/

//#include <stdio.h>
//#include <stdarg.h>

#include "Gs.h"
#include "GsNotify.h"
#include <GsThreads.h>  // TO BE CHECKED:to protect against concurent access!
#include <IkInfo.hxx> 
#include <IkSevere.hxx> 
#include <IkWarning.hxx> 
#include <IkFatal.hxx>
#include <stdarg.h>

char GsIkTestSuffix[IKMAXSUFFIX+1] = ""; /* should be enough */

/* when Ik is not there, output to stdout */

#ifdef AVIRER // for info messages, IkC handles that !
void GsNotifyV0Stdout(const char * dest,int type, const char *fmt,...) 
  /* ala v0r3 routine*/
{
  va_list ap;
  va_start(ap,fmt);
  printf("%s%s"IKPIPESEP,dest,GsIkTestSuffix);
  printf("%d"IKPIPESEP,type);
  printf(IKTEXTFIELDSTART);
  vprintf(fmt,ap);
  printf(IKTEXTFIELDEND);
  va_end(ap);
}

int GsNotifyV2Stdout(const char * dest, IkMessageP mess)
{
  printf("%s%s"IKPIPESEP,dest,GsIkTestSuffix);
  printf("%d"IKPIPESEP,mess->get_type());
  printf(IKTEXTFIELDSTART);
  printf("%s",mess->to_string().c_str());
  printf(IKTEXTFIELDEND);
  return CDAS_SUCCESS;
}

#endif

static inline void GsNotifySetString (string *mess, char * source)
{
  // TO BE CHECKED, syntax from IkC, suspicious char[]->string ?
  *mess = source; 
}

/* old GsNotifyIk , ala v0r3 routine */
void GsNotifyIk(const char * dest,int type, const char *fmt,...)
{
#define MAXMSGSIZE 2048
    va_list ap;
    int n;
    va_start(ap,fmt);
    char tmp[MAXMSGSIZE + 2];
    n=vsnprintf(tmp,MAXMSGSIZE,fmt,ap);
    va_end(ap);
    GsGetAccessTo(&GsIkLocker);
    if(n<0) IkWarningSend ("GsNotifyIk : next message is truncated");
  switch (type)
  {
  case IK_INFO :
    {
      IkInfo mess;
      GsNotifySetString(&mess.msg,tmp);
      mess.send(dest);
    }
      break;
  case IK_WARNING :
    {
    IkWarning mess;
    GsNotifySetString(&mess.msg,tmp);
    mess.send(dest);
    }
    break;
  case IK_SEVERE :
    {
    IkSevere mess;
    GsNotifySetString(&mess.msg,tmp);
    mess.send(dest);
    }
    break;
  case IK_FATAL :
    {
    IkFatal mess;
    GsNotifySetString(&mess.msg,tmp);
    mess.send(dest);
    }
    break;
  default :
    IkSevereSend("Message is in old Format: thrown away");
    break;
  }
    GsReleaseAccessTo(&GsIkLocker);
}

int GsNotify(const char * dest, IkMessageP mess)
{
  return(mess->send((char *)dest));
}

