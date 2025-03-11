#ifndef TCDASUTIL_H
#define TCDASUTIL_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2   1.57079632679489661923  /* pi/2 */
#endif

#include <vector>
#include <string>
#include <time.h>

namespace kCdasUtil { 
/// CdasUtil class, for various convertions
const double CMICRO  =  299.792458;
const double CSPEED  =  0.299792458;
const double CSPEEDSI=  299792458;
const double RAD2DEG =  (180.0/M_PI);
const double DEG2RAD =  (M_PI/180.0);
const double TwoPi     =  (2.*M_PI);

  /// GPS starts on 6th of January 1980
const int GPSStartSecond = 315964800;
  /// Leap seconds since GPS start
const unsigned int NLEAP = 17;
// filling Leap second vector in GPS seconds
// see http://hpiers.obspm.fr/iers/bul/bulc/UTC-TAI.history
// and http://hpiers.obspm.fr/iers/bul/bulc/bulletinc.dat
// when GPS starts, it has 19s offset with TAI 
// when auger starts, UTC offset with TAI is 32s, meaning 13s for GPS-UTC
// valid up to Jan 1st 2016
extern int LeapVal[NLEAP];

extern  std::string UTCDate(int gpssecond);     ///< GPS to text conversion
unsigned int UTCsecond(int gpssecond);  ///< GPS to UTC conversion
time_t UNIXsecond(int gpssecond);       ///< GPS to Unix conversion
int Leap(int gpssecond);                ///< Number of leap seconds
void GPS2Date(unsigned int & gpstime, int& fday, int& fmonth, int& fyear,int& fhour,int& fmin,int& fsec); ///< GPS to complete date
void StringDate2GPS(std::string& sdate, unsigned long & gps); ///< complete date to GPS
 time_t Date2UTC(int day, int month, int year);
 time_t Date2UTC(int day, int month, int year, int hour);
//Convert UTC to year
void UTC2Date(time_t utctime, int &day, int &month, int &year);
 void UTC2Date(time_t utctime, int &day, int &month, int &year,int& hour,int &min);
 void UTC2Date(time_t utctime, int &day, int &month, int &year,int& hour,int &min,int& sec);

}
#endif
