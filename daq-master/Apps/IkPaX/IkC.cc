#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include "IkClient.h"
#include "IkC.h"

#include <iostream>
#include <cstdlib>

#define MODE (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)


using namespace std;

unsigned int gCDASInitWasHere = 0;
string _IkCName_="Ik";

// LOGGING FEATURES (Xb 11/09/2005)
FILE *gLogFile=NULL;
int gDay=0;

void LogMessage(const char *message) {
  // getting date
  char datebuf[256];
  time_t t;
  time(&t);
  if (!gLogFile || gDay!=(int)(t/86400)) {
    gDay=(int)(t/86400);
    if (gLogFile) fclose(gLogFile);
    stringstream filename;
    filename << "/Raid/var/log/" << _IkCName_ << "/";
    mkdir(filename.str().c_str(),MODE);
    struct tm *tmptm=gmtime(&t);
    filename << (tmptm->tm_year+1900);
    mkdir(filename.str().c_str(),MODE);
    filename << "/";
    if (tmptm->tm_mon+1<10) filename << 0;
    filename << (tmptm->tm_mon+1);
    mkdir(filename.str().c_str(),MODE);
    filename << "/" << _IkCName_ << "_" << (tmptm->tm_year+1900) << "_";
    if (tmptm->tm_mon+1<10) filename << 0;
    filename << (tmptm->tm_mon+1) << "_";
    if (tmptm->tm_mday<10) filename << 0;
    filename << (tmptm->tm_mday) << "_00h00.log";
    gLogFile=fopen(filename.str().c_str(),"a");
  }
  ctime_r(&t, datebuf);
  if (strchr(datebuf,'\n')) *strchr(datebuf,'\n')='\0';
  if (gLogFile) {
    struct timeval newtv;
    fprintf(gLogFile,"%s%s\n",datebuf,message);
    fflush(gLogFile);
  }
}

void InternalLog(const char *text,string err) {
  stringstream s;
  s << "|" << _IkCName_ << "|Log|" << err << "|" << text << "|";
  LogMessage(s.str().c_str());
  cerr << s.str().c_str() << endl;
}

// using IkClient now
int CDASInit(const char *name, const char *IkServerHostname)
{
  // This locks out IkMonitor from calling IkJustRequest without
  // calling IkJustConnect
  gCDASInitWasHere = 1;
  _IkCName_ = name;
  IkJustConnect((char *) IkServerHostname,(char *) name);
  return 0;
}

// DO NOT FUCK AROUND WITH IkMonitor STRINGS ANYMORE!!!
// I COMPLETELY IGNORE ALL THE AND/OR CRAP
// YOU BETTER NOT SEND ONE LARGER THAN LIKE 500 CHARS, TOO
int IkMonitor(void (*function)(IkMessage *ikm), const char *fmt, ...)
{
  int type = -1;
  string stringtype;
  char buffer[500];

  va_list ap;

  if (!gCDASInitWasHere)
    {
      cerr << "CDASInit() should have been called before IkMonitor!" << endl;
      cerr << "How did this program ever work before?!" << endl;
      exit(1);
    }

  if (strlen(fmt) > 0)
    {
      va_start(ap, fmt);
      vsnprintf(buffer, 500, fmt, ap);
      va_end(ap);
    }
  string selection = buffer;
  string::size_type type_start, type_end;
  type_start = selection.find("type is ");
  if (type_start != string::npos)
    {
      type_end = type_start + strlen("type is ");
      type = atoi(selection.substr(type_end, selection.length()-type_end).c_str());
      cout << "Selecting messages of type " << type << endl;
      stringtype = IkFindTypeStr(type);
      if (stringtype == IkUnknownType)
	{
	  cerr << "Request for unknown type " << type << endl;
	  exit(1);
	}
    }
  if (type > 0)
    {
      IkJustRequest((char *)_IkCName_.c_str(), (char *) stringtype.c_str(), 0, function);
    }
  else
    {
      IkJustRequest((char *)_IkCName_.c_str(), NULL, 0, function);
    }
  return 0;
}

