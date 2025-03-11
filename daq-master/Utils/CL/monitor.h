/*
   $Author: castera $
   $Date: 2001/06/28 15:09:11 $
   $Log: monitor.h,v $
   Revision 1.9  2001/06/28 15:09:11  castera
   Sync with CdF (definition of Channels for monit.)
    Modified Files:
    	monitor.h
    ----------------------------------------------------------------------

   Revision 1.8  2001/06/01 12:23:18  guglielm
   Changed DAC channels for PMT High voltages (according to Patrick mail)
   PMT1 : channel 7 (was 5)
   PMT2 : channel 6
   PMT3 : channek 5 (was 7)

   Revision 1.7  2001/05/28 10:54:01  guglielm
   Bug correction (too many #endif)

   Revision 1.6  2001/05/23 10:56:17  jmb

    Modified Files:
    	central_local.h monitor.h run_config.h

   ----------------------------------------------------------------------

   Revision 1.5  2001/05/22 13:22:44  castera
   *** empty log message ***

   Revision 1.4  2001/05/14 10:00:25  guglielm
   Add TAG_EVT_SVR in TAGTYPE (to add monitoring data in T3 event)

   Revision 1.3  2001/04/18 13:51:28  guglielm
   Added comments regarding DAC channel usage.

   Revision 1.2  2001/04/12 09:41:26  guglielm
   Changed comment regarding PMT High Voltages DAC.
   Defined DAC channels for PMT HV.

   Revision 1.1.1.1  2001/03/28 15:03:07  revenu
   no message

   Revision 1.4  2001/03/12 09:40:24  os9

    	monitor.h

   Revision 1.3  2001/03/01 13:15:05  os9
   no message

*/
/* monitor.h : Local Station Software - Pierre Auger Project
   ----------
              JM Brunet, L Gugliemi, G Tristram
                  College de France - Paris

   The structure defined in this include file are contained in the date
   module 'Monitor'

   Created    10/09/99 - JMB
   Modified : 24/05/00 - Version 2.4

-------------------------------------------------------------------- */

#ifndef _MONITOR_
#define _MONITOR_

#include "run_config.h"


   /* 32 ADCs : bits 0-11 : ADC value
                bit    13 : Warning Red
                bit    14 : Warning Orange
                bit    15 : ADC not in use
   Meanning :
     Chan[ 0]:Tpmt1        Chan[ 1]:Ipmt2          Chan[ 2]:Ipmt3
     Chan[ 3]:Tpmt2        Chan[ 4]:Ipmt1          Chan[ 5]:Vpmt3
     Chan[ 6]:Vpmt1        Chan[ 7]:Vpmt2          Chan[ 8]:GND
     Chan[ 9]:Tfree        Chan[10]:Telect         Chan[11]:3.3 V
     Chan[12]:Twater       Chan[13]:Water level    Chan[14]:Tpmt3
     Chan[15]:Tair in      Chan[16]:V1sol panel    Chan[17]:I2battery
     Chan[18]:T2battery    Chan[19]:V2sol panel    Chan[20]:I1battery
     Chan[21]:V2battery    Chan[22]:T1battery      Chan[23]:V1battery
     Chan[24]:5 V          Chan[25]:free           Chan[26]:Patm out
     Chan[27]:12 V ou pot  Chan[28]:Vcloud offset  Chan[29]:Vcloud therm
     Chan[30]:Vcloud comp  Chan[31]:Vcloud uncomp
   */

typedef struct {   /* ADCs */
  unsigned short Chan[NADC];
  unsigned short InputRegister;
} MONITOR_READ;

#define ADCMASK 0xFFF

#define SUMWARNINGRED    1
#define SUMWARNINGORANGE 1<<1

#define ADCWARNINGRED      1<<13
#define ADCWARNINGORANGE   1<<14
#define ADCWARNINGNOTINUSE 1<<15


   /* 8 DACs 12 bits : stored in 16 bits.
      **************
      Chan[0] : Vpmt3     Chan[1] : Vpmt2       Chan[2] : Vpmt1
      Chan[3] : LED pulse 
      Chan[4] : to Chan[7] : Thresholds for the fast track FrontEnd (E.A.)

      LGG - 11/4/2001
      ---------------
      Chan[0] : single majority threshold
      Chan[1] : 
      Chan[2] : TOT threshold
      Chan[3] : TOT Time
      Chan[4] : 
      Chan[5] : Pmt3 HV
      Chan[6] : Pmt2 HV
      Chan[7] : Pmt1 HV
   */

#define HV1_CHAN 7
#define HV2_CHAN 6
#define HV3_CHAN 5
#define SINGLE_THRESH_CHAN 0
#define TOT_THRESH_CHAN 2
#define TOT_TIME_CHAN 3

typedef struct {   /* DACs */
  unsigned short Chan[NDAC];
  unsigned short OutputRegister;
} MONITOR_WRITE;

   /* 32 bits Summary :
                 - bit  0 : = 1 : at least 1 ADC error
                 - bit  1 :       at least 1 ADC warning
                 - bit 16 :       GPS State in Alarm
                 - bit 17 :       GPS State unknown
                 - bit 18 :       No 1 pps from GPS
                 - bit 31 :       Run not enable in this station
   */


typedef struct {         /* Content of the monitoring buffer */
  unsigned int   Second; /* Starting time */
  unsigned int   ItemNb; /* mean done on this number of items */
  unsigned int   Summary;
  MONITOR_READ   Read;
  MONITOR_WRITE  Write;
} MONITOR;

#if defined (_OS9000)

#define MAX_MONIT_BUFFER  50
#define MONIT_BUFFER_NAME "MonitBuffer"
#define NB_MONIT_RELEASE  5
#define MONIT_SIZE  MAX_MONIT_BUFFER*sizeof(MONITOR)

typedef enum { TAG_REQ_ACK= 1, TAG_SEND, TAG_EVT_SVR, TAG_DATA_SVR=5, TAG_SPY 
} TAGTYPE;


#if defined(_MONITOR_MAIN_)

EvbId MonitId;

#else

extern EvbId MonitId;

#endif
#endif

typedef struct 
{
	unsigned int requestId;
	unsigned int bufferOk;
	MONITOR monitor;
} monitorBlock, *monitorBlockP; 

#include "slowcontrol.h"

#endif
