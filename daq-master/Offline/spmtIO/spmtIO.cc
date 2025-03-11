#include "spmtIO.h"

ClassImp(spmtIO)

void spmtIO::Print() const {
  std::cout << "spmtIO : tankID = " << LsId
            << "Nevents = " <<evt.size()
            << std::endl;
}

void spmtIO::ResetData() {

  LsId = 0;
  evt.clear();
  muon.clear();
  thres.clear();
}
