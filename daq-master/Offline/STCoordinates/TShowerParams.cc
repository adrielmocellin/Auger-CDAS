#include "TShowerParams.h"
#include "STClibrary.h"
#include "TCdasUtil.h"
#include "math.h"
#include <iostream>
#include <cstring>

using namespace TSdParams;

TShowerParams::TShowerParams():
  fT0(0),            
  fU(0),             
  fV(0),             
  fXCore(0),         
  fYCore(0),         
  fSRef(0),          
  fRefDist(0),       
  fLDFRcut(0),       
  fXmax(0),          
  fR(0),             
  fChi2(0),          
  frChi2(0),         
  fddl(0),           
  fBeta(0),          
  fFitBeta(false),         
  fGamma(0),         
  fE(0),             
  fdE(0),            
  fZCore(0),         
  fGPSSecond(0)
{
  for (unsigned int i = 0; i<8; ++i)
    for (unsigned int j = 0; j<8; ++j)
      fVarCov[i][j] = 0;  
  
}




double TShowerParams::W() const{
  double a=fU*fU+fV*fV;
  if (a<=1)
    return sqrt(1-a);
  return nan("nan");
 
}

double TShowerParams::W(double margin){
  double a=fU*fU+fV*fV;
  if (a<=1)
    return sqrt(1-a);
  if (a>1 && a<1+margin) {
    fU/=sqrt(a);
    fV/=sqrt(a);
    return 0;
  }
  return nan("nan");
}

static const double theta(double u, double v) {
  return asin(sqrt(u * u + v * v));
};

static const double phi(double u, double v) {
  return atan2(v, u);
};

double TShowerParams::Theta(string s) const {
  if (strstr(s.c_str(), "deg"))
    return theta(fU, fV) * kCdasUtil::RAD2DEG;
  else
    return theta(fU, fV);
};

double TShowerParams::Phi(string s) const {
  if (strstr(s.c_str(), "deg"))
    return phi(fU, fV) * kCdasUtil::RAD2DEG;
  else
    return phi(fU, fV);
};

double TShowerParams::dTheta(string s) const {
  double dt = sqrt(cos(Phi()) * cos(Phi()) * fVarCov[eU][eU]
                   + sin(Phi()) * sin(Phi()) * fVarCov[eV][eV]
                   +
                   2 * cos(Phi()) * sin(Phi()) * fVarCov[eU][eV]) /
              cos(Theta());
  if (strstr(s.c_str(), "deg"))
    return dt * kCdasUtil::RAD2DEG;
  else
    return dt;
};

double TShowerParams::dPhi(string s) const {
  double dp = sqrt(cos(Phi()) * cos(Phi()) * fVarCov[eV][eV]
                   + sin(Phi()) * sin(Phi()) * fVarCov[eU][eU]
                   - 2 * cos(Phi()) * sin(Phi()) * fVarCov[eU][eV]) / sin(Theta());
  if (strstr(s.c_str(), "deg"))
    return dp * kCdasUtil::RAD2DEG;
  else
    return dp;
};

static void
uvwp(double fU, double fV, double fX, double fY, double *up, double *vp) {
  double w = sqrt(1 - fU * fU - fV * fV);
  *up = fU - fX * w * 2 * kSTC::Gamma;
  *vp = fV - fY * w * 2 * kSTC::Gamma;
  double wp = w + fX * fU * 2 * kSTC::Gamma + fY * fV * 2 * kSTC::Gamma;
  double n = sqrt((*up) * (*up) + (*vp) * (*vp) + wp * wp);
  *up /= n;
  *vp /= n;
  wp /= n;
};

double TShowerParams::LocalTheta(string s) const {
  double up, vp;
  uvwp(fU, fV, fXCore, fYCore, &up, &vp);
  if (strstr(s.c_str(), "deg"))
    return theta(up, vp) * kCdasUtil::RAD2DEG;
  else
    return theta(up, vp);
};

double TShowerParams::LocalPhi(string s) const {
  double up, vp;
  uvwp(fU, fV, fXCore, fYCore, &up, &vp);
  if (strstr(s.c_str(), "deg"))
    return phi(up, vp) * kCdasUtil::RAD2DEG;
  else
    return phi(up, vp);
};

double TShowerParams::Easting() const {
  double n, e;
  xy2ne(fXCore, fYCore, &n, &e);
  return e;
};

double TShowerParams::Northing() const {
  double n, e;
  xy2ne(fXCore, fYCore, &n, &e);
  return n;
};

double TShowerParams::L() const {
  double l,b;
  auger2gal(0,0,fGPSSecond,Theta(),Phi(),&l,&b);
  while (l>180)
    l-=360;  // [-180:180]
  while (l<-180)
    l+=360;
  return l;
}

double TShowerParams::B() const {
  double l,b;
  auger2gal(0,0,fGPSSecond,Theta(),Phi(),&l,&b);
  return b;
}

double TShowerParams::Ra() const {
  double ra,dec;
  auger2radec(0,0,fGPSSecond,Theta(),Phi(),&ra,&dec);
  return ra;
}

double TShowerParams::Dec() const {
  double ra,dec;
  auger2radec(0,0,fGPSSecond,Theta(),Phi(),&ra,&dec);
  return dec;
}
