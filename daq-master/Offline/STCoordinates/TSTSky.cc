#include "TSTSky.h"
#include <time.h>

TSTSky gSTSky;

void TSTSky::SetPoint(const TShowerParams &sp) {
  return SetPoint(kCdasUtil::UNIXsecond(sp.fGPSSecond),sp.Theta(),sp.Phi());
}

void TSTSky::SetPoint(const time_t UNIXtime, double theta, double phi) {
  theta = rad2deg(theta);
  phi = rad2deg(phi);
  struct tm *gdate = gmtime(&UNIXtime);
  return SetPoint(gdate->tm_year + 1900, gdate->tm_mon + 1,
                  gdate->tm_mday, gdate->tm_hour, gdate->tm_min,
                  gdate->tm_sec, -90 - phi, 90 - theta);
}

double TSTSky::ut2jd(Date date) {
  int aa, bb;
  int yy, mm;
  double ymd;
  double jd_tmp;

  ymd = (double) date.year + (((double) date.month) * 0.01)
        + (((double) date.day) * 0.0001);

  if (utc.month > 2) {
    yy = date.year;
    mm = date.month;
  } else {
    yy = date.year - 1;
    mm = date.month + 12;
  }

  if (ymd >= 1582.1015) {
    aa = yy / 100;
    bb = 2 - aa + (aa / 4);
  } else
    bb = 0;

  jd_tmp = (double) ((int) ((double) yy * 365.25)
                     + (int) ((double) (mm + 1) * 30.6001) + date.day +
                     bb)
           + 1720994.5 + date.time / 24.0;
  return (jd_tmp);

}

void TSTSky::SetPoint(int year, int month, int day, int hour, int min,
                      int sec, double az, double el) {
  utc.year = year;
  utc.month = month;
  utc.day = day;
  utc.time = (double) hour + (double) min / 60.0 + (double) sec / 3600.0;
  hz.az = az;
  hz.el = el;
  if (hz.az < -180.0)
    hz.az = 360.0 + hz.az;
  if (hz.az > 180.0)
    hz.az = 360.0 - hz.az;

  jd = ut2jd(utc);
  nut();
  gst = ut2gst();

  ho2eq();
  eq2gal();
  /***
  printf("ddd %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
  utc.year,utc.month, utc.day,jd,gst,hz.az, hz.el,
  gst,sol.d_psi, sol.d_epsi,
  eq.ra, eq.dec,
  gal.l, gal.b);
  ***/

}

void TSTSky::nut()
/* caliclate "nutation" effect( unit: degree ) */
{
  double tt;                  /* time (Julius century) from 1900           */
  double ll;                  /* mean ecliptic longtitude of the sun (deg) */
  double ld;                  /* mean ecliptic longtitude of the moon(deg) */
  double mm;                  /* mean anomaly of the sun  (deg)            */
  double md;                  /* mean anomaly of the moon (deg)            */
  double omega;               /* ecliptic lg. of ascending node of moon    */
  double cc;                  /* central equation of the sun               */

  double ll_t, ld_t, mm_t, md_t, omega_t;
  double d_psi_t, d_epsi_t;

  tt = (jd - 2415020.0) / 36525.0;
  ll_t = 279.6967 + 36000.7689 * tt + 0.000303 * tt * tt;
  ld_t = 270.4342 + 481267.8831 * tt - 0.001133 * tt * tt;
  mm_t = 358.4758 + 35999.0498 * tt - 0.000150 * tt * tt;
  md_t = 296.1046 + 477198.8491 * tt + 0.009192 * tt * tt;
  omega_t = 259.1833 - 1934.1420 * tt + 0.002078 * tt * tt;

  ll = fmod(ll_t, 360.0);
  ld = fmod(ld_t, 360.0);
  mm = fmod(mm_t, 360.0);
  md = fmod(md_t, 360.0);
  if ((omega = fmod(omega_t, 360.0)) < 0.0)
    omega += 360.0;

  d_psi_t = -(17.2327 + 0.01737 * tt) * sin(deg2rad(omega))
            - (1.2729 + 0.00013 * tt) * sin(deg2rad(2.0 * ll))
            + 0.2088 * sin(deg2rad(2.0 * omega))
            - 0.2037 * sin(deg2rad(2.0 * ld))
            + (0.1261 - 0.00031 * tt) * sin(deg2rad(mm))
            + 0.0675 * sin(deg2rad(md))
            - (0.0497 - 0.00012 * tt) * sin(deg2rad(2.0 * ll + mm))
            - 0.0342 * sin(deg2rad(2.0 * ld - omega))
            - 0.0261 * sin(deg2rad(2.0 * ld + mm))
            + 0.0214 * sin(deg2rad(2.0 * ll - mm))
            - 0.0149 * sin(deg2rad(2.0 * (ll - ld) + md))
            + 0.0124 * sin(deg2rad(2.0 * ll - omega))
            + 0.0114 * sin(deg2rad(2.0 * ld - md));

  d_epsi_t = (9.2100 + 0.00091 * tt) * cos(deg2rad(omega))
             + (0.5522 - 0.00029 * tt) * cos(deg2rad(2.0 * ll))
             - 0.0904 * cos(deg2rad(2.0 * omega))
             + 0.0884 * cos(deg2rad(2.0 * ld))
             + 0.0216 * cos(deg2rad(2.0 * ll + mm))
             + 0.0183 * cos(deg2rad(2.0 * ld - omega))
             + 0.0113 * cos(deg2rad(2.0 * ld + md))
             - 0.0093 * cos(deg2rad(2.0 * ll - mm))
             - 0.0006 * cos(deg2rad(2.0 * ll - omega));

  sol.d_psi = d_psi_t / 3600.0;
  sol.d_epsi = d_epsi_t / 3600.0;

  cc = (1.919460 - 0.004789 * tt - 0.000014 * tt * tt) * sin(deg2rad(mm))
       + (0.020094 - 0.000100 * tt) * sin(deg2rad(2.0 * mm))
       + 0.000293 * sin(deg2rad(3.0 * mm));

  sol.lambda_s = ll + cc;
  sol.epsi = 23.452294 - 0.0130125 * tt
             - 0.00000164 * tt * tt
             + 0.000000503 * tt * tt * tt + 0.00256 * cos(deg2rad(omega));

}

