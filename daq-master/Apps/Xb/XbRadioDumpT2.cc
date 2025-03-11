#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstdio>

#define MODE (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)

#include "IkC.h"
#include "DbC.h"
#include "DbCConfig.h"
#include "XbIk.h"
#include "XbPm.h"
#include "XbArray.h"
#include "XbAlgo.h"
#include "XbT2.h"
#include "Xb.h"

#include "UiCmdLine.h"

string PmName,EbName,name;
int newt2config=1,verbose=0,debug=0,trivial_algo=180,carmen_miranda=300;
char *ArrayFile=NULL;

void DumpT2(int refSecond, int nT2, struct t2 *input) {
  //static FILE *f=fopen("/Raid/tmp/T2dump","w");
  //fwrite((void *) &refSecond, sizeof(refSecond), 1, f);
  //fwrite((void *) &nT2, sizeof(nT2), 1,  f);
  //fwrite((void *) input, sizeof(struct t2), nT2, f);
  time_t t = refSecond;
  t += 315964800;
  struct tm * stm=gmtime(&t);
  stringstream dirnm;
  stringstream fileend;
  stringstream s1;
  dirnm << "/Raid/tmp/XbDumpRadio/";
  mkdir(dirnm.str().c_str(),MODE);
  dirnm << stm->tm_year+1900 << "/";
  mkdir(dirnm.str().c_str(),MODE);
  if (stm->tm_mon<9) dirnm << "0";
  dirnm << stm->tm_mon+1 << "/";
  mkdir(dirnm.str().c_str(),MODE);
  fileend << "_" << stm->tm_year+1900 << "_";
  if (stm->tm_mon<9) fileend << "0";
  fileend << stm->tm_mon+1 << "_";
  if (stm->tm_mday<10) fileend << "0";
  fileend << stm->tm_mday << "_00h00.dat";
  s1 << dirnm.str() << "radio_t2" << fileend.str();
  FILE *f=fopen(s1.str().c_str(),"a");
  for (int i=0;i<nT2;i++)
    if (input[i].ID==942 || input[i].ID==1325 || input[i].ID==1351 || input[i].ID==1764) fprintf(f,"%d %d %d %d\n",refSecond,input[i].ID,input[i].time,input[i].energy);
  fclose(f);
}


int main(int argc, char* argv[]) {
  struct t2 input[NT2MAX];

  /* Reading command line */
  UiParseCommandLine(argc,argv);
  //PmAddr="192.168.1.110";
  PmAddr="pmx.cdas.augernet";
  PmPort=10600;

  /* Initialisation to CDAS  */
  name="XbRadioDump";
  PmName=PM;
  EbName=EB;
  if (TaskIkSuffix) {
    name+=TaskIkSuffix;
    PmName+=TaskIkSuffix;
    EbName+=TaskIkSuffix;
  }
  CDASInit(name.c_str());

  /* Asking for Ik Messages */
  IkMonitor(XbIk,"destination is \"%s\" or type is %d",name.c_str(),IKSDREQUEST);

  XbArrayInit(1);

  /* Connect to Pm */
  if (! XbPmConnect("XbRadioDump")) {
    cerr << "Cannot connect to Pm." << endl;
    exit(1);
  }

  int nt2=0;
  XbPmSendStatus(LOG);

  while (1) {
    //  Listen to IkMessages.
    IkMessageCheck();
    nt2=XbPmGetT2(input,0);
    if (verbose) cerr << "At second " << second_ << " received "<< nt2 << " T2 from Pm." << endl;
    DumpT2(second_, nt2, input);
  }
  return 0;
}
