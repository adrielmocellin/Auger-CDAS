#include "TROOT.h"
#include "TServerSocket.h"
#include "TSocket.h"
#include "TClass.h"
#include "TMessage.h"

#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "T3Notify.h"


TROOT TRoot ("Fdc","FD client");

int main(int argc, char* argv[]) {
  // Open connection to server
  TSocket *sock = new TSocket("localhost", 10500);


  TMessage mess(kMESS_OBJECT);
  //TMessage mess(kMESS_OBJECT | kMESS_ACK);

  T3Notify t3fd;
  if (argc==2) t3fd.SetEyeId(atoi(argv[1]));
  for (int i = 0; i < 250; i++) {
    time_t t=time(NULL);
    t3fd.SetT3Time(t-315964800+14,179114966);
    t3fd.SetT3Id(4096+i);
    t3fd.SetSDPAngle(1.57);//M_PI*rand()*1./(RAND_MAX+1.));
    mess.Reset();              // re-use TMessage object
    mess.WriteObject(&t3fd);     // write object in message buffer
    sock->Send(mess);          // send message
    sleep(5);
  }
  // Close the socket
  sock->Close();
  return 0;
}
