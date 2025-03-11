/*-------------------------------------------------------------------------*/
/*!
  \file MoIO_MrFile
  \brief This file contains the description of the MoRFile class
         which is useful when you want to find monitoring files
	 in the different monitoring directory
  \author Cyril Lachaud for the P. Auger Collaboration
  \date creation : 7 Dec 2001
 */
/*-------------------------------------------------------------------------*/
#ifndef MoIO_MrFile_H
#define MoIO_MrFile_H

#include <vector>
#include <time.h>

class TMrFile {
 public:
  TMrFile(int,int,int,int,int);
  TMrFile(const char* );
  TMrFile(const TMrFile& );
  TMrFile& operator=(const TMrFile&);
  bool operator<( const TMrFile&) const ;
  bool operator<=( const TMrFile&) const ;
  bool operator>( const TMrFile&) const ;
  bool operator>=( const TMrFile&) const ;
  bool operator==(const TMrFile&) const ;
  bool operator!=(const TMrFile&) const;

  char* filename();

 private:
  char filename_[256];
  int yearnum_;
  int monthnum_;
  int daynum_;
  int hournum_;
  int minnum_;
};

void SelectMrFile(std::vector<TMrFile>&, time_t, time_t);
void SelectMrFile(std::vector<TMrFile>&, const char*);

#endif
