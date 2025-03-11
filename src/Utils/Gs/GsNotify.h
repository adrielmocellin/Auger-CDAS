/*--------------------------------------------------------------------------------*/
/*
  GsNotify.h
  ...........declarations of wrappers for stdin/stdout or Ik messages
  (Should be renamed "GrsSIk" ? )
*/
/*--------------------------------------------------------------------------------*/
#ifndef __GSNOTIFY__
#define __GSNOTIFY__

#include "GsIk.h"
#include <UiMessage.h>


#define IKPIPESEP " "
#define IKRECEIVESEP " "
#define IKTEXTFIELDSTART "'"
#define IKTEXTFIELDEND "'\n"

/*************************/
/* outputs */
/* old GsNotifyIk, ala v0r3 routine*/
void GsNotifyIk(const char * dest,int type, const char *fmt,...);
/* new one (formated Ik Messages) */
int GsNotify(const char * dest, IkMessageP mess);


#endif




