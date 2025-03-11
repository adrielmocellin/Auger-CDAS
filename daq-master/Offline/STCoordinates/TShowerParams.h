#ifndef _SHOWER_PARAMS_
#define _SHOWER_PARAMS_

#include <string>
#include <math.h>
using namespace std;

/**
   @file TShowerParams.h
*/

//! TShowerParams class : contains shower parameters after fit

namespace TSdParams {
  enum ShowersParams { eT0, eU, eV, eXCore, eYCore, eSRef, eXmax, eR };
};
/// TShowerParams class : contains shower parameters after fit
class TShowerParams {
public:
 TShowerParams();
  virtual ~ TShowerParams() {};
  
  /// Shower Parameters expected to be common. All are defined in Auger Official Referential (cf GAP 2001-038)
  double fT0;                    ///< Time of the core in nanosecond with respect to fRecSecond;
  double fU;                  ///< reconstructed u = sin(theta)*cos(phi)
  double fV;                  ///< reconstructed v = sin(theta)*sin(phi)
  double fXCore;              ///< X position of the core
  double fYCore;              ///< Y position of the core
  double fTCore;              ///< T position of the core
  double fSRef;              ///< signal in VEM at fRefDist m from the core
  double fRefDist;            ///< Infill Mod: fSRefDist=450 m if event.UseInfillRec=1  
                              ///  Normal Mod: fRefDist=1000 m if event.UseInfillRec=0  
  double fLDFRcut;            ///< Infill Mod: cut of LDF function 
  double fXmax;               ///< maximum of the shower (vertical)
  double fR;                  ///< curvature radius
   
  // Last fit information
  double fVarCov[8][8];       ///< covariance matrix of the above parameters in the same order!
  double fChi2;               ///< chi2 square of the last fit
  double frChi2;              ///< is the reduced chi-square (chi2/ddl)
  double fddl;                ///< number of degrees of freedom (is the number of tanks of the event)

  // Relating to the ldf
  double fBeta;               ///< ldf slope
  bool fFitBeta;              ///< tells if beta is fix or fitted
  double fGamma;              ///< ldf correction to power law

  // Energy
  double fE;                  ///< Energy of the shower (EeV)
  double fdE;                 ///< error in energy

  // ZCore
  double fZCore;              ///< Z position of the core

  // GPS second
  unsigned int fGPSSecond;    ///< second of the shower (in GPS)

  // Errors on parameters
  double dT0() const {
    return sqrt(fVarCov[TSdParams::eT0][TSdParams::eT0]);
  } ///< error on fT0 (in second)
  double dU() const {
    return sqrt(fVarCov[TSdParams::eU][TSdParams::eU]);
  } ///< error on U
  double dV() const {
    return sqrt(fVarCov[TSdParams::eV][TSdParams::eV]);
  } ///< error on V
  double dXCore() const {
    return sqrt(fVarCov[TSdParams::eXCore][TSdParams::eXCore]);
  } ///< error on XCore
  double dYCore() const {
    return sqrt(fVarCov[TSdParams::eYCore][TSdParams::eYCore]);
  } ///< error on YCore
  double dXYCoreCorrelation() const {
    return fVarCov[TSdParams::eYCore][TSdParams::eXCore];
  }
  double dSRef() const {
    return sqrt(fVarCov[TSdParams::eSRef][TSdParams::eSRef]);
  } ///< error on fSRef
  double dXmax() const {
    return sqrt(fVarCov[TSdParams::eXmax][TSdParams::eXmax]);
  } ///< error of Xmax
  double dR() const {
    return sqrt(fVarCov[TSdParams::eR][TSdParams::eR]);
  } ///< error of R

  // Functions to get other parameters
  double Northing() const;          ///< Northing of the core
  double dNorthing() const {
    return dYCore();
  }
  double Easting() const;           ///< Easting of the core
  double dEasting() const {
    return dXCore();
  }
  
  /// sqrt(1-u^2-v^2) allowing for a margin (rescale u and v if necessary). Returns nan else
  double W(double margin);
  double W() const;

  // Get sky information
  double L() const;
  double B() const;
  double Ra() const;
  double Dec() const;

  /// Theta, Phi are defined in Auger Official Referential (cf GAP 2001-038)
  double Theta(string s = "rad") const;     ///< zenith angle of the shower, in radians, in Auger frame
  double dTheta(string s = "rad") const;    ///< error on zenithal angle
  double Phi(string s = "rad") const;       ///< "azimuth" of the shower, from north measured eastwards : east = 0, north = + Pi/2
  double dPhi(string s = "rad") const;      ///< error on azimuth

  /// These local values are with respect to vertical at core position
  double LocalTheta(string s = "rad") const;        ///< zenithal angle of the shower, in radians, in local frame
  double dLocalTheta(string s = "rad") const {
    return dTheta(s);
  } // error on local zenith == error on zenith
  double LocalPhi(string s = "rad") const;  ///< "azimuth" of the shower, from north measured eastwards : east = 0, north = + Pi/2
  double dLocalPhi(string s = "rad") const {
    return dPhi(s);
  } // error on local azimtuh == error on azimuth
};

#endif
