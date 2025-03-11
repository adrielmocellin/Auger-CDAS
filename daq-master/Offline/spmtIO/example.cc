
#include "spmtIO.h"
#include <TFile.h>
#include <TTree.h>
using namespace std;
int main(int argc,char *argv[])
{
  spmtIO *dat;
  TFile *f;
  int i;
  int j,nEntries;
  //dat=new spmtIO;
  dat=NULL;
  for(i=1;i<argc;i++){
    f=new TFile(argv[i]);
    if(! f->IsZombie()){
      TTree* t = (TTree*)f->Get("spmt_monit");
      t->SetBranchAddress("data",&dat);
      nEntries = t->GetEntries();
      cout << "======= file: "<< argv[i]
           << " ======= Entries: "<< nEntries
           << " ======" << endl;
      for(j=0;j<nEntries;j++){
        t->GetEntry(j);
        cout << dat->LsId <<endl;
        //<< dat->muon.size() << " "
        //     << dat->thres.size() << endl;
        cout << " Events: " <<dat->evt.size() <<endl;
        for(int k=0;k<dat->evt.size();k++){
          cout << dat->evt[k].GPSsec
               << " " << dat->evt[k].GPSTicks
               << " ch: " << dat->evt[k].charge_FADC[0]
               << " " << dat->evt[k].charge_FADC[1]
               << " " << dat->evt[k].charge_FADC[2]
               << " " << dat->evt[k].charge_FADC[3]
               << " " << dat->evt[k].charge_FADC[4]
               << " " << dat->evt[k].charge_FADC[5]
               << " pk: " << dat->evt[k].peak_FADC[0]
               << " " << dat->evt[k].peak_FADC[1]
               << " " << dat->evt[k].peak_FADC[2]
               << " " << dat->evt[k].peak_FADC[3]
               << " " << dat->evt[k].peak_FADC[4]
               << " " << dat->evt[k].peak_FADC[5]
               << " st: " << dat->evt[k].saturation[0]
               << " " << dat->evt[k].saturation[1]
               << " " << dat->evt[k].saturation[2]
               << " " << dat->evt[k].saturation[3]
               << " " << dat->evt[k].saturation[4]
               << " " << dat->evt[k].saturation[5]
               <<endl;
        }
      }
      delete(t);
      dat=NULL;
    }
    delete(f);
  }
}
