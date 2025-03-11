#ifndef _STCCONSTANTS_H_
#define _STCCONSTANTS_H_
#include <string>
/**
 * @file STCconstants.h
 * @brief this file contains the main constants for the computation
 * of orbital elements
 */

namespace kSTC {
/// Constants for changes in coordinates
  // UTM - xyz - LatLong part
const std::string AugerUTMZone("19H");
const int  WGS84  = 23;       ///< UTM stuff
const double Alpha = 0.00252;
const double Beta  = 0.000603;
const double Gamma = 0.00000007853;     ///< UTM conversion constants
  /// UTM -> cartesian conversion, easting and northing references (69.25,35.25 cf GAP 2001-038)
const double E0  = 477256.0;
const double N0  = 6099203.0;
const double A0  =   1400.0;

const double NorthDeclination = 3.53;
  // hour - rad convertion part
  /**
   *  * @brief converts hours (longitude for instance) into degrees
   *   */
const double Hrtod = 15.;
  /**
   *  * @brief converts degrees into hours
   *   */
const double Dtohr = 0.066666666666666;
  /**
   *  * @brief converts radians into hours
   *   */
const double Rtohr = 3.8197186342;
  /**
   *  * @brief converts hours into radians
   *   */
const double Hrtor = 0.261799387799;
  /**
     @brief is the julian date of 1/1/1970 UTC (origin for unix seconds)
  */
const double UnixStartJD = 2440587.5;
  // astro part: ecliptic and galactic values at J2000. All values are in decimal degrees.
  /**
   * @def kRefEquinox
   * @brief is the reference equinox for the computations
   */
const int RefEquinox = 2000;
  /**
   * @def eps_2000
   * @brief ecliptic obliquity  = 23 deg 26' 21''.448 (J2000)
   */
const double Eps_2000 = 23.4392911111;
  /**
   * @def alphaG_2000
   * @brief right ascension of galactic North Pole
   */
const double AlphaG_2000 = 192.85948;
  /**
   * @def deltaG_2000
   * @brief declination of galactic North Pole
   */
const double DeltaG_2000 = 27.12825;
  /**
   * @def lomega_2000
   * @brief galactic longitude of celestial equator
   */
const double Lomega_2000 = 32.93192;
  /**
   * @def alphaE_2000
   * @brief ecliptic longitude of galactic North Pole
   */
const double AlphaE_2000 = 180.02322;
  /**
   * @def deltaE_2000
   * @brief ecliptic latitude of galactic North Pole
   */
const double DeltaE_2000 = 29.811438523;
  /**
   * @def Eomega_2000
   * @brief galactic longitude of ecliptic equator
   */
const double Eomega_2000 = 6.3839743;

  // for GPS
  /**
   * @def GPSSTART
   * @brief value of january 6, 1980, 00h00 UT in julian days
   */
const double GPSSTART = 2444244.5;
  /**
   * @def J2000
   * @brief is the value of J2000 in Julian days; a julian year is 365.25 days
   */
const double J2000 = 2451545.0;
}

#endif
