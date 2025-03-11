#ifndef TSTSKY_H
#define TSTSKY_H

#include <math.h>
#include <time.h>
#include "TShowerParams.h"
#include "TCdasUtil.h"

/// convertion routines to get sky angles
class TSTSky {
public:
  struct Date {
    int year;
    int month;
    int day;
    double time;
  }
  utc;
  struct Solar {              /* nutation and solar coordinate parameters  */
    double d_psi;           /* nutation of ecliptic longtitude */
    double d_epsi;          /* nutation of obliquity angle */
    double lambda_s;        /* real ecliptic longtitude of sun */
    double epsi;            /* obliquity angle */
  }
  sol;
  struct Equator {            /* eqatorial coodinate */
    double ra;              /* right ascention */
    double dec;             /* decrination */
  }
  eq;
  struct Geo {                /* geodetic coordinate */
    double lg;              /* longtitude */
    double lat;             /* latitude */
  }
  lg_lat;
  struct Horizon {            /* Horizontal coordinate */
    double az;              /* Azmuthal Angle */
    double el;              /* Elevation Angle */
  }
  hz;
  struct Gal {                /* galactic coodinate */
    double l;               /* galactic longitude */
    double b;               /* galactic latitude */
  }
  gal;
  double jd, gst;

  double deg2rad(double deg) {
    return (deg * M_PI / 180.0);
  }
  double rad2deg(double rad) {
    return (rad * 180.0 / M_PI);
  }
  double ut2jd(Date date);
  double ut2gst();
  double ut2gmst();
  void nut();

  void ho2eq();
  void eq2gal();

  TSTSky() {
    lg_lat.lg = 69.25;
    lg_lat.lat = -35.25;
  };
  TSTSky(double lg, double lat) {
    lg_lat.lg = lg;
    lg_lat.lat = lat;
  };
  ~TSTSky() {}
  ;
  void SetPoint(int year, int month, int day, int hour, int min, int sec,
                double az, double el);
  void SetPoint(const time_t UNIXtime, double auger_theta, double auger_phi);
  void SetPoint(const TShowerParams &sp);
  double GetRa() {
    return (eq.ra);
  }
  double GetDec() {
    return (eq.dec);
  }
  double GetL() {
    return (gal.l);
  }
  double GetB() {
    return (gal.b);
  }
};

extern TSTSky gSTSky;
#endif
