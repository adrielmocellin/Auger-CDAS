
//
// File   : T3Notify.hh
//
// Purpose: Declaration of the class T3Notify
//
// $Id$
//


/** @file T3Notify.hh
  * Declaration of the class T3Notify.
  * @author H.-J. Mathes, FzK
  */
  
  
#ifndef _T3Notify_hh_
#define _T3Notify_hh_

#include <iostream>

#include <TObject.h>

#include <CdasProtocol.hh>

/** Object which is sent to CDAS when a T3 trigger was detected.
  */
class T3Notify : public TObject {
public:
    /** First and last T3 id which we could use.
      */
    enum ET3Limits
     {
      /** Minimum allowed T3 id. */
      kMinT3Id = 4097,
      /** Maximum allowed T3 id. */
      kMaxT3Id = 8191
    };
    
    /** Default constructor of the class T3Notify. 
      *
      * The T3 id is set to an illegal default value of 0. The other variables 
      * are set to their defaults, too.
      */
    T3Notify();

    /** Destructor of the class T3Notify. */
    virtual ~T3Notify() {}

#if (defined __GNUC__) && !(__GNUC__ < 3)
    // don't override method of base class, avoid warning -Woverloaded-virtual
    using TObject::Dump;
#endif // __GNUC__

    /** Dump the T3 information to the specified ostream (cout by default). */
    virtual void Dump(std::ostream& ostr=std::cout)
     { ((T3Notify*)this)->Show(ostr); }

    /** Print the T3 information verbosely to the specified ostream
      * (cout by default).
      */
    virtual void Show(std::ostream& ostr=std::cout) const;

    /** Get the Id of the Eye which emitted the T3. */
    UInt_t GetEyeId() const
     { return fEyeId; }
    
    /** Get the number of T3 pixels for this T3. */
    UInt_t GetNPixels() const
     { return fNumPixels; }
     
    /** Get the azimuth angle of the SDP (Shower Detector Plane) of the
      * current T3.
      */
    Float_t GetSDPAngle() const
     { return fSDPAngle; }
    
    /** Zenith angle of the SDP normal in global cartesian (X= East, Y = North)
      * (spherical) coordinates.
      */    
    Float_t GetSDPTheta() const
     { return fSDPTheta;}
     
    /** Azimuth angle of the SDP normal in global cartesian (X= East, Y = North)
      * (spherical) coordinates.
      */        
    Float_t GetSDPPhi() const
     { return fSDPPhi;} 
     
    /** Get the total T3 signal. */
    Float_t GetTotalSignal()
     { return fTotalSignal; }
    
    /** Get the GPS time when the current T3 has happened. */
    UInt_t GetT3GPSTime() const
     { return fT3GPSTime; }

    /** Get the I3 id which was assigned to th ecurrent T3. This id is a number
      * which cycles periodic between 4097 and 8191.
      */
    UShort_t GetT3Id() const
     { return fT3Id; }

    /** Get the nano time (nsec resolution) of the current T3. */
    UInt_t GetT3GPSNanoTime() const
     { return fT3NanoTime; }

    /** Set the Id of the Eye from where the T3 was emitted. */
    void SetEyeId(UInt_t eye_id)
     { fEyeId = eye_id; }
    
    /** Set the number of T3 pixels for this T3 event. */
    void SetNPixels(UInt_t n_pixel)
     { fNumPixels = n_pixel; }
     
    /** Set the angle of the SDP (Shower Detector Plane) on ground which
      * should point towards the tanks in the field where the shower hit
      * the ground.
      */
    void SetSDPAngle(Float_t angle)
     { fSDPAngle = angle; }
    
     /** Set the Zenith angle of the SDP normal in global cartesian
       *  (X= East, Y = North)
       * (spherical) coordinates*/        
    void SetSDPTheta(Float_t sdptheta)
     { fSDPTheta = sdptheta; }
     
    /** Set the Azimuth angle of the SDP normal in global cartesian
      * (X= East, Y = North)
      * (spherical) coordinates*/        
    void SetSDPPhi(Float_t sdpphi)
     { fSDPPhi = sdpphi; } 
        
    /** Set the total T3 signal. */
    void SetTotalSignal(Float_t total_signal) 
     { fTotalSignal = total_signal; }
     
    /** Set the T3 id of the current object. */
    void SetT3Id(UShort_t t3_id)
     { fT3Id = t3_id; }

    /** Set the time when the T3 has happened. */
    void SetT3Time(UInt_t gps_time, UInt_t nano_time)
     { fT3GPSTime = gps_time; fT3NanoTime = nano_time; }

private:

    /** Initialize the data of a T3Notify object. */
    void _Init();

    Version_t     fStreamerVersion; //! version read from file
    
    UShort_t      fT3Id;       // Rolling T3 Id, 4096 < id < 8192
    UInt_t        fEyeId;      // Eye which produced the T3
    UInt_t        fT3GPSTime;  // GPS Time when the T3 occurred
    UInt_t        fT3NanoTime; // Time in nano sec when the T3 occurred
    Float_t       fSDPAngle;   // Angle of SDP on ground level
                               // in radian, counted from East
    Float_t       fSDPTheta;   // Zenith angle of the SDP Normal
    Float_t       fSDPPhi;     // Azimuth angle of the SDP Normal
    
    UInt_t        fNumPixels;  // Number of pixels found in T3
    Float_t       fTotalSignal; // Total signal in T3 pixels
    
    /* next might follow some physics infomation ... */
    
    ClassDef(T3Notify,FdCdasVERSIONv4)
};

#endif  // _T3Notify_hh_
