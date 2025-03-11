#ifndef _MrS_MESS_H_
#define _MrS_MESS_H_

#ifndef DPA
#include "IkC.h"

enum EIkFlag {
  EIkInfo, EIkWarning, EIkSevere, EIkFatal,
  EIkSpeech
};

int HandleMessage(IkMessage* message);
#endif
void SendMessage(string);
void SendMessage(string,EIkFlag);
#endif
