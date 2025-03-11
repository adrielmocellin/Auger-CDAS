/**
   @file LatLong-UTMConversion.cc
   @brief this files implements the elements declared in the header
   file LatLong-UTMConversion.h
 
   Reference ellipsoids derived from Peter H. Dana's website-
   http://www.utexas.edu/depts/grg/gcraft/notes/datum/elist.html
   Department of Geography, University of Texas at Austin Internet:
   pdana@mail.utexas.edu 3/22/95
   
   Source Defense Mapping Agency. 1987b. DMA Technical Report:
   Supplement to Department of Defense World Geodetic System 1984
   Technical Report. Part I and II. Washington, DC: Defense Mapping
   Agency
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "TCdasUtil.h"
#include "LatLong-UTMConversion.h"

/**
   @var static Ellipsoid ellipsoid
   @brief this is the various ellipsoids of reference which are
   commonly used to describe the shape of the earth
*/
static Ellipsoid ellipsoid[] =
  {
    //  id, Ellipsoid name, Equatorial Radius, square of eccentricity
    Ellipsoid( -1, "Placeholder", 0, 0),//placeholder only, To allow array indices to match id numbers
    Ellipsoid( 1, "Airy", 6377563, 0.00667054),
    Ellipsoid( 2, "Australian National", 6378160, 0.006694542),
    Ellipsoid( 3, "Bessel 1841", 6377397, 0.006674372),
    Ellipsoid( 4, "Bessel 1841 (Nambia) ", 6377484, 0.006674372),
    Ellipsoid( 5, "Clarke 1866", 6378206, 0.006768658),
    Ellipsoid( 6, "Clarke 1880", 6378249, 0.006803511),
    Ellipsoid( 7, "Everest", 6377276, 0.006637847),
    Ellipsoid( 8, "Fischer 1960 (Mercury) ", 6378166, 0.006693422),
    Ellipsoid( 9, "Fischer 1968", 6378150, 0.006693422),
    Ellipsoid( 10, "GRS 1967", 6378160, 0.006694605),
    Ellipsoid( 11, "GRS 1980", 6378137, 0.00669438),
    Ellipsoid( 12, "Helmert 1906", 6378200, 0.006693422),
    Ellipsoid( 13, "Hough", 6378270, 0.00672267),
    Ellipsoid( 14, "International", 6378388, 0.00672267),
    Ellipsoid( 15, "Krassovsky", 6378245, 0.006693422),
    Ellipsoid( 16, "Modified Airy", 6377340, 0.00667054),
    Ellipsoid( 17, "Modified Everest", 6377304, 0.006637847),
    Ellipsoid( 18, "Modified Fischer 1960", 6378155, 0.006693422),
    Ellipsoid( 19, "South American 1969", 6378160, 0.006694542),
    Ellipsoid( 20, "WGS 60", 6378165, 0.006693422),
    Ellipsoid( 21, "WGS 66", 6378145, 0.006694542),
    Ellipsoid( 22, "WGS-72", 6378135, 0.006694318),
    Ellipsoid( 23, "WGS-84", 6378137, 0.00669438)
  };

Ellipsoid::Ellipsoid():    
  id(0), 
  ellipsoidName(""),
  EquatorialRadius(0),
  eccentricitySquared(0)
{}

