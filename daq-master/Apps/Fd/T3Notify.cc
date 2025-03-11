
//
// File   : T3Notify.cc
//
// Purpose: Implementation of the class T3Notify
//
// $Id$
//


#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>


/** @file T3Notify.cc
  * Implementation of the class T3Notify.
  * @author H.-J. Mathes, FzK
  */
  
#include <TBuffer.h>
#include <TClass.h>  // TClass::ReadBuffer(), TClass::WriteBuffer()

//#include <ErrorLogging.hh>

#include "CheckVersion.hh"

#include "T3Notify.hh"

using namespace std;

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
// class T3Notify - implementation
//

T3Notify::T3Notify()
 {
  _Init();
}

// -------------------------------------------------------------------------

void T3Notify::_Init()
 {
  fT3Id        = 0;
  fEyeId       = 1;
  fT3GPSTime   = 0;
  fT3NanoTime  = 0;
  fSDPAngle    = 0.0;
  
  fSDPTheta    = 0.0;        // new for v2
  fSDPPhi      = 0.0;
  
  fNumPixels   = 0;          // new for v3

  fTotalSignal = 0.0;        // new for v4
}

// -------------------------------------------------------------------------

static string ShowBytes(const unsigned char *data,size_t size,bool show=false)
 {
  ostringstream bytes;
  
  if ( show ) {
    bytes << "   ";
    if ( size < sizeof(int) ) {
      for ( size_t i=0; i<(sizeof(int) - size); ++i )
        bytes << "     ";
    }
    const unsigned char *cp = data;
    while ( cp < (data + size) ) {
      bytes << " 0x" << hex << setfill('0') << setw(2) 
            << (unsigned int)*cp << setfill(' ') << dec;
      ++cp;
    }
  }
  
  return bytes.str();
}

// -------------------------------------------------------------------------

void T3Notify::Show(ostream& ostr) const
 {
  // show also print binary values
  bool show = false;
  
  // show the T3Notify class members...
  ostr << "------  T3Notify  ------" << endl;
  
  // this is the version (of the class) at compile time !!!
  ostr << "Class/StreamerVers: " << setw(2) << T3Notify::Class_Version() 
       << " - " << setw(2) << fStreamerVersion << endl;
  
  ostr << "EyeId:       " << setw(10) << fEyeId
       << ShowBytes((const unsigned char *)&fEyeId, sizeof(fEyeId), show) << endl;
  ostr << "T3Id:        " << setw(10) << fT3Id
       << ShowBytes((const unsigned char *)&fT3Id, sizeof(fT3Id), show) << endl;

  ostr << "GPSTime:     " << setw(10) << fT3GPSTime
       << ShowBytes((const unsigned char *)&fT3GPSTime, sizeof(fT3GPSTime), show) << endl;
  ostr << "NanoTime:    " << setw(10) << fT3NanoTime 
       << ShowBytes((const unsigned char *)&fT3NanoTime, sizeof(fT3NanoTime), show) << endl;
  ostr << "SDPAngle:    " << setw(10) << setprecision(3) << fSDPAngle 
       << ShowBytes((const unsigned char *)&fSDPAngle, sizeof(fSDPAngle), show) << endl;
  
  ostr << "SDPTheta:    " << setw(10) << setprecision(3) << fSDPTheta 
       << ShowBytes((const unsigned char *)&fSDPTheta, sizeof(fSDPTheta), show) << endl;
  ostr << "SDPPhi:      " << setw(10) << setprecision(3) << fSDPPhi 
       << ShowBytes((const unsigned char *)&fSDPPhi, sizeof(fSDPPhi), show) << endl;
  
  ostr << "fNumPixels:  " << setw(10) << fNumPixels 
       << ShowBytes((const unsigned char *)&fNumPixels, sizeof(fNumPixels), show) << endl;
  
  ostr << "fTotalSignal:" << setw(10) << fTotalSignal
       << ShowBytes((const unsigned char *)&fTotalSignal, sizeof(fTotalSignal), show) << endl;
       
  ostr << "------------------------" << endl;
}

// -------------------------------------------------------------------------

static void BufferShow(TBuffer& R__b,UInt_t R__c,ostream& ostr=std::cout);

void T3Notify::Streamer(TBuffer &R__b)
 {
  // Stream an object of class T3Notify.

  UInt_t R__s, R__c;
  
  if ( R__b.IsReading() ) {
    
    Version_t R__v = fStreamerVersion = R__b.ReadVersion(&R__s, &R__c);

#if 0
    FD_COUT << "T3Notify::Streamer() - R__v= " << R__v 
            << " R__s= " << R__s << " R__c= " << R__s << endl;

#if 0
    BufferShow( R__b, R__c - sizeof(R__v), 
                FD_LOGSTREAM(FdUtil::ErrorLogger::eInfo) );
    
    R__b.CheckByteCount(R__s, R__c, T3Notify::IsA());
    R__b.SetBufferOffset(-(R__c - sizeof(R__v)));
#endif
#endif
    
    if ( R__v == FdCdasVERSIONv1 ) {

      // if ( fgPrintLevel )
      //   cerr << "T3Notify::Streamer() - fix for version= 1 !" << endl;

      TObject::Streamer(R__b);
      
      R__b >> fT3Id;
      R__b >> fEyeId;
      R__b >> fT3GPSTime;
      R__b >> fT3NanoTime;
      R__b >> fSDPAngle;
      R__b.CheckByteCount(R__s, R__c, T3Notify::IsA());
    }
    else if ( R__v == FdCdasVERSIONv2 ) {

      // if ( fgPrintLevel )
      //  cerr << "T3Notify::Streamer() - fix for version= 2 !" << endl;

      TObject::Streamer(R__b);
      
      R__b >> fT3Id;
      R__b >> fEyeId;
      R__b >> fT3GPSTime;
      R__b >> fT3NanoTime;
      R__b >> fSDPAngle;
      R__b >> fSDPTheta;
      R__b >> fSDPPhi;
      R__b.CheckByteCount(R__s, R__c, T3Notify::IsA());
    }
    else { // FdCdasVERSIONv3++ --> ROOT class evolution scheme
      T3Notify::Class()->ReadBuffer(R__b, this, R__v, R__s, R__c);
    }
  } else { // support only new Streamer scheme of ROOT v 3.x
    
    T3Notify::Class()->WriteBuffer(R__b, this);
  }
}

ClassImp(T3Notify)

// -------------------------------------------------------------------------

/** This method creates a hex dump of the 'payload' of an area in the Tbuffer.
  */
static void BufferShow(TBuffer& R__b,UInt_t R__c,ostream& ostr)
 {
  unsigned int i, j;
  unsigned char *data = new unsigned char[R__c+1];
  
  if ( !data ) {
    //FD_CERR << __FILE__ << ": BufferShow() - new[] failed!" << endl;
    return;
  }
  
  for ( i=0; i<R__c; i++ )
    R__b >> data[i];
  
  for ( i=0; i<R__c; i+=16 ) {
  
    // output hex numbers
    for ( j=i; j<i+16; j++ ) {
      if ( j<R__c )
        ostr << " " << setfill('0') << setw(2)
             << hex << (unsigned int)(unsigned char)data[j] << dec 
	     << setfill(' ');
      else
        ostr << "   ";
    }

    ostr << "   ";
    
    // output ASCII chars if feasible
    for ( j=i; j<i+16; j++ ) {
      if ( j<R__c ) {
        if ( isprint(data[j]) )
          ostr << data[j];
        else
          ostr << ".";
      }
    }
    ostr << endl;
  }

  ostr << endl;
  
  delete [] data;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
