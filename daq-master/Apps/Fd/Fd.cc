#include <unistd.h>
#include <list>

#include "IkC.h"
#include "DbC.h"
#include "DbCArray.h"

#include "Fd.h"

#include "UiCmdLine.h"

#define FD "Fd"

#include "TROOT.h"
#include "TServerSocket.h"
#include "TSocket.h"
#include "TClass.h"
#include "TMessage.h"

#include "T3Notify.h"
#include "IoSdData.h"

TROOT TRoot ("Fd","FD server");

string PmName;
int debug=1,verbose=1,use_phi=0;

int main(int argc, char* argv[]) {
  TMessage * mess=NULL;
  list<TSocket *> soklis;

  /* Reading command line */
  UiParseCommandLine(argc,argv);

  /* Initialisation to CDAS  */
  {
    string name;
    name=FD;
    PmName=PM;
    if (TaskIkSuffix) {
      name+=TaskIkSuffix;
      PmName+=TaskIkSuffix;
    }
    CDASInit(name.c_str());
  }

  TServerSocket *ss=new TServerSocket(10500,kTRUE);

  if (! ss->IsValid() || ss->GetLocalPort()!=10500) {
    IkFatalSend("Cannot bind or even stranger thing from ROOT");
    return(1);
  }

  if (verbose) {
    cout << "  =====================" << endl;
    cout << "  FD production version" << endl;
    cout << "  =====================" << endl;
    cout << "Has Following buildings:" << endl;
    for (int i=1;i<6;i++) cout << "Site " << i << " " << kIoSd::FdX(i) << " " << kIoSd::FdY(i) << " " << kIoSd::FdZ(i) << endl;
    cout << "Statistical Veto using:" << endl;
    cout << " * Physics rate: 1 event per " << 1./PHYSRATE << " seconds" << endl;
    cout << " * Deadtime " << 100*DEADTIME << "\%" << endl;
    cout << "Lidar timing veto used with 0.5\% deadtime" << endl;
    cout << "SDP at ground never used" << endl;
    cout << "FD building altitude not used" << endl;
    cout << "Time window of " << WINDOW << " useconds used" << endl;
    stringstream s;
    s << "Fd running with sites: ";
    for (int i=1;i<6;i++) s << i << " " << kIoSd::FdX(i) << " " << kIoSd::FdY(i) << " " << kIoSd::FdZ(i) << "; ";
    s << " Parameters: " << 1./PHYSRATE << " " << 100*DEADTIME << " " << WINDOW;
    InternalLog(s.str().c_str(),IKINFO);
  }

  while (1) {
    ss->SetOption(kNoBlock,0);  // Blocking mode until we get one connection
    TSocket *socket=ss->Accept();
    if (verbose) {
      stringstream s;
      s << "First client...";
      InternalLog(s.str().c_str(),IKINFO);
    }
    IkInfoSend("First client from FD");
    /* Building array */
    FdTriggerArrayInit();
    ss->SetOption(kNoBlock,1);  // Non blocking mode afterwards
    socket->SetOption(kNoBlock,1);  // Non blocking mode for all sockets
    soklis.push_back(socket);

    // Reading info from fluorescence ? Or wait for signal ?
    while(1) {
      // Trying to get another eventual client...
      //IkMessageCheck();
      socket=ss->Accept();
      usleep(50000);
      if (socket && (long int)socket!=-1) {  // ROOT is so awfull... giving -1 as an address, this becomes architecture dependant...
        soklis.push_back(socket);
        socket->SetOption(kNoBlock,1);  // Non blocking mode for all sockets
        stringstream s;
        s << "New client from FD, " << soklis.size() << " total";
        IkInfoSend(s.str().c_str());
        InternalLog(s.str().c_str(),IKINFO);
        if (soklis.size()==1) {
	  stringstream s;
          s << "One client, reloading Db";
          InternalLog(s.str().c_str(),IKINFO);
          FdTriggerArrayInit();
        }
      }
      list<TSocket*>::iterator so;
      for (so=soklis.begin();so!=soklis.end();so++) {
        int read=(*so)->Recv(mess);
        if (mess!=NULL && mess->What() == kMESS_OBJECT && strcmp(mess->GetClass()->GetName(),"T3Notify")==0) {
          T3Notify *t3 = (T3Notify *)mess->ReadObject(mess->GetClass());
          stringstream s;
          if (debug) t3->Dump(s);
          InternalLog(s.str().c_str(),IKINFO);
          if (IsValidT3(t3))
            FdTriggerT3Emit(t3->GetEyeId(),t3->GetT3Id(),t3->GetT3GPSTime(),t3->GetT3GPSNanoTime()/1000,t3->GetSDPAngle());
          delete t3;
          delete mess;
          mess=NULL;
        } else if (read!=-4) {   // read == -4 means no message but socket Ok. So nice conventions...
          if (mess!=NULL && mess->What() == kMESS_OBJECT) {
            stringstream s;
            s << "Unknown object received, closing socket";
            IkWarningSend(s.str().c_str());
            if (verbose) InternalLog(s.str().c_str(),IKWARNING);
            delete mess;
          } else if (mess!=NULL) {
            stringstream s;
            s << "Unknown message received, closing socket";
            IkWarningSend(s.str().c_str());
            if (verbose) InternalLog(s.str().c_str(),IKWARNING);
            delete mess;
          } else {
            stringstream s;
            s << "Connection closed";
            IkWarningSend(s.str().c_str());
            if (verbose) InternalLog(s.str().c_str(),IKINFO);
          }
          (*so)->Close();
          soklis.remove(*so);
          break;
        }
      }
    }
  }
  return 0;
}
