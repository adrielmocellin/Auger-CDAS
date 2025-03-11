#include <iostream>

#ifndef DPA
#include "IkC.h"
#include "IkSuPing.hxx"
#include "IkSuPong.hxx"
#endif

#ifndef DPA
int HandleMessage(IkMessage* message) {
  switch(message->get_type()) {
  case IKSUPING:
    IkSuPong mpong;
    mpong.PongValue = ((IkSuPing*)message)->PingValue;
    mpong.send(message->get_sender());
    break;
  }
  return 0;
}
#endif

void SendMessage(string message, int flag) {

  SendMessage(message);

#ifndef DPA
  switch (flag) {
    case EIkInfo: IkInfoSend(message.c_str()); break;
    case EIkWarning: IkWarningSend(message.c_str()); break;
    case EIkSevere: IkSevereSend(message.c_str()); break;
    case EIkFatal: IkFatalSend(message.c_str()); break;
    case EIkSpeech: IkSpeechSend(message.c_str()); break;
  }
#endif
}

void SendMessage(string message) {
  cout << message << endl;
}

