/*-------------------------------------------------------------------------*/
/*!
  \file MoIO_TrFile
  \brief This file contains the description of the TrFile class
         which is useful when you want to find trigger files
	 in the different monitoring directory
  \author Cyril Lachaud for the P. Auger Collaboration
  \date creation : 25/01/2003
 */
/*-------------------------------------------------------------------------*/
#ifndef MoIO_TrFile_H
#define MoIO_TrFile_H

#include <vector>
#include <time.h>

class TTrFile {
 public:
  TTrFile(int,int,int);
  TTrFile(const char* );
  TTrFile(const TTrFile& );
  TTrFile& operator=(const TTrFile&);
  bool operator<( const TTrFile&) const ;
  bool operator<=( const TTrFile&) const ;
  bool operator>( const TTrFile&) const ;
  bool operator>=( const TTrFile&) const ;
  bool operator==(const TTrFile&) const ;
  bool operator!=(const TTrFile&) const;

  char* filename();

 private:
  char filename_[256];
  int yearnum_;
  int monthnum_;
  int daynum_;
  int hournum_;
  int minnum_;
};

void SelectTrFile(std::vector<TTrFile>&, time_t, time_t);
void SelectTrFile(std::vector<TTrFile>&, const char*);

#endif
