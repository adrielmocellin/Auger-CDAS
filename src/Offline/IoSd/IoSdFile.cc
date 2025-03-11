#include "IoSdFile.h"
#include <cstdlib>

IoSdKey::IoSdKey(TKey * k, int filenumber) {
  const char *name = k->GetName();
  Name = name;
  ClassName = k->GetClassName();
  FileNumber = filenumber;
  if ((ClassName == "AugerEvent") && !strstr(name, "#Fd-only")) {
    EventId id;
    unsigned int sdmul, fdmul, npix, fdpatt,gps;
    char algo[30];
    string algo2;
    sscanf(name, "%llu#%u#%u#%u#%u#%s", &id, &sdmul, &fdmul, &npix,
           &fdpatt,algo);
    EvtId = id;
    Multiplicity = sdmul;
      T3Algo= strchr(algo,'#')+1;
    algo2= strchr(algo,'#') + 1;
    std::size_t pos=algo2.find('#');
    if(pos==string::npos ){
      T3Algo=algo2;
    }
    else{
    T3Algo=algo2.substr(0,pos);
    gps=atoi (strchr(algo2.c_str(),'#')+1);
    EvtGps=(unsigned int)gps;
    algo2= strchr(algo, '#') + 1;
   }
    return;
  }

  if ((ClassName == "IoSdEvent") && strchr(name, '#')) {
    //name should have the format
    //  "AUGERID#MULTIPLICITY#ALGORITHMS#GPSSECOND#UUB_MULTIPLICIT"
    //
    // It will set the parameters:
    // EvtId = AUGERID - event identification
    //
    // Multiplicity = MULTIPLICITY - number of stations with "error"=0
    //               (working only with UB)
    //
    // T3Algo = ALGORITHMS - name of the trigger
    //
    // EvtGps = GPSSECOND - time of the event (gps second)
    //
    // sum to Multiplicity the value UUB_MULTIPLICIT - number of
    //                stations with error=256 (stations with UUB and
    //                report T3 with traces).

    EventId id;
    unsigned int mul,gps;
    char algo[20];
    string algo2;
    sscanf(name, "%llu#%u#%s", &id, &mul,algo);
    EvtId = id;
    Multiplicity = mul;
    T3Algo=algo;
    algo2=algo;
    std::size_t pos=algo2.find("#");
    if(pos==string::npos)
      T3Algo=algo2;
    else {
      T3Algo=algo2.substr(0,pos);
      gps=atoi(algo2.substr(pos+1).c_str());
      EvtGps=(unsigned int)gps;
      unsigned int uub_mul=0;
      pos=algo2.find("#",pos+1);
      //if(pos != std::string::npos){
      //  uub_mul=atoi(algo2.substr(pos+1).c_str());
      //  Multiplicity+=uub_mul;
      //}
    }
    return;
  }
};

// ---------------------------------------------------------
IoSdFile::IoSdFile(const char *name) {
  Nentries = 0;
  FileName = name;
  if (!(File = new TFile(FileName.c_str(), "READ")) || File->IsZombie()) {
    cerr << "Impossible to open file " << FileName << endl;
    return;
  }
  Nentries = File->GetNkeys();
};

// ---------------------------------------------------------
IoSdFile::~IoSdFile() {
  if (File) {
    File->Delete("*;*");
    File->Close();
    delete File;
  }
};

// ---------------------------------------------------------
IoSdFile::IoSdFile(const IoSdFile & f) {
  File = NULL;
  FileName = f.FileName;
  Nentries = f.Nentries;
};
