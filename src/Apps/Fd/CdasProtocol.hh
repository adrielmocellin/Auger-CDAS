
//
// File   : CdasProtocol.hh
//
// Purpose: Declaration of protocol structures to CDAS
//
// $Id$
//


/** @file CdasProtocol.hh
  * Protocol definition for the interface to CDAS.
  * @author H.-J. Mathes, FzK
  */
  

/** @mainpage AUGER FD-DAQ T3 and CDAS Interface
  *
  * @section intro Introduction
  *
    This package contains the definition of the data current protocol between FD
    and CDAS. It consists mainly of the transfer class T3Notify as well of a
    socket based connector class CdasConnector which handles a few extra things.
  *
  * @section usage Usage
  *
  * @bug
  *
  * @todo
  *  @li try to merge CdasConnector with class FdRoot::TSocketConnector
  *  @li identify T3 emitters via (new) T3Identity object ?
  *
  * @section changes CHANGES
  * 
  * The list of changes is maintained in the file CHANGES
  * @verbinclude CHANGES
  */

#ifndef _FdCdas_CdasProtocol_hh_
#define _FdCdas_CdasProtocol_hh_

#include <Rtypes.h>

//#include <FdCdasVersion.hh>


// namespace FdCdas {

/** First protocol version to communicate with the CDAS T3 consumer.
  *
  * This protocol was used during the EA phase.
  */
#define FdCdasVERSIONv1       1

/** Second protocol version to communicate with the CDAS T3 consumer.
  *
  * This extended protocol can be also used for the EVB-LIDAR 
  * communication.
  */
#define FdCdasVERSIONv2       2

/** Third protocol version to communicate with the CDAS T3 consumer.
  *
  * This protocol has been extended to pass more info to the EVB-LIDAR 
  * consumer.
  */
#define FdCdasVERSIONv3       3

/** Fourth protocol version to communicate with the CDAS T3 consumer.
  *
  * This protocol has been extended by the 'total T3 signal'.
  */
#define FdCdasVERSIONv4       4

/** Actual protocol version to communicate with the CDAS T3 consumer. */
#define FdCdasVERSION         FdCdasVERSIONv4

#include <T3Notify.hh>

// } // namespace FdCdas

#endif // _FdCdas_CdasProtocol_hh_
