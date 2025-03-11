#include "IkMessage.h"
#include <IkCTypes.h>

int IkMessage::get_type() {
  return IkFindType(Type);
}

#include <IkClient.h>
int IkMessage::send(const char *dest) {
  IkMessageSend(*this);
  return (dest!=NULL);
}
