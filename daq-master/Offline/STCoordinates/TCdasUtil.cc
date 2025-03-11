#include "TCdasUtil.h"
#include "STClibrary.h"
#include <cstdlib>

// filling Leap second vector in GPS seconds
// see http://hpiers.obspm.fr/iers/bul/bulc/UTC-TAI.history
// and http://hpiers.obspm.fr/iers/bul/bulc/bulletinc.dat
// when GPS starts, it has 19s offset with TAI 
// when auger starts, UTC offset with TAI is 32s, meaning 13s for GPS-UTC
// valid up to Jan 1st 2016
int kCdasUtil::LeapVal[kCdasUtil::NLEAP] = {46828800, 78364800, 109900800, 173059200, 252028800, 315187200, 346723200, 393984000, 425520000, 457056000, 504489600, 551750400, 599184000, 820108800,914803200,1025136000,1435708800};


// Taking care of Leap second for sky pointing. In any case, who cares?
// XB 25/01/2016: Note that this is NOT the UTCsecond, ie the number of seconds elapsed since 1 Jan 1970. Name is misleading but has been used for years so will not be changed
unsigned int kCdasUtil::UTCsecond(int gpssecond) {
  return kCdasUtil::GPSStartSecond+gpssecond-kCdasUtil::Leap(gpssecond);
}

time_t kCdasUtil::UNIXsecond(int gpssecond) {
  // Unix has no leap second, like GPS
  // XB 25/01/2016: BUT we have to take them into account for the UNIX function time() to work. It is therefore like the "UTCSecond" above, which is actually not UTC, but that's another topic
  return (time_t)(kCdasUtil::GPSStartSecond+gpssecond-kCdasUtil::Leap(gpssecond));
}

std::string kCdasUtil::UTCDate(int gpssecond) {
  setenv("TZ", "UTC", 1);
  const time_t u=kCdasUtil::UNIXsecond(gpssecond);
  std::string UTCDate = (std::string) (ctime(&u));
  UTCDate.erase(UTCDate.size() - 1, 1);
  return UTCDate;
}

// Counting leap seconds
int kCdasUtil::Leap(int gpssecond) {
  int ret=0;
  for (unsigned int i=0;i<kCdasUtil::NLEAP;i++) {
    if (gpssecond>kCdasUtil::LeapVal[i]+ret)
      ret++;
    else
      break;
  }
  return ret;
}

void kCdasUtil::GPS2Date(unsigned int & gpstime, int& fday, int& fmonth, int& fyear,int& fhour,int& fmin,int& fsec)
{
  tm *ptm;
  time_t utctime= (time_t)gpstime+315964800;;
  ptm=gmtime(&utctime);
  fyear=ptm->tm_year+1900;
  fmonth=ptm->tm_mon+1;
  fday=ptm->tm_mday;
  fhour=ptm->tm_hour;
  fmin=ptm->tm_min;
  fsec=ptm->tm_sec;
  return;
}

void kCdasUtil::StringDate2GPS(std::string& sdate, unsigned long & gps)
{
  int yy,mo,dd,hh,mm,ss;
  std::string syy,smo,sdd,shh,smm,sss;
  syy=sdate.substr(0,4); 
  smo=sdate.substr(5,2); 
  sdd=sdate.substr(8,2); 
  shh=sdate.substr(11,2); 
  smm=sdate.substr(14,2); 
  sss=sdate.substr(17,2); 
  yy=atoi(syy.c_str());
  mo=atoi(smo.c_str());
  dd=atoi(sdd.c_str());
  hh=atoi(shh.c_str());
  mm=atoi(smm.c_str());
  ss=atoi(sss.c_str());
  
  date2gps(yy,mo,dd,hh,mm,ss,gps);
  //cout<<kCdasUtil::Leap(gps)<<endl;
  gps+=kCdasUtil::Leap(gps);
  return ;
}

time_t kCdasUtil::Date2UTC(int day, int month, int year)
{
  time_t utc;
  struct tm time_str;
  time_str.tm_year = year-1900;
  time_str.tm_mon=month-1;
  time_str.tm_mday=day;
  time_str.tm_hour = 12;
  time_str.tm_min = 0;
  time_str.tm_sec = 0;
  //TTimeStamp *t = new TTimeStamp();
  //int offset=t->GetZoneOffset();
  //setenv ("TZ","UTC",1);
  utc=timegm(&time_str);
  return utc;
}
time_t kCdasUtil::Date2UTC(int day, int month, int year, int hour)
{
  time_t utc;
  struct tm time_str;
  time_str.tm_year = year-1900;
  time_str.tm_mon=month-1;
  time_str.tm_mday=day;
  time_str.tm_hour = hour;
  time_str.tm_min = 0;
  time_str.tm_sec = 0;
  //TTimeStamp *t = new TTimeStamp();
  //int offset=t->GetZoneOffset();
  //setenv ("TZ","UTC",1);
  utc=timegm(&time_str);
  return utc;
}
//Convert UTC to year
void kCdasUtil::UTC2Date(time_t utctime, int &day, int &month, int &year)
{
  int hour,min,sec;
   tm *ptm;
  ptm=gmtime(&utctime);
  year=ptm->tm_year+1900;
  month=ptm->tm_mon+1;
  day=ptm->tm_mday;
  hour=ptm->tm_hour;
  min=ptm->tm_min;
  sec=ptm->tm_sec;
  return;
}

void kCdasUtil::UTC2Date(time_t utctime, int &day, int &month, int &year,int& hour,int &min)
{
  int sec;
   tm *ptm;
  ptm=gmtime(&utctime);
  year=ptm->tm_year+1900;
  month=ptm->tm_mon+1;
  day=ptm->tm_mday;
  hour=ptm->tm_hour;
  min=ptm->tm_min;
  sec=ptm->tm_sec;
  return;
}
void kCdasUtil::UTC2Date(time_t utctime, int &day, int &month, int &year,int& hour,int &min,int& sec)
{

   tm *ptm;
  ptm=gmtime(&utctime);
  year=ptm->tm_year+1900;
  month=ptm->tm_mon+1;
  day=ptm->tm_mday;
  hour=ptm->tm_hour;
  min=ptm->tm_min;
  sec=ptm->tm_sec;
  return;
}

