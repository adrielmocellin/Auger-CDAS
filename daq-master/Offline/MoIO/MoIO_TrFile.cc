#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <string.h>
#include <iostream>

#include "MoIO_TrFile.h"

#define kMaxFileNameSize 256
using namespace std;
void SelectTrFile(std::vector<TTrFile>& tr_files, time_t time_beg, time_t time_end) {

  tr_files.clear(); // Reset the file list content

  // 1st part :
  // Make a loop in the FILEPATH/ directory
  // and fill a vector with all the monitoring files found
  // then the vector is sorted according to the TTrFile class sort methods

  std::vector<TTrFile> allfiles;
  DIR *dirp;
  struct dirent *direntp;
  char filedir[kMaxFileNameSize];
  char filenameforselection[kMaxFileNameSize];
  char temp_string[kMaxFileNameSize];

  for (int i=2001; i<2021; i++) {
    for (int j=1; j<13; j++) {
      strcpy(filedir,kMrFilePath);
      sprintf(temp_string,"%d/%.2d/",i,j);
      strcat(filedir,temp_string);

      if ((dirp=opendir(filedir))==NULL) {
	//cerr <<"Could not open " << filedir <<" directory:: " <<strerror(errno)<< endl;
      } else while ((direntp=readdir (dirp))!=NULL) {
	if( (strstr(direntp->d_name,"t2_20")!=NULL) && (strstr(direntp->d_name,".dat")!=NULL)) {
	  strcpy(filenameforselection,filedir);
	  strcat(filenameforselection,direntp->d_name);
	  allfiles.push_back(filenameforselection);
	}
      }
      closedir(dirp);
    }
  }

  int size=allfiles.size();
  if (size==0) return;
  std::sort<std::vector<TTrFile>::iterator>(allfiles.begin(),allfiles.end());
  
  // List all the files found in the directory FILEPATH (i.e. /Raid/monit/Sd/ by default)
  //cout << "List of files found in directory " << FILEPATH << endl;
  //for (unsigned int i=0; i<allfiles.size(); i++)
  //cout << allfiles[i].filename() << endl;
  

  // 2nd part
  // Let's define the range in time of the selection
  // Then we build two TTrFile to be compared with the previous selection
  struct tm *ptime_beg;
  ptime_beg=gmtime(&time_beg);

  int y1=ptime_beg->tm_year+1900;
  int mo1=ptime_beg->tm_mon+1;
  int d1=ptime_beg->tm_mday; 

  struct tm *ptime_end;
  time_end = time_end-10;         // 10 seconds less than the time to avoid adding ininteressant files
  ptime_end=gmtime(&time_end);

  int y2=ptime_end->tm_year+1900; 
  int mo2=ptime_end->tm_mon+1;
  int d2=ptime_end->tm_mday; 

  TTrFile oldestfile(y1,mo1,d1);
  TTrFile newestfile(y2,mo2,d2);

  // 3rd part
  // Make the final selection
  std::vector<TTrFile>::iterator f1,f2,fsel1,fsel2;
  f1=lower_bound(allfiles.begin(),allfiles.end(),oldestfile);
  f2=upper_bound(allfiles.begin(),allfiles.end(),newestfile);

  //cout << endl << " oldestfile : " << oldestfile.filename() << endl;
  //cout << " newestfile : " <<  newestfile.filename() << endl;
  //cout << " begin = " << (allfiles.begin())->filename() << endl;
  //cout << " end = " << (allfiles.end())->filename() << endl;
  //cout << " f1 = " << f1->filename() << endl;
  //cout << " f2 = " << f2->filename() << endl;

  //if (f1 != allfiles.begin()) fsel1=f1-1;
  //else fsel1=f1;
  fsel1=f1;

  //if (f2) fsel2=f2-1;
  fsel2=f2-1;

  //cout << "selected first file  " << fsel1->filename() << endl;
  //cout << "selected last file   " << fsel2->filename() << endl;
  for (std::vector<TTrFile>::iterator iter=fsel1; iter<=fsel2; iter++) tr_files.push_back(*iter);
  //int ns = tr_files.size();
  //cout << ns  << "  files selected " << endl;

  //For obtaining the list of files that have been selected
  //for (unsigned int i=0; i<tr_files.size(); i++)
  //  cout << "Files selected : " << (tr_files[i]).filename() << endl;
}


