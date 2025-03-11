#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <string.h>
#include <iostream>

#include "MoIO_MrFile.h"

#define kMaxFileNameSize 256
using namespace std;

// SelectMoRFile function definition : model for selecting files code
void SelectMrFile(std::vector<TMrFile>& mc_rootfiles, time_t time_beg, time_t time_end) {

  mc_rootfiles.clear(); // Reset the rootfile list content

  // 1st part :
  // Make a loop in the ROOTFILEPATH/ directory
  // and fill a vector with all the monitoring rootfiles found
  // then the vector is sorted according to the TMrFile class sort methods

  std::vector<TMrFile> allrootfiles;
  DIR *dirp;
  struct dirent *direntp;
  char rootfiledir[kMaxFileNameSize];
  char rootfilenameforselection[kMaxFileNameSize];
  char temp_string[kMaxFileNameSize];

  for (int i=2001; i<2021; i++) {
    for (int j=1; j<13; j++) {
      strcpy(rootfiledir,kMrFilePath);
      sprintf(temp_string,"%d/%.2d/",i,j);
      strcat(rootfiledir,temp_string);
      
      if ((dirp=opendir(rootfiledir))==NULL) {
	//cerr <<"Could not open " << rootfiledir <<" directory:: " <<strerror(errno)<< endl;
      }
      else while ((direntp=readdir(dirp))!=NULL) {
	if( (strstr(direntp->d_name,"mc_20")!=NULL) && (strstr(direntp->d_name,".root")!=NULL)) {
	  strcpy(rootfilenameforselection,rootfiledir);
	  strcat(rootfilenameforselection,direntp->d_name);
	  allrootfiles.push_back(rootfilenameforselection);
	}
      }
      if (dirp!=NULL) closedir(dirp);
    }
  }

  int size=allrootfiles.size();
  if (size==0) return;
  std::sort<std::vector<TMrFile>::iterator>(allrootfiles.begin(),allrootfiles.end());
  
  // List all the files found in the directory ROOTFILEPATH (i.e. /Raid/monit/Sd/ by default)
  //cout << "List of files found in directory " << ROOTFILEPATH << endl;
  //for (unsigned int i=0; i<allrootfiles.size(); i++)
  //cout << allrootfiles[i].filename() << endl;
  

  // 2nd part
  // Let's define the range in time of the selection
  // Then we build two TMrFile to be compared with the previous selection
  struct tm *ptime_beg;
  ptime_beg=gmtime(&time_beg);

  int y1=ptime_beg->tm_year+1900; int mo1=ptime_beg->tm_mon+1;
  int d1=ptime_beg->tm_mday; int h1=ptime_beg->tm_hour; int m1=ptime_beg->tm_min;

  struct tm *ptime_end;
  ptime_end=gmtime(&time_end);

  int y2=ptime_end->tm_year+1900; int mo2=ptime_end->tm_mon+1;
  int d2=ptime_end->tm_mday; int h2=ptime_end->tm_hour; int m2=ptime_end->tm_min;

  TMrFile oldestfile(y1,mo1,d1,h1,m1);
  TMrFile newestfile(y2,mo2,d2,h2,m2);

  // 3rd part
  // Make the final selection
  std::vector<TMrFile>::iterator f1,f2,fsel1,fsel2;
  f1=lower_bound(allrootfiles.begin(),allrootfiles.end(),oldestfile);
  f2=upper_bound(allrootfiles.begin(),allrootfiles.end(),newestfile);

  //cout << endl << " oldestfile : " << oldestfile.filename() << endl;
  //cout << " newestfile : " <<  newestfile.filename() << endl;
  //cout << " begin = " << (allrootfiles.begin())->filename() << endl;
  //cout << " end = " << (allrootfiles.end())->filename() << endl;
  //cout << " f1 = " << f1->filename() << endl;
  //cout << " f2 = " << f2->filename() << endl;

  if (f1 != allrootfiles.begin()) fsel1=f1-1;
  else fsel1=f1;
 
  fsel2=f2-1;
  //if (f2) fsel2=f2-1;

  //cout << "selected first file  " << fsel1->filename() << endl;
  //cout << "selected last file   " << fsel2->filename() << endl;
  for (std::vector<TMrFile>::iterator iter=fsel1; iter<=fsel2; iter++) mc_rootfiles.push_back(*iter);
  //int ns = mc_rootfiles.size();
  //cout << ns  << " Root files selected " << endl;

  //// For obtaining the list of files that have been selected
  //for (unsigned int i=0; i<mc_rootfiles.size(); i++)
  //  cout << (mc_rootfiles[i]).filename() << endl;
}


