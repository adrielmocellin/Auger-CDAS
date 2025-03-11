#ifndef _STCLIBRARY_H_
#define _STCLIBRARY_H_

/**
 * @file STClibrary.h
 * @brief this file defines functions to deal with time and space coordinates
 * @author Benoit Revenu
 * 
 * Time and dates:\n
 * The julian day is a continuous count of days and fractions from the
 * beginning of the year -4712. By tradition, the julian day begins at 
 * Greenwich mean noon, that is, at 12h UT (Universal Time).
 * the variable "day" is always expressed in unit and fraction of day.
 * \n
 * Coordinates:\n
 * Basic coordinates conversion functions :\n
 * ************* ATTENTION ******************\n
 * ALL the angles (longitude, latitude, lambda, beta, l, b, dec) are\n
 * given in decimal degrees EXCEPT ra (right\n
 * ascension) which is in decimal hours.\n
 * ********************************************\n
 * Remember : the longitude is measured positively eastwards.\n
 * the azimuth is measured westwards from the South.\n
 
 * @todo take into account the precession of the equinox and allow the
 * positions with respect to any equinox
 */

#include <stdio.h>
#include <stdlib.h>
#include "STCconstants.h"
#include "LatLong-UTMConversion.h"

/**
 *  * @brief this function converts sexagesimal values into decimal value
 *   * @return the decimal value
 *    */
double sexa2dec(int hr, int min, double sec);

/**
 *  * @brief this function converts a decimal value into sexagesimal values
 *   */
void dec2sexa(double dec, int * hr, int * min, double *sec);

/**
 *  * @brief this function is the modulo function
 *   * @return x mod y
 *    */
double mod(double x, double y = 0);

/**
 * @brief conversion of UTC hour, min, sec in fraction of day
 * @return the fraction of the day between 0 and 1
 *
 * given the hour, min and sec, converts into the fraction of the day
 */
double hms2day(int hour, int min, double sec);

/**
 * @brief conversion of a fraction of day into hour, min and sec
 *
 * given the fraction of the day between 0 and 1, computes the hour, min and sec
 */
void day2hms(double day, int * hour, int * min, double * sec);

/**
 * @brief converts gps seconds since january 6, 1980 00h00 UTC into date (UTC)
 * @param gps_seconds is the value of time given by the GPS receiver+ns
 * @param year is the corresponding year
 * @param month is the corresponding month
 * @param day is the corresponding day
 * @param time is the corresponding UTC-time
 * @param leap is the current leap second value
 */
void gps2utcdate(double gps_seconds, int * year, int * month, int * day,
                 double * time, int leap);

/**
 * @brief converts a calendar date to gps. WARNING, if the date is previous 
 * to the 6th of January 1980, the function executes a exit(1)
*/
void date2gps(int year, int month, int day, int hour, int min, int sec, unsigned long &gps_seconds);

/**
 * @brief converts unix time to julian day
 */
void unix2jd(double unixseconds, double * jd);

/**
 * @brief converts julian day to unix time
 */
void jd2unix(double jd, double * unixseconds);

/**
 * @brief converts unix time to utc date
 */
void unix2date(double unixseconds, int* year, int* month, int* day, double* utc);

/**
 * @brief conversion of a UTC date (with decimal day) into Julian Date
 */
void date2jd(int year, int month, double day, double * jd);

/**
 * @brief conversion of a UTC date (with decimal hour) into Julian Date
 */
void date2jd(int year, int month, int day, double hour, double * jd);

/**
 * @brief conversion of GPS second in Julian Date
 */
void gps2jd(unsigned int gps_seconds, double *jd);

/**
 * @brief conversion of GPS second in Julian Date
 * the complete jd is gpsstart+fracjd, can be useful in case you need
 * high accuracy
 */
void gps2jd(unsigned int gps_seconds, double *gpsstart, double *fracjd);

/**
 * @brief conversion of a date in UTC into Modified Julian Date
 *
 * the modified Julian day (MJD) is Julian day - 2400000.5. \n
 * Contrarily to the JD, the MJD begins at Greenwich mean midnight !
 */
void date2mjd(int year, int month, double day, double * mjd);

/**
 * @brief reciprocal of date2jd
 */
void jd2date(double jd, int * year, int * month, double * day);

/**
 * @brief returns the amount of time in days between 2 julian dates
 * @return the date interval in days jd1-jd2
 */
double dateinterval(double jd1, double jd2);

/**
 * @brief same as before after a conversion with date2jd
 */
double dateinterval(int year, int month, double day,
                    int year1, int month1, double day1);

/**
 * @brief given a date, calculates the day of the week (0=sunday)
 * @return the day of the week (from 0=sunday to 7=saturday)
 */
int daynamefromdate(int year, int month, double day);

