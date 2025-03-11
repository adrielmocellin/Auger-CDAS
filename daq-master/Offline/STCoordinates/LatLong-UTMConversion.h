#ifndef _LATLONGCONV_H_
#define _LATLONGCONV_H_

/**
   @file LatLong-UTMConversion.h
   @author Benoit Revenu (using already existing code)
   @brief this files declares the class and functions used in the
   latitude-longitude and UTM coordinates conversions
*/

#include <string>
#include "STCconstants.h"

using namespace std;

/**
   @brief converts latitude-longitude into UTM. Lat and Long must be in degrees
*/
void LLtoUTM(int ReferenceEllipsoid, double Lat, double Long,
             double &UTMNorthing, double &UTMEasting, string& UTMZone);

/**
   @brief converts UTM into latitude-longitude
   @returns lat and long in degrees
*/
void UTMtoLL(int ReferenceEllipsoid, double UTMNorthing, double UTMEasting, const string& UTMZone,
             double& Lat,  double& Long );

/**
   @brief This routine determines the correct UTM letter designator for the
   given latitude in degrees
   
   @return 'Z' if latitude is outside the UTM limits
   of 84N to 80S Written by Chuck Gantz- chuck.gantz@globalstar.com
   The letters 'I' and 'O' are not used.
*/
char UTMLetterDesignator(double Lat);

/**
   @class Ellipsoid
	 @brief this class allows to change the reference ellipsoid used in
   the description of the shape of the earth
*/
class Ellipsoid {
public:
  Ellipsoid(); ///< default construcor
  Ellipsoid(int Id, string name, double radius, double ecc) {
    id = Id;
    ellipsoidName = name;
    EquatorialRadius = radius;
    eccentricitySquared = ecc;
  } ///< constructor

  int id; ///< ident of the reference ellipsoid (default = 23 = WGS-84)
  string ellipsoidName; ///< name of the reference ellipsoid
  double EquatorialRadius;
  /**< equatorial radius associated to this
     reference ellipsoid
  */
  double eccentricitySquared;  ///< square of eccentricity of the ellipsoid

};

// Usefull routines for Northing-Easting to X-Y convertions

void ne2xy(double n, double e, double *x, double *y);

void nea2xyz(double n, double e, double a, double *x, double *y, double *z);

void xy2ne(double x, double y, double *n, double *e);

void
xy2ne(double x, double y, double dx, double dy, double *n, double *e,
      double *dn, double *de);

#endif