void SelectMrFile(std::vector<TMrFile>& mc_rootfiles, const char *flag) {

  if (strcmp(flag,"C")!= 0 && strcmp(flag,"L")!= 0) {
    cout << "SelectMoRFile(...) has been called with flag = " << flag << endl;
    cout << "Only call with C for Current or L for Last are possible " << endl;
  }

  mc_rootfiles.clear(); // Reset the rootfile list content
  // 1st part :
  // Make a loop in the ROOTFILEPATH/ directory
  // and fill a vector with all the monitoring rootfiles found
  // then the vector is sorted according to the TMrFile class sort methods

  std::vector<TMrFile> allrootfiles;

  DIR *dirp;
  struct dirent *direntp;
  char rootfiledir[kMaxFileNameSize];
  char rootfilenameforselection[kMaxFileNameSize];
  char temp_string[kMaxFileNameSize];

  for (int i=2001; i<2021; i++) {
    for (int j=1; j<13; j++) {
      strcpy(rootfiledir,kMrFilePath);
      sprintf(temp_string,"%d/%.2d/",i,j);
      strcat(rootfiledir,temp_string);
      
      if ((dirp=opendir(rootfiledir))==NULL) {
	//cerr <<"Could not open " << rootfiledir <<" directory:: " <<strerror(errno)<< endl;
      } else while ((direntp=readdir (dirp))!=NULL) {
	if( (strstr(direntp->d_name,"mc_20")!=NULL) && (strstr(direntp->d_name,".root")!=NULL)) {
	  strcpy(rootfilenameforselection,rootfiledir);
	  strcat(rootfilenameforselection,direntp->d_name);
	  allrootfiles.push_back(rootfilenameforselection);
	}
      }
      if (dirp!=NULL) closedir(dirp);
    }
  }

  int size=allrootfiles.size();
  if (size==0) return;
  std::sort<std::vector<TMrFile>::iterator>(allrootfiles.begin(),allrootfiles.end());

  if (strcmp(flag,"C") == 0) {
    mc_rootfiles.push_back(allrootfiles[size-1]);
    return;
  }

  if (strcmp(flag,"L") == 0) {
    mc_rootfiles.push_back(allrootfiles[size-2]);
    return;
  }

  cout << "You should not see that message !!!! Bug in MoIO_MrFile.cc" << endl;
  cout << "We have been called with flag = " << flag << endl;
}


// Methods of the TMrFile Class
/*----------------------------------------------------------------------*/
TMrFile:: TMrFile(int y=0,int mo=0,int d=0,int h=0,int m=0):yearnum_(y), monthnum_(mo), daynum_(d), hournum_(h),minnum_(m) {
  sprintf(filename_,"%d/%.2d/mc_%d_%.2d_%.2d_%.2dh%.2d.root",y,mo,y,mo,d,h,m);
}

/*----------------------------------------------------------------------*/
TMrFile::TMrFile(const char* name) {

  strcpy(filename_,name);

  int year1, year2, month1, month2, day, hour, min;
	char *buf=(char*)name;
	buf+=strlen(kMrFilePath);
	// In case path is with trailing '/'
	if (buf[0]=='/') buf++;
  sscanf(buf,"%d/%d/mc_%d_%d_%d_%dh%d.root",&year1,&month1,&year2,&month2,&day,&hour,&min);

  yearnum_=year1;
  monthnum_=month1;
  daynum_=day;
  hournum_=hour;
  minnum_=min;
}

/*----------------------------------------------------------------------*/
TMrFile::TMrFile(const TMrFile& file) {
  strcpy(filename_,file.filename_);
  yearnum_=file.yearnum_;
  monthnum_=file.monthnum_;
  daynum_=file.daynum_;
  hournum_=file.hournum_;
  minnum_=file.minnum_;
}

/*----------------------------------------------------------------------*/
TMrFile& TMrFile::operator=(const TMrFile& file) {
  strcpy(filename_,file.filename_);
  yearnum_=file.yearnum_;
  monthnum_=file.monthnum_;
  daynum_=file.daynum_;
  hournum_=file.hournum_;
  minnum_=file.minnum_;
  return *this;
}

/*----------------------------------------------------------------------*/
bool TMrFile::operator<(const TMrFile& f) const  {
  struct tm *timestructure;
  timestructure = (struct tm*)malloc(sizeof(struct tm));

  timestructure->tm_year = yearnum_ - 1900;
  timestructure->tm_mon = monthnum_ - 1;
  timestructure->tm_mday = daynum_;
  timestructure->tm_hour = hournum_;
  timestructure->tm_min = minnum_;
  timestructure->tm_sec = 0;

  time_t left  = mktime(timestructure);

  timestructure->tm_year = f.yearnum_ - 1900;
  timestructure->tm_mon = f.monthnum_ - 1;
  timestructure->tm_mday = f.daynum_;
  timestructure->tm_hour = f.hournum_;
  timestructure->tm_min = f.minnum_;
  timestructure->tm_sec = 0;

  time_t right = mktime(timestructure);

  free(timestructure);

  return ((left)<(right));
}

/*----------------------------------------------------------------------*/
bool TMrFile::operator==(const TMrFile& f) const {
  struct tm *timestructure;
  timestructure = (struct tm*)malloc(sizeof(struct tm));

  timestructure->tm_year = yearnum_ - 1900;
  timestructure->tm_mon = monthnum_ - 1;
  timestructure->tm_mday = daynum_;
  timestructure->tm_hour = hournum_;
  timestructure->tm_min = minnum_;
  timestructure->tm_sec = 0;

  time_t left  = mktime(timestructure);

  timestructure->tm_year = f.yearnum_ - 1900;
  timestructure->tm_mon = f.monthnum_ - 1;
  timestructure->tm_mday = f.daynum_;
  timestructure->tm_hour = f.hournum_;
  timestructure->tm_min = f.minnum_;
  timestructure->tm_sec = 0;

  time_t right = mktime(timestructure);

  free(timestructure);

  return ((left)==(right));
}

/*----------------------------------------------------------------------*/
bool TMrFile::operator!=(const TMrFile& f) const {
  return !((*this)==(f));
}

/*----------------------------------------------------------------------*/
bool TMrFile::operator<=(const TMrFile& f) const {
  return ( ((*this)<(f))||((*this)==(f)) ) ;
}

/*----------------------------------------------------------------------*/
bool TMrFile::operator>(const TMrFile& f) const {
  return ( !( ((*this)<(f))||((*this)==(f)) )) ;
}

/*----------------------------------------------------------------------*/
bool TMrFile::operator>=(const TMrFile& f) const {
  return (!(((*this)<(f)))) ;
}

/*----------------------------------------------------------------------*/
char* TMrFile::filename() {
  return filename_;
}

