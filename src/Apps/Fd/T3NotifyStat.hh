
//
// File   : T3NotifyStat.hh
//
// Purpose: Declaration of the class T3NotifyStat
//
// $Id$
//



/** @file T3NotifyStat.hh
  * Declaration of the class T3NotifyStat.
  * @author H.-J. Mathes, FzK
  */


#ifndef _FdCdas_T3NotifyStat_hh_
#define _FdCdas_T3NotifyStat_hh_

#include <iostream>

#include <TObject.h>

#include <CdasProtocol.hh>

#ifndef __CINT__
# include <T3Notify.hh>
#endif  // __CINT__

/** This class is intended to account for the timing of the T3 process'
  * activities. Therefor it contains some (time) statistics variables.
  *
  * <b>Note:</b> All times should be set/passed in the GPS time scale !
  */
class T3NotifyStat : public T3Notify {
public:
    /** Constructor of the class T3NotifyStat. All timing variables are set to
      * the current time.
      */
    T3NotifyStat();

    /** Return the time [usec] difference between the time when the T3 was 
      * finished and the time after the T3 was sent to CDAS.
      */
    UInt_t GetCdasSentDelay() const;
    
    /** Return the time difference [usec] between the event time and the T3 
      * start.
      */
    UInt_t GetT3StartDelay() const;
    
    /** Return the time [usec] which the T3 processor used for its calculation 
      * and decision taking.
      */
    UInt_t GetT3ProcessingTime() const;
    
    /** Set the time when the event was generated.
      */
    void SetEventTime(UInt_t sec=0,UInt_t nano=0);

    /** Set the time when the T3 processing is done.
      */
    void SetT3DoneTime(UInt_t sec=0,UInt_t nano=0);
    
    /** Set the time when the T3 processor received the event and started
      * with the processing.
      */
    void SetT3StartTime(UInt_t sec=0,UInt_t nano=0);

    /** Set the time when the T3 was sent to CDAS. This takes therefor the
      * time (on the local system) to sent the message into account.
      */
    void SetCdasSentTime(UInt_t sec=0,UInt_t nano=0);

    /** Display the T3 time statistics information to the ostream (cout.)
      */
    void Show(std::ostream& ostr=std::cout) const;

private:
    /** We use this union to hide the long long which we want to stream against
      * cint who still doesn't know how to handle it.
      */
    typedef union _FakedLong {
#ifndef __CINT__
      long long fLong;
#endif // __CINT__
      struct {
        UInt_t fInt1;
	UInt_t fInt2;
      } f2Long;
    } FakedLongRec, *FakedLong;

    /** Set to the current time in GPS scale.
      */
    static void SetCurrentTime(FakedLongRec&);

    //FakedLongRec      fCdasRecvTime; // time when T3 was received by CDAS
    FakedLongRec      fCdasSentTime; // time when T3 was sent to CDAS
    FakedLongRec      fEventTime;    // time when the event happened
    FakedLongRec      fT3DoneTime;   // time when T3 decision was done
    FakedLongRec      fT3StartTime;  // time when T3 recv'd event

    friend TBuffer& operator<<(TBuffer&, FakedLongRec&);
    friend TBuffer& operator>>(TBuffer&, FakedLongRec&);

    ClassDef(T3NotifyStat,FdCdasVERSION)
};

/** Operator<<() to stream a long long into a TBuffer object.
  *
  * Note: Root in its latest stable implementation (3.02/07) still doesn't 
  *       support the streaming of the type long long !
  */
TBuffer& operator<<(TBuffer&, T3NotifyStat::FakedLongRec&);

/** Operator>>() to read a long long from a TBuffer object.
  */
TBuffer& operator>>(TBuffer&, T3NotifyStat::FakedLongRec&);

#endif  // _FdCdas_T3NotifyStat_hh_