/**
 * @brief day number in the year
 */
int date2daynumber(int year, int month, int day);

/**
 * @brief given a year and a the number of the day on the year, converts into month and day
 * 
 * reciprocal of date2daynumber
 */
void daynumber2date(int year, int daynb, int * month, int * day);

/**
 * @brief check wether the given year is leap of not
 * @return 1 or 0 wether the input year is a leap year or not
 */
int leapyear(int year);

/**
 * @brief compute the easter sunday of the input year
 */
void easterdate(int year, int * month, int * day);

/**
 * @brief converts the Julian Date (UTC) into the LOCAL sidereal time (in hours)
 * (for the Greenwich sidereal time, just set longitude to zero)
 *
 * the result lst is in decimal hours the longitude is the
 * geographical longitude of the observation site and is expressed in
 * decimal degrees; the longitudes are measured POSITIVELY EASTWARDS
 * for instance, Washington=-77, Vienna=+16 (opposite to J. Meeus
 * book). This convention is the same than that of the IAU but is the
 * opposite than those of the other planets.
 * refjd is useful when you need high accuracy so that the full jd is
 * jd+refjd
 */
void ct2lst(double longitude, double jd, double * lst, double refjd=0);
// *********************************************************************************


// coordinates *********************************************************************
/**
 * @brief computes the precession coefficients between the two equinoxes
 * the output coefficients are in arcseconds (decimal)
 */
void precess_coeffs(int equinox_input, int equinox_output,
                    double * zeta, double * z, double * theta);

/**
 * @brief compute the value of the ecliptic obliquity angle for this value of JD
 * @return the value of the ecliptic obliquity in decimal degrees
 */
double ecliptic_obliquity(double jd);


/**
 * @brief converts radec into ecliptic coordinates
 */
void radec2ecl(double ra, double dec, double * lambda,
               double * beta, int eqn=kSTC::RefEquinox);

/**
 * @brief converts ecliptic into radec coordinates
 */
void ecl2radec(double lambda, double beta, double * ra,
               double * dec, int eqn=kSTC::RefEquinox);

/**
 * @brief converts radec into galactic coordinates
 */
void radec2gal(double ra, double dec, double * l,
               double * b, int eqn=kSTC::RefEquinox);

/**
 * @brief converts galactic into radec coordinates
 */
void gal2radec(double l, double b, double * ra,
               double * dec, int eqn=kSTC::RefEquinox);

/**
 * @brief converts azimuth-elevation into radec
 *  if refjd is != 0 then the full julian day is jd+refjd
 */
void azel2radec(double latitude, double longitude, double az, double el,
                double jd, double * ra, double * dec, double refjd=0);

/**
 *  @brief converts radec into azimuth-elevation coordinates
 *  if refjd is != 0 then the full julian day is jd+refjd
 */
void radec2azel(double latitude, double longitude, double ra, double dec,
                double jd, double * az, double * el, double refjd=0);

/**
 * @brief converts ecliptic coordinates into galactic coordinates
 */
void ecl2gal(double lambda, double beta, double * l, double * b, int eqn=kSTC::RefEquinox);

/**
 * @brief converts galactic into ecliptic coordinates
 */
void gal2ecl(double l, double b, double * lambda, double * beta,
             int eqn=kSTC::RefEquinox);

/**
 * @brief converts ecliptic coordinates into azimuth and elevation
 *  if refjd is != 0 then the full julian day is jd+refjd
 */
void ecl2azel(double latitude, double longitude, double lambda,
              double beta, double jd, double * az, double * el, double refjd=0);

/**
 * @brief converts azimuth-elevation into ecliptic coordinates
 *  if refjd is != 0 then the full julian day is jd+refjd
 */
void azel2ecl(double latitude, double longitude, double az, double el,
              double jd, double * lambda, double * beta, double refjd=0);

/**
 * @brief converts galactic coordinates into azimuth and elevation
 *  if refjd is != 0 then the full julian day is jd+refjd
 */
void gal2azel(double latitude, double longitude, double l, double b,
              double jd, double * az, double * el, double refjd=0, int eqn=kSTC::RefEquinox);


/**
 * @brief converts azimuth-elevation into galactic coordinates
 *  if refjd is != 0 then the full julian day is jd+refjd
 */
void azel2gal(double latitude, double longitude, double az, double el,
              double jd, double * l, double * b, double refjd=0, int eqn=kSTC::RefEquinox);

/**
 * @brief converts auger data into galactic coordinates
 */
void auger2gal(double x, double y, unsigned int sec, double theta, double phi,
               double * l, double * b, int eqn=kSTC::RefEquinox);

/**
 * @brief converts auger data into radec
 */
void auger2radec(double x, double y, unsigned int sec, double theta, double phi,
                 double * ra, double * dec);

#endif