double TSTSky::ut2gst()
/* convert from UTC to Greenwich Apparent Siderial Time */
{
  double nut_hour;            /* compensation term of nutation (hour) */
  double gmst;                /* Greenwich Mean Siderial Time         */
  double gst;                 /* Greenwich Apparent Siderial Time     */

  gmst = ut2gmst();

  nut_hour = sol.d_epsi * cos(deg2rad(sol.epsi)) / 15.0;

  gst = fmod((gmst + nut_hour), 24.0);

  return (gst);
}

double TSTSky::ut2gmst()
/* convert from UTC to Greenwich Mean Siderial Time */
{
  double jd_0;                /* JD when  UTC = 0 o'clock */
  double tt;

  double d_gmst_0;            /* GMST(0 o'clock) at rotation */
  double gmst_0;              /* GMST(0 o'clock) at  hour */
  double gmst;                /* GMST at hour */
  Date utc_0;

  utc_0.year = utc.year;
  utc_0.month = utc.month;
  utc_0.day = utc.day;
  utc_0.time = 0.0;

  jd_0 = ut2jd(utc_0);
  tt = (jd_0 - 2415020.0) / 36525.0;

  d_gmst_0 = 0.276919398 + (tt * 100.0021359) + (0.000001075 * tt * tt);
  gmst_0 = (d_gmst_0 - (double) ((int) d_gmst_0)) * 24.0;

  gmst = gmst_0 + (utc.time * 366.25 / 365.25);

  return (gmst);

}

void TSTSky::ho2eq()
/* convert from horizontal coordinate to equatorial coordinate */
{
  double hh;                  /* time angle (deg)   */
  double gclat_rad;           /* geocentric latitude (rad) */
  double ra_deg;              /* right ascention (deg)   */
  double az_rad, el_rad;      /* elevation and azimuth (rad)  */
  double x, y, z;
  double theta, phi;

  az_rad = deg2rad(hz.az);
  el_rad = deg2rad(hz.el);
  /*
     gclat_rad = atan( 0.99664719 * 0.99664719 * tan( deg2rad( lg_lat.lat ) ) );
   */
  gclat_rad = deg2rad(lg_lat.lat);

  x = (cos(gclat_rad) * sin(el_rad))
      + (sin(gclat_rad) * cos(el_rad) * cos(az_rad));
  y = (cos(el_rad) * sin(az_rad));
  z = (sin(gclat_rad) * sin(el_rad))
      - (cos(gclat_rad) * cos(el_rad) * cos(az_rad));
  theta = asin(z);
  phi = atan2(y, x);

  hh = rad2deg(phi);
  ra_deg = gst * 15.0 - lg_lat.lg - hh;
  if (ra_deg < 0.0)
    ra_deg += 360.0;
  if (ra_deg > 360.0)
    ra_deg -= 360.0;

  eq.ra = ra_deg;
  eq.dec = rad2deg(theta);
}

void TSTSky::eq2gal() {
  double x, y, z;
  double theta, phi;
  double ra_rad, dec_rad;
  double i_rad, l_0_rad, omeg_rad;
  double l_rad;

  double GE_EQE_RA = 282.85;  /* RA of ascending node of galactic equator (deg) */
  double GE_EQE_L = 32.933;   /* galactic lg of ascending node (deg) epoc 2000  */
  double GE_EQE = 62.867;     /* angle between equator and galactic equator(deg) */

  ra_rad = deg2rad(eq.ra);
  dec_rad = deg2rad(eq.dec);
  i_rad = deg2rad(GE_EQE);
  l_0_rad = deg2rad(GE_EQE_L);
  omeg_rad = deg2rad(GE_EQE_RA);

  x = (cos(dec_rad) * cos(ra_rad - omeg_rad));
  y = (sin(dec_rad) * sin(i_rad))
      + (cos(dec_rad) * sin(ra_rad - omeg_rad) * cos(i_rad));
  z = (sin(dec_rad) * cos(i_rad))
      - (cos(dec_rad) * sin(ra_rad - omeg_rad) * sin(i_rad));

  theta = asin(z);
  phi = atan2(y, x);

  l_rad = l_0_rad + phi;
  if (l_rad > M_PI)
    l_rad -= (2.0 * M_PI);

  gal.l = rad2deg(l_rad);
  gal.b = rad2deg(theta);
}
