#include "IoSdRootI.h"

//#### Default reading mechanism, i.e. "a la IoSd" ####### //

static IoSdEvent *IoSdReader(TFile * f, IoSdKey & key) {
  IoSdEvent *tmp;
  tmp = (IoSdEvent *) f->Get(key.Name.c_str());
  if (!tmp)
    return NULL;
  counted_ptr < IoSdEvent > p(tmp);
  tmp->_rawEvent = p;
  return tmp;
}


//######################### SINGLETON #########################//
IoSdRootI *IoSdRootI::instance = new IoSdRootI();

IoSdRootI *IoSdRootI::Instance() {
  static int firstCall = 1;
  if (firstCall) {
    firstCall = 0;
    instance->_Reader = IoSdReader;
  }
  return instance;
}

IoSdRootI *theRootInterface() {
  return IoSdRootI::Instance();
}

//#############################################################//

static bool _GoodKey(TKey * key) {
  return (strstr(key->GetClassName(), "IoSdEvent") ||
          (strstr(key->GetClassName(), "AugerEvent")
           && !strstr(key->GetName(), "#Fd-only")));
}

// ---------------------------------------------------------
void IoSdRootI::Input(const char *fileName) {
  cerr << fileName << endl;
  IoSdFile file(fileName);
  if (!(file.File) || file.File->IsZombie())
    return;
  Files.push_back(file);
  //NtotEntries += file.Nentries;      // Not more valid since we have
  // "bad keys" for the header
   TIter next(file.File->GetListOfKeys());
  TKey *ktmp;
  while ((ktmp = (TKey *) next()))
    if (_GoodKey(ktmp)) {
       IoSdKey tofill(ktmp, Files.size() - 1);
        Keys.push_back(tofill);
      NtotEntries++;
    }
}

// ---------------------------------------------------------
void IoSdRootI::DeleteFile(int file) {
  if (file != -1) {
    Files[file].File->Delete("*;*");
    Files[file].File->Close();
    delete Files[file].File;
    Files[file].File = NULL;
  }
}

// ---------------------------------------------------------
void IoSdRootI::Reset() {
  DeleteFile(CurrentFileId);
  Files.clear();
  Keys.clear();
  NtotEntries = 0;
  CurrentFileId = -1;
}

// ---------------------------------------------------------
const char *IoSdRootI::CurrentFileName() {
  if (CurrentFileId != -1)
    return Files[CurrentFileId].FileName.c_str();
  return NULL;
}

// ---------------------------------------------------------
IoSdEvent *IoSdRootI::ReadEventFromFile(IoSdKey & key) {
  if (key.FileNumber != CurrentFileId) {
    DeleteFile(CurrentFileId);
    CurrentFileId = key.FileNumber;
    Files[CurrentFileId].File =
      new TFile(Files[CurrentFileId].FileName.c_str(), "READ");
  }
  //  return (IoSdEvent *)Files[CurrentFileId].File->Get(key.Name.c_str());
  IoSdEvent *tmp = _Reader(Files[CurrentFileId].File, key);
  if (!tmp)
    return NULL;
  tmp->AugerId = key.EvtId;
  tmp->RootClassName = key.ClassName;
  return tmp;
}
IoSdEvent *IoSdRootI::ReadEventFromFileGps(IoSdKey & key) {
  if (key.FileNumber != CurrentFileId) {
    DeleteFile(CurrentFileId);
    CurrentFileId = key.FileNumber;
    Files[CurrentFileId].File =
      new TFile(Files[CurrentFileId].FileName.c_str(), "READ");
  }
  //  return (IoSdEvent *)Files[CurrentFileId].File->Get(key.Name.c_str());
  IoSdEvent *tmp = _Reader(Files[CurrentFileId].File, key);
  if (!tmp)
    return NULL;
  tmp->AugerId = key.EvtId;
  tmp->Trigger.Second = key.EvtGps;
  tmp->RootClassName = key.ClassName;
  return tmp;
}

// ---------------------------------------------------------
IoSdEvent *IoSdRootI::ReadEventFromFile(EventPos pos) {
  IoSdKey key = Keys[pos];
  return ReadEventFromFile(key);
}

// ---------------------------------------------------------
IoSdEvent *IoSdRootI::ReadEventFromFile(string key) {
  vector < IoSdKey >::iterator iter;
  for (iter = Keys.begin(); iter != Keys.end(); iter++)
    if (iter->Name == key)
      break;
  if (iter == Keys.end())
    return NULL;
  return ReadEventFromFile(*iter);
}

// ---------------------------------------------------------
IoSdEvent *IoSdRootI::ReadEventFromFile(unsigned int id) {
  vector < IoSdKey >::iterator iter;
  for (iter = Keys.begin(); iter != Keys.end(); iter++)
    if (iter->EvtId == id)
      break;
  if (iter == Keys.end())
    return NULL;
  return ReadEventFromFile(*iter);
}


IoSdEvent *IoSdRootI::ReadEventFromFileGps(unsigned int gps) {
  vector < IoSdKey >::iterator iter;
  for (iter = Keys.begin(); iter != Keys.end(); iter++)
    if (iter->EvtGps == gps)
      break;
  if (iter == Keys.end())
    return NULL;
  return ReadEventFromFileGps(*iter);
}

