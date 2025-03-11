/*! \file
    \brief Interaction with Ik: sending T3 and receving T3 request
*/
#include "IkC.h"
#include "IkCtRequest.hxx"
#include "IkT3.hxx"
#include "XbT2.h"
#include "XbIk.h"
#include <cstdio>
#include <cstdlib>


int t3id=-1;
int Gt3idmin=-1;
int Gt3idmax=-1;
char *Gfilename=NULL;

void XbIk(IkMessage *ikm) {
  switch (ikm->get_type()) {
  case IKCTREQUEST:
    observer=((IkCtRequest*)ikm)->StationId;
    IkInfoSend("Received Request for tank %d",observer);
    break;
  default:
    IkWarningSend("Received Unknown IkMessage:%s",ikm->to_text().c_str());
    break;
  }
}

#include "XbArray.h"

// T3ID:
//      0 -> 4090 for SD
//      4096 -> 8192 for FD
//      8200 -> 9000 for XbFast
//      40000 -> for Eb (in the re-request of T3)
//

unsigned short XbIkGetId() {
  t3id++;
  if (t3id>=Gt3idmax) t3id=Gt3idmin;
  if (t3id%5==0) {
    FILE*f=fopen(Gfilename,"w");
    if (f==NULL) {
      IkFatalSend("Cannot open %s for writing",Gfilename);
      exit(1);
    }
    fprintf(f,"%d\n",t3id);
    fclose(f);
  }
  return t3id;
}

void XbIkInitId(char*filename, int t3idmin, int t3idmax) {
  Gt3idmin=t3idmin;
  Gt3idmax=t3idmax;
  Gfilename=filename;
  if (t3id==-1) {
    FILE*f=fopen(Gfilename,"r");
    if (f==NULL) t3id=Gt3idmin;
    else {
      fscanf(f,"%d",&t3id);
      fclose(f);
    }
    t3id+=10;
    if (t3id>=Gt3idmax) t3id=Gt3idmin;
  }
}

void XbIkT3Emit(t3 in) {
  IkT3 ikm;
  ikm.id=XbIkGetId();
  ikm.refSecond=in.refSecond;
  ikm.refuSecond=in.refuSecond;
  ikm.mode="LIST";
  ikm.algo=in.algo;
  ikm.SDPAngle=0.;
  for(unsigned int i=0; i<in.point.size(); i++) {
    ikm.addresses.push_back(in.point[i].ID);
    ikm.offsets.push_back(in.point[i].offset);
    ikm.window.push_back(in.point[i].window);
  }
  ikm.send("Pm");
}
