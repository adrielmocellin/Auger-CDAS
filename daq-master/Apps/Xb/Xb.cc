/*! \file
    \brief Main program
*/

#include "IkC.h"
#include "XbIk.h"
#include "XbPm.h"
#include "XbArray.h"
#include "XbAlgo.h"
#include <cstdlib>
#include <cstdio>


#include "UiCmdLine.h"

int newt2config=1,verbose=1,debug=0,trivial_algo=1800,carmen_miranda=600;

int main(int argc, char* argv[]) {
  struct t2 input[NT2MAX];

  /* Reading command line */
  UiParseCommandLine(argc,argv);
  if (argc==1) {
    //PmAddr="192.168.1.110";
    PmAddr="pmx.cdas.augernet";
    PmPort=10600;
  }

  /* Initialisation to CDAS  */
  CDASInit("Xb");

  /* Asking for Ik Messages */
  IkMonitor(XbIk,"destination is \"Xb\" or type is %d",IKSDREQUEST);

  XbArrayInit(1);

  /* Connect to Pm */
  if (! XbPmConnect("Xb")) {
    stringstream s;
    s << "Cannot connect to Pm.";
    InternalLog(s.str().c_str(),IKFATAL);
    exit(1);
  }

  int nt2=0;
  XbAlgoT2Init();
  XbIkInitId();
  XbPmSendStatus(LOG);

  while (1) {
    //  Listen to IkMessages.
    IkMessageCheck();
    nt2=XbPmGetT2(input);
    if (verbose) {
      stringstream s;
      s << nt2 << " T2 from Pm.";
      InternalLog(s.str().c_str(),IKDEBUG);
    }
    vector<t2list> out=XbAlgoT2Process(input,nt2);

    while (out.size()>0) {
      XbIkT3Emit(XbAlgoT3Build(out[out.size()-1]));
      XbAlgoFree(out[out.size()-1]);
      out.pop_back();
    }
  }
  return 0;
}
