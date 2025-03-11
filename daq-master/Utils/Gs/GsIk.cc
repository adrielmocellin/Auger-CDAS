/*--------------------------------------------------------------------------------*/
/*
  GsIk.c
  ...........wrappers for stdin/stdout or Ik messages
*/
/*--------------------------------------------------------------------------------*/

#include <Gs.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <UiMessage.h>

extern char GsIkTestSuffix[IKMAXSUFFIX+1];

/**************************/
/* pointer to a routine in the server to process inputs */
int (*ServerIkMessageHandler)(char * from, 
			      char * to, 
			      int type, 
			      char * data);
/***************************/
/* temporary (?) miscellanea */

#define tokenSize 100

struct tokens
{
  char source[tokenSize];
  char dest[tokenSize];
  int type;
  char * data;
} GsIkTokens;


/***************************/
/* input */

void GsProcessIkMessage(char * mess)
{
  int type;
  char * source;
  char textField[100];
  char * util;

  if (!mess) return (CDAS_INVALID);
  GsIkMessageParse(mess);
  source = GsIkMessageSource(mess);
  if (!strstr(source,FROMSU))
  {
    if (ServerIkMessageHandler)
      return (*ServerIkMessageHandler)(GsIkMessageSource(mess),
				       GsIkMessageDest(mess),
				       GsIkMessageType(mess),
				       GsIkMessageData(mess));
    else GsNotifyIk(TOLOG,IK_WARNING,"No server handler for Ik messages");
  }
  GsNotifyIk(TOLOG,IK_INFO,
		   "message was %s",mess);
  switch (type=GsIkMessageType(mess))
  {
  case SUKILL :  
    /* for test purposes */
    GsNotifyIk(TOLOG,IK_INFO,"data part was :%s",GsIkMessageData(mess));
/* TO BE DONE : cleanup function */
    GsTerminate(EXIT_SUCCESS);
    break;
  case SUPING :
    /* idea : launch a client to a service that emit SUPONG */
    /* in the meanwhile : */
    strncpy(textField,GsIkMessageData(mess),99);textField[99]=0;
    if(strlen(SUPONGSTR)>strlen(textField))
      strcpy(textField,SUPONGSTR);
    else
      {
	util=strstr(textField,SUPINGSTR);if (!util) util=textField; 
	memcpy(util,SUPONGSTR,strlen(SUPONGSTR));
      }
    GsNotifyIk(TOSU,SUPONG,"%s",textField); 
    GsNotifyIk(TOLOG,SUPONG,"%s",textField);
break;
  default :
    if (!strcmp(mess,"stop")) GsTerminate(EXIT_SUCCESS);
    GsNotifyIk(TOLOG,IKSEVERE,"unknown IkMessage : %s",mess);
    return(CDAS_MSG_TYPE_ERROR);
    break;
  }
}

void GsIkMessageParse(char * mess)
{
  int usedChars;
  usedChars = 0;
  GsIkTokens.data = NULL;
  /* warning, not robust */
  if (sscanf(mess,"%99s"IKRECEIVESEP"%99s"IKRECEIVESEP"%d"IKRECEIVESEP"%n", 
	     GsIkTokens.source,GsIkTokens.dest,&GsIkTokens.type,&usedChars) < 3)
    {
      GsIkTokens.source[0] = GsIkTokens.dest[0] = 0;
      GsIkTokens.type = CDAS_INVALID;
      GsIkTokens.data = mess;
      GsNotifyIk(TOLOG,IKSEVERE,"unable to processIkMessage : %s",mess);
      return;
    }
  GsIkTokens.data = mess+usedChars;
}

char * GsIkMessageSource(char * mess)
{
  int len, suffLen, done = 0;
  char *p_str, *p_subStr;

  if (GsIkTestSuffix[0] != '\0') {
    /* remove any Iksuffix from the source field) */
    suffLen = strlen(GsIkTestSuffix);
    len = strlen(GsIkTokens.source);
    p_str = GsIkTokens.source;
    while (done == 0) {
      p_subStr = strstr(p_str, GsIkTestSuffix);
      if (p_subStr == NULL) {
	/* we reached the end of the string and did not find it. */
	break;
      } else {
	/* we found it, if it is not at the end of the string, then it is not
	   the suffix, try again */
	if ((p_subStr[suffLen]) == '\0') {
	  /* we found the suffix, remove it from the string */
	  p_subStr[0] = '\0';
	  done = 1;
	} else {
	  /* it is not at the end of the string, start after here and look
	     again */
	  p_str = &(p_subStr[suffLen]);
	}
      }
    }
  }
  return(GsIkTokens.source);
}
char * GsIkMessageDest(char * mess)
{
  return(GsIkTokens.dest);
}
int GsIkMessageType(char * mess)
{
  return(GsIkTokens.type);
}
char * GsIkMessageData(char * mess)
{
  return(GsIkTokens.data);
}



