void LLtoUTM(int ReferenceEllipsoid, double Lat, double Long,
             double &UTMNorthing, double &UTMEasting, string& UTMZone) {
  //converts lat/long to UTM coords.  Equations from USGS Bulletin
  //1532 East Longitudes are positive, West longitudes are negative.
  //North latitudes are positive, South latitudes are negative Lat and
  //Long are in decimal degrees Written by Chuck Gantz-
  //chuck.gantz@globalstar.com

  double a = ellipsoid[ReferenceEllipsoid].EquatorialRadius;
  double eccSquared = ellipsoid[ReferenceEllipsoid].eccentricitySquared;
  double k0 = 0.9996;

  double LongOrigin;
  double eccPrimeSquared;
  double N, T, C, A, M;

  //Make sure the longitude is between -180.00 .. 179.9
  double LongTemp = (Long+180)-int((Long+180)/360)*360-180; // -180.00 .. 179.9;

  double LatRad = Lat*kCdasUtil::DEG2RAD;
  double LongRad = LongTemp*kCdasUtil::DEG2RAD;
  double cLatRad = cos(LatRad);
  double sLatRad = sin(LatRad);
  double tLatRad = sLatRad/cLatRad;
  double LongOriginRad;
  int    ZoneNumber;

  ZoneNumber = int((LongTemp + 180)/6) + 1;

  if( Lat >= 56.0 && Lat < 64.0 && LongTemp >= 3.0 && LongTemp < 12.0 )
    ZoneNumber = 32;

  // Special zones for Svalbard
  if( Lat >= 72.0 && Lat < 84.0 ) {
    if(      LongTemp >= 0.0  && LongTemp <  9.0 )
      ZoneNumber = 31;
    else if( LongTemp >= 9.0  && LongTemp < 21.0 )
      ZoneNumber = 33;
    else if( LongTemp >= 21.0 && LongTemp < 33.0 )
      ZoneNumber = 35;
    else if( LongTemp >= 33.0 && LongTemp < 42.0 )
      ZoneNumber = 37;
  }
  LongOrigin = (ZoneNumber - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone
  LongOriginRad = LongOrigin * kCdasUtil::DEG2RAD;

  //compute the UTM Zone from the latitude and longitude
  char utmz[4];
  sprintf(utmz, "%d%c", ZoneNumber, UTMLetterDesignator(Lat));
  UTMZone = utmz;

  eccPrimeSquared = (eccSquared)/(1-eccSquared);

  N = a/sqrt(1-eccSquared*sLatRad*sLatRad);
  T = tLatRad*tLatRad;
  C = eccPrimeSquared*cLatRad*cLatRad;
  A = cLatRad*(LongRad-LongOriginRad);

  double e4=eccSquared*eccSquared;
  double e6=e4*eccSquared;
  double e8=e4*e4;

  double c1 = 1-eccSquared/4
              -3*e4/64
              -5*e6/256
              -175*e8/16384;

  double c2 = -(3*eccSquared/8
                +3*e4/32
                +45*e6/1024
                +105*e8/4096);

  double c3 = 15*e4/256
              +45*e6/1024
              +525*e8/16384;

  double c4 = -(35*e6/3072
                +175*e8/12288);

  double c5 = 315*e8/131072;

  M = a*( c1*LatRad + c2*sin(2*LatRad) + c3*sin(4*LatRad) + c4*sin(6*LatRad) + c5*sin(8*LatRad) );

  double A2 = A*A;
  double A3 = A2*A;
  double T2 = T*T;

  UTMEasting = (double)(k0*N*(A+(1-T+C)*A3/6
                              + (5-18*T+T2+72*C-58*eccPrimeSquared)*A2*A3/120)
                        + 500000.0);

  UTMNorthing = (double)(k0*(M+N*tLatRad*
                             (A2/2+(5-T+9*C+4*C*C)*A*A3/24
                              + (61-58*T+T2+600*C-330*eccPrimeSquared)*A3*A3/720)));

  if(Lat < 0)
    UTMNorthing += 10000000.0; //10000000 meter offset for southern hemisphere

}

char UTMLetterDesignator(double Lat) {
  //This routine determines the correct UTM letter designator for the
  //given latitude returns 'Z' if latitude is outside the UTM limits
  //of 84N to 80S Written by Chuck Gantz- chuck.gantz@globalstar.com
  //	les lettres 'I' et 'O' sont sautees.
  char LetterDesignator;

  if((84 >= Lat) && (Lat >= 72))
    LetterDesignator = 'X';
  else if( ( Lat >= -80 ) && ( Lat < -32 ) )
    LetterDesignator = (char)((int)floor((Lat+80)/8)+'C');
  else if( ( Lat >= -32 ) && ( Lat < 8 ) )
    LetterDesignator = (char)((int)floor((Lat+32)/8)+'J');
  else if( ( Lat >= 8 ) && ( Lat < 72 ) )
    LetterDesignator = (char)((int)floor((Lat-8)/8)+'P');
  else if( ( Lat >= 72 ) && ( Lat <= 84 ) )
    LetterDesignator = 'X';
  else
    LetterDesignator = 'Z';

  return LetterDesignator;
}


void UTMtoLL(int ReferenceEllipsoid, double UTMNorthing, double UTMEasting, const string& UTMZone,
             double& Lat,  double& Long ) {
  double k0 = 0.9996;
  double a = ellipsoid[ReferenceEllipsoid].EquatorialRadius;
  double eccSquared = ellipsoid[ReferenceEllipsoid].eccentricitySquared;
  double eccPrimeSquared;
  double e1 = (1-sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));
  double N1, T1, C1, R1, D, M;
  double LongOrigin;
  double mu, phi1, phi1Rad;
  double x, y;
  int ZoneNumber;
  char* ZoneLetter;
  int NorthernHemisphere; //1 for northern hemispher, 0 for southern

  x = UTMEasting - 500000.0; //remove 500,000 meter offset for longitude
  y = UTMNorthing;

  ZoneNumber = strtoul(UTMZone.c_str(), &ZoneLetter, 10);
  if((*ZoneLetter - 'N') >= 0)
    NorthernHemisphere = 1;//point is in northern hemisphere
  else {
    NorthernHemisphere = 0;//point is in southern hemisphere
    y -= 10000000.0;//remove 10,000,000 meter offset used for southern hemisphere
  }

  LongOrigin = (ZoneNumber - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone

  eccPrimeSquared = (eccSquared)/(1-eccSquared);
  double e4=eccSquared*eccSquared;
  double e6=e4*eccSquared;
  double e8=e4*e4;
  double e1_2 = e1*e1;

  M = y / k0;
  mu = M/(a*(1-eccSquared/4-3*e4/64-5*e6/256
             -175*e8/16384));

  phi1Rad = mu	+ (3*e1/2-27*e1_2*e1/32)*sin(2*mu)
            + (21*e1_2/16-55*e1_2*e1_2/32)*sin(4*mu)
            +(151*e1*e1_2/96)*sin(6*mu)+(1097*e1_2*e1_2/512)*sin(8*mu);
  phi1 = phi1Rad*kCdasUtil::RAD2DEG;

  double cphi1 = cos(phi1Rad);
  double sphi1 = sin(phi1Rad);
  double tphi1 = sphi1/cphi1;

  N1 = a/sqrt(1-eccSquared*sphi1*sphi1);
  T1 = tphi1*tphi1;
  C1 = eccPrimeSquared*cphi1*cphi1;
  R1 = a*(1-eccSquared)/pow(1-eccSquared*sphi1*sphi1, 1.5);
  D = x/(N1*k0);
  double D2 = D*D;
  double D4 = D2*D2;
  double C1_2 = C1*C1;
  double T1_2 = T1*T1;

  Lat = phi1Rad - (N1*tphi1/R1)*
        (D2/2-(5+3*T1+10*C1-4*C1_2-9*eccPrimeSquared)*D4/24
         +(61+90*T1+298*C1+45*T1_2-252*eccPrimeSquared-3*C1_2)*D2*D4/720);
  Lat = Lat * kCdasUtil::RAD2DEG;

  Long = (D-(1+2*T1+C1)*D*D2/6+(5-2*C1+28*T1-3*C1_2+8*eccPrimeSquared+24*T1_2)*D*D4/120)/cphi1;

  Long = LongOrigin + Long * kCdasUtil::RAD2DEG;

}



void ne2xy(double n, double e, double *x, double *y) {
  /// cf GAP-2001-038
  *x = (1 + kSTC::Beta) * (e - kSTC::E0) + kSTC::Alpha * (n - kSTC::N0);
  *y = (1 + kSTC::Beta) * (n - kSTC::N0) - kSTC::Alpha * (e - kSTC::E0);
};

void nea2xyz(double n, double e, double a, double *x, double *y, double *z) {
  /// cf GAP-2001-038
  ne2xy(n, e, x, y);
  *z = a - kSTC::A0 - (pow(n - kSTC::N0, 2) +
                       pow(e - kSTC::E0, 2)) * kSTC::Gamma;
};

void xy2ne(double x, double y, double *n, double *e) {
  double upb = 1 + kSTC::Beta;
  double coeff = kSTC::Alpha * kSTC::Alpha + upb * upb;
  *n = kSTC::N0 + (kSTC::Alpha * x + upb * y) / coeff;
  *e = kSTC::E0 + (upb * x - kSTC::Alpha * y) / coeff;
};

void
xy2ne(double x, double y, double dx, double dy, double *n, double *e,
      double *dn, double *de) {
  double upb = 1 + kSTC::Beta;
  double coeff = kSTC::Alpha * kSTC::Alpha + upb * upb;
  *n = kSTC::N0 + (kSTC::Alpha * x + upb * y) / coeff;
  *e = kSTC::E0 + (upb * x - kSTC::Alpha * y) / coeff;
  *dn = (kSTC::Alpha * fabs(dx) + upb * fabs(dy)) / coeff;
  *de = (upb * fabs(dx) + kSTC::Alpha * fabs(dy)) / coeff;
};

