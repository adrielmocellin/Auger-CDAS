#ifndef _IKClient_
#define _IKClient_

#include "IkMessage.h"

#ifdef _MAIN_IKCLIENT_H_
char *gAppName;
#else
extern char *gAppName;
#endif

int IkJustConnect(char *ip, char *name);
int IkJustRequest(char *name, char *type=NULL, int any=0, void (*f)(IkMessage *ikm)=NULL);
void IkConnect(char *ip,char *name,char *type=NULL,int any=0,void (*f)(IkMessage *ikm)=NULL);
void IkMessageCheck(int verbose=0);
void IkMessageSend(IkMessage &ikm);

#endif
