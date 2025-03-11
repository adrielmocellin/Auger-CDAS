
//
// File   : T3NotifyStat.cc
//
// Purpose: Implementation of the class T3NotifyStat
//
// $Id$
//


#include <iostream>
#include <iomanip>

#include <sys/time.h>
#include <unistd.h>

/** @file T3NotifyStat.cc
  * Implementation of the class T3NotifyStat.
  * @author H.-J. Mathes, FzK
  */

#include <TBuffer.h>

#include <TimeConvert.hh>

#include "CheckVersion.hh"
#include "T3NotifyStat.hh"

using namespace std;
using namespace FdUtil;

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// 
// class T3NotifyStat - implementation
// 

T3NotifyStat::T3NotifyStat()
 {
  SetCurrentTime( fCdasSentTime );
  SetCurrentTime( fEventTime );
  SetCurrentTime( fT3DoneTime );
  SetCurrentTime( fT3StartTime );
}

// -------------------------------------------------------------------------

UInt_t T3NotifyStat::GetCdasSentDelay() const
 {
  return (fCdasSentTime.fLong - fT3DoneTime.fLong) / 1000;
}

// -------------------------------------------------------------------------

UInt_t T3NotifyStat::GetT3StartDelay() const
 {
  return (fT3StartTime.fLong - fEventTime.fLong) / 1000;
}

// -------------------------------------------------------------------------

UInt_t T3NotifyStat::GetT3ProcessingTime() const
 {
  return (fT3DoneTime.fLong - fT3StartTime.fLong) / 1000;
}

// -------------------------------------------------------------------------

void T3NotifyStat::SetCurrentTime(FakedLongRec& rec)
 {
  struct timezone tz;
  struct timeval tv;
  
  gettimeofday( &tv, &tz );
  
  rec.fLong =   1000000000 * (long long)
                TimeConvert::GetGPSTimeFromUnixTime( tv.tv_sec )
              + 1000 * (long long)tv.tv_usec;
}

// -------------------------------------------------------------------------

void T3NotifyStat::SetEventTime(UInt_t sec, UInt_t nano)
 {
  if ( !sec )
    SetCurrentTime( fEventTime );
  else
    fEventTime.fLong = 1000000000 * (long long)sec + (long long)nano;
}

// -------------------------------------------------------------------------

void T3NotifyStat::SetT3DoneTime(UInt_t sec, UInt_t nano)
 {
  if ( !sec )
    SetCurrentTime( fT3DoneTime );
  else
    fT3DoneTime.fLong = 1000000000 * (long long)sec + (long long)nano;
}

// -------------------------------------------------------------------------

void T3NotifyStat::SetT3StartTime(UInt_t sec, UInt_t nano)
 {
  if ( !sec )
    SetCurrentTime( fT3StartTime );
  else
    fT3StartTime.fLong = 1000000000 * (long long)sec + (long long)nano;
}

// -------------------------------------------------------------------------

void T3NotifyStat::SetCdasSentTime(UInt_t sec, UInt_t nano)
 {
  if ( !sec )
    SetCurrentTime( fCdasSentTime );
  else
    fCdasSentTime.fLong = 1000000000 * (long long)sec + (long long)nano;
}

// -------------------------------------------------------------------------

void T3NotifyStat::Show(ostream& ostr) const
 {
  T3Notify::Show( ostr );
  
  ostr << "-------------------- T3NotifyStat --------------------" << endl;
  
  ostr << "Event at:        " << setw(16) << fEventTime.fLong << endl;
  ostr << "T3 started at:   " << setw(16) << fT3StartTime.fLong 
       << " dt= " << setw(9) << GetT3StartDelay() << " usec" << endl;
  ostr << "T3 finished at:  " << setw(16) << fT3DoneTime.fLong
       << " dt= " << setw(9) << GetT3ProcessingTime() << " usec" << endl;
  
  if ( fCdasSentTime.fLong > fT3StartTime.fLong ) {
    ostr << "Sent to CDAS at: " << setw(16) << fCdasSentTime.fLong
         << " dt= " << setw(9) << GetCdasSentDelay() << " usec" << endl;
    ostr << "Total delay:      " << setw(9) 
         << (fCdasSentTime.fLong - fEventTime.fLong)/1000 << " usec" << endl;
  }
  
  ostr << "------------------------------------------------------" << endl;
}

// -------------------------------------------------------------------------

void T3NotifyStat::Streamer(TBuffer &R__b)
 {
   // Stream an object of class T3NotifyStat.

   UInt_t R__s, R__c;
   
   if ( R__b.IsReading() ) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if ( R__v ) {
        // cerr << "Version mismatch in T3NotifyStat::Streamer()" << endl;
      }
      T3Notify::Streamer(R__b);
      R__b >> fCdasSentTime;
      R__b >> fEventTime;
      R__b >> fT3DoneTime;
      R__b >> fT3StartTime;
      R__b.CheckByteCount(R__s, R__c, T3NotifyStat::IsA());
   }
   else {
      R__c = R__b.WriteVersion(T3NotifyStat::IsA(), kTRUE);
      T3Notify::Streamer(R__b);
      R__b << fCdasSentTime;
      R__b << fEventTime;
      R__b << fT3DoneTime;
      R__b << fT3StartTime;
      R__b.SetByteCount(R__c, kTRUE);
   }
}

// -------------------------------------------------------------------------

TBuffer& operator<<(TBuffer& buf, T3NotifyStat::FakedLongRec& l)
 {
  buf << l.f2Long.fInt1;
  buf << l.f2Long.fInt2;
  return buf;
}

// -------------------------------------------------------------------------

TBuffer& operator>>(TBuffer& buf, T3NotifyStat::FakedLongRec& l)
 {
  buf >> l.f2Long.fInt1;
  buf >> l.f2Long.fInt2;
  return buf;
}

ClassImp(T3NotifyStat)

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
