#ifndef _SPMTROOT_H_
#define _SPMTROOT_H_
#include "spmtIO.h"
#include <string>

#include <TTree.h>
#include <TFile.h>

class SPMTRoot
{
public:
  SPMTRoot();
  ~SPMTRoot();

  void SetOutPath(const char *path);
  int NewFile(time_t curtime);
  void CloseFile();
  int AddData(spmtIO *dat);

private:
  std::string fOutDirPath_fmt;//output dir format (strftime)
  std::string fFileName_fmt; //output filename format (strftime).
  std::string fFileName;
  std::string fFileName_nobackup;
  time_t fFileTimeNext;
  time_t fNewFileInterval;
  TFile *fFile;

  TTree *fTree;
  spmtIO fData;

};

#endif