void SelectTrFile(std::vector<TTrFile>& tr_files, const char *flag) {

  if (strcmp(flag,"C")!= 0 && strcmp(flag,"L")!= 0) {
    cout << "Only call with C for Current or L for Last are possible " << endl;
  }

  tr_files.clear(); // Reset the file list content
  // 1st part :
  // Make a loop in the FILEPATH/ directory
  // and fill a vector with all the monitoring files found
  // then the vector is sorted according to the TTrFile class sort methods

  std::vector<TTrFile> allfiles;

  DIR *dirp;
  struct dirent *direntp;
  char filedir[kMaxFileNameSize];
  char filenameforselection[kMaxFileNameSize];
  char temp_string[kMaxFileNameSize];

  for (int i=2001; i<2021; i++) {
    for (int j=1; j<13; j++) {
      strcpy(filedir,kMrFilePath);
      sprintf(temp_string,"%d/%.2d/",i,j);
      strcat(filedir,temp_string);
      
      if ((dirp=opendir(filedir))==NULL) {
	//cerr <<"Could not open " << filedir <<" directory:: " <<strerror(errno)<< endl;
      } else while ((direntp=readdir (dirp))!=NULL) {
	if( (strstr(direntp->d_name,"t2_20")!=NULL) && (strstr(direntp->d_name,".dat")!=NULL)) {
	  strcpy(filenameforselection,filedir);
	  strcat(filenameforselection,direntp->d_name);
	  allfiles.push_back(filenameforselection);
	}
      }
      closedir(dirp);
    }
  }

  int size=allfiles.size();
  if (size==0) return;
  std::sort<std::vector<TTrFile>::iterator>(allfiles.begin(),allfiles.end());

  if (strcmp(flag,"C") == 0) {
    tr_files.push_back(allfiles[size-1]);
    return;
  }

  if (strcmp(flag,"L") == 0) {
    tr_files.push_back(allfiles[size-2]);
    return;
  }

  cout << "You should not see that message !!!! Bug in MoIO_MoRFile.cc" << endl;
  cout << "We have been called with flag = " << flag << endl;
}


// Methods of the TTrFile Class
/*----------------------------------------------------------------------*/
TTrFile:: TTrFile(int y=0,int mo=0,int d=0):yearnum_(y), monthnum_(mo), daynum_(d) {
  sprintf(filename_,"%d/%.2d/t2_%d_%.2d_%.2d.dat",y,mo,y,mo,d);
}

/*----------------------------------------------------------------------*/
TTrFile::TTrFile(const char* name) {

  strcpy(filename_,name);

  int year1, year2, month1, month2, day;
	char *buf=(char*)name;
	buf+=strlen(kMrFilePath);
	// In case path is with trailing '/'
	if (buf[0]=='/') buf++;
  sscanf(buf,"%d/%d/t2_%d_%d_%d.dat",&year1,&month1,&year2,&month2,&day);

  yearnum_=year1;
  monthnum_=month1;
  daynum_=day;
}

/*----------------------------------------------------------------------*/
TTrFile::TTrFile(const TTrFile& file) {
  strcpy(filename_,file.filename_);
  yearnum_=file.yearnum_;
  monthnum_=file.monthnum_;
  daynum_=file.daynum_;
}

/*----------------------------------------------------------------------*/
TTrFile& TTrFile::operator=(const TTrFile& file) {
  strcpy(filename_,file.filename_);
  yearnum_=file.yearnum_;
  monthnum_=file.monthnum_;
  daynum_=file.daynum_;
  return *this;
}

/*----------------------------------------------------------------------*/
bool TTrFile::operator<(const TTrFile& f) const  {
  struct tm *timestructure;
  timestructure = (struct tm*)malloc(sizeof(struct tm));

  timestructure->tm_year = yearnum_ - 1900;
  timestructure->tm_mon = monthnum_ - 1;
  timestructure->tm_mday = daynum_;
  timestructure->tm_hour = 0;
  timestructure->tm_min = 0;
  timestructure->tm_sec = 0;

  time_t left  = mktime(timestructure);

  timestructure->tm_year = f.yearnum_ - 1900;
  timestructure->tm_mon = f.monthnum_ - 1;
  timestructure->tm_mday = f.daynum_;
  timestructure->tm_hour = 0;
  timestructure->tm_min = 0;
  timestructure->tm_sec = 0;

  time_t right = mktime(timestructure);

  free(timestructure);

  return ((left)<(right));
}

/*----------------------------------------------------------------------*/
bool TTrFile::operator==(const TTrFile& f) const {
  struct tm *timestructure;
  timestructure = (struct tm*)malloc(sizeof(struct tm));

  timestructure->tm_year = yearnum_ - 1900;
  timestructure->tm_mon = monthnum_ - 1;
  timestructure->tm_mday = daynum_;
  timestructure->tm_hour = 0;
  timestructure->tm_min = 0;
  timestructure->tm_sec = 0;

  time_t left  = mktime(timestructure);

  timestructure->tm_year = f.yearnum_ - 1900;
  timestructure->tm_mon = f.monthnum_ - 1;
  timestructure->tm_mday = f.daynum_;
  timestructure->tm_hour = 0;
  timestructure->tm_min = 0;
  timestructure->tm_sec = 0;

  time_t right = mktime(timestructure);

  free(timestructure);

  return ((left)==(right));
}

/*----------------------------------------------------------------------*/
bool TTrFile::operator!=(const TTrFile& f) const {
  return !((*this)==(f));
}

/*----------------------------------------------------------------------*/
bool TTrFile::operator<=(const TTrFile& f) const {
  return ( ((*this)<(f))||((*this)==(f)) ) ;
}

/*----------------------------------------------------------------------*/
bool TTrFile::operator>(const TTrFile& f) const {
  return ( !( ((*this)<(f))||((*this)==(f)) )) ;
}

/*----------------------------------------------------------------------*/
bool TTrFile::operator>=(const TTrFile& f) const {
  return (!(((*this)<(f)))) ;
}

/*----------------------------------------------------------------------*/
char* TTrFile::filename() {
  return filename_;
}

