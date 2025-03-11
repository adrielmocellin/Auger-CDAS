/*
   $Author: castera $
   $Date: 2001/05/22 16:21:59 $
   $Log: run_config.h,v $
   Revision 1.4  2001/05/22 16:21:59  castera
   sync with CdF files;AC
    Modified Files:
    	CL.h central_local.h monitor.h run_config.h slowcontrol.h
    Added Files:
    	gpsstatus.h latlong.h
    ----------------------------------------------------------------------

   Revision 1.8  2001/04/12 09:45:47  guglielm
   Added include files gpsstatus.h and latlong.h
   Deleted structure Posit_H (now in gpsstatus.h)


*/
/* run_config.h : Local Station Software - Pierre Auger Project
   ------------
              JM Brunet, L Gugliemi, G Tristram
                  College de France - Paris

   The structures defined in this include file are contained in the data
   module 'Config'


   Created    03/03/99 - JMB : Version 2.3
   Modified   24/05/00 - JMB : Version 2.4

-------------------------------------------------------------------- */

#ifndef _RUN_CONFIG_
#define _RUN_CONFIG_


#define NADC  32
#define NPMT   3
#define NDAC   8

#include "latlong.h"
#include "gpsstatus.h"

#include "monitor.h"

typedef enum {              /* For M_CONFIG_SET message */
  CONF_UPDATE_ALL= 0,       CONF_UPDATE_FE_PARAMS,
  CONF_UPDATE_ACQ_PARAMS,   CONF_UPDATE_GPS_PARAMS,
  CONF_UPDATE_MONIT_PARAMS, CONF_UPDATE_CALIB_PARAMS
} UPDATE_PARAMS;

/*---- Front End Params : */
/*     ****************** */

#ifdef EA_BUFFER          /* For the Engineering Array FrontEnd board */
                          /* (from Patrick Allison)                   */

/* Note: all thresholds are in PLD/ASIC format: 0-1023.   */
/* We translate this into the V1 analog threshold inside  */
/* the trigger setting function. */

typedef struct ThresholdStruct
{
  unsigned short pmt1;   /* Threshold level for PMT 1 */
  unsigned short pmt2;   /* Threshold level for PMT 2 */
  unsigned short pmt3;   /* Threshold level for PMT 3 */
  unsigned short pmtsum; /* Threshold level for the sum of all PMT channels,
                            for boards that support it. */
} THRESHOLD;

typedef struct V1_TrigStruct      /* V1 setting */
{
  unsigned char high_mask;        /* Bit pattern of allowed tubes:
                                     PMT1 is 0x1, PMT2 is 0x2, PMT3 is 0x4 */
  unsigned char tot_mask;         /* Bit pattern of allowed tubes in the
     	                             time-over-threshold (FAST1) trigger */
  unsigned char high_coincidence; /* Number of tubes needed for a FAST0
                                     trigger */
  unsigned char tot_coincidence;  /* Number of tubes needed for a FAST1
                                     trigger */
} V1TRIG_SETTINGS;

typedef struct V2_TrigStruct      /* V2 setting */
{
  unsigned char high_mask;   /* Bit pattern of allowed tubes */
  unsigned char tot_mask;    /* Bit pattern of allowed tubes for FAST1 */
  unsigned char control;     /* Control integer for the FE v2 ASIC/PLD */
                             /* Not really sure what this does yet... the
                                documentation is weak on explaining this */
  unsigned char pretrigger;  /* Number of 'pretrigger' bins */
} V2TRIG_SETTINGS;

typedef struct FE_ParamsStruct
{
  V1TRIG_SETTINGS V1Settings; /* Settings for the V1 board */
  V2TRIG_SETTINGS V2Settings; /* Settings for the V2 board */
  THRESHOLD Fast0;            /* Thresholds for the FAST0 trigger */
  THRESHOLD Fast1;            /* Thresholds for the FAST1 trigger */
  unsigned int tot_thresh;    /* Threshold for the time portion of the FAST1
                                 trigger */
} FE_PARAMS;

#else                     /* For the final FrontEnd board */

typedef struct { 
  unsigned int Width, Occupancy, Thresh, Delay;
} FAST_WINDOW;

typedef struct {
  unsigned int Width, ZeroSupThresh, MuZeroSup, MuGateWidth;
} SLOW_WINDOW;

typedef struct {            /* FrontEnd electronics parameters */
  unsigned char IsFast0Enabled, IsFast1Enabled, 
                IsSlowEnabled,  IsRandomEnabled;
  FAST_WINDOW fast0;
  FAST_WINDOW fast1;
  SLOW_WINDOW slow;
  unsigned int FastZeroSupThresh, FastRandomTriggerDelay,
    FastExternTriggerDelay;
  unsigned int Scaler1LowThresh, Scaler2LowThresh, Scaler3LowThresh;
} FE_PARAMS;

#endif

typedef struct {            /* Acquisition parameters */
  unsigned short HardAddress;
  unsigned short VEMmin;
  unsigned char Trigger2Algorithm, RunEnable;
} ACQ_PARAMS;


typedef struct {            /* GPS parameters */
  unsigned char AlarmLim[2] ; /* Alarm limit in 100s of nsec */
  unsigned char CabDel[4]   ; /* Antenna cable delay nsec    */ 
  unsigned char Mask        ; /* Mask angle degrees  0.. 89  */
  POSIT_H Posit_h           ; /* Position for hold_position  */ 
} GPS_PARAMS;

typedef struct {            /* Monitor parameters */
  unsigned short ReadRate;         /* Read ADCs frequency in second */
  unsigned char Times;             /* number of reads before a buffer write */
  unsigned char IsSendRegular;
  unsigned char IsTPC;             /* If Tank Power Control */
  unsigned char DACinUSE;          /* On / Off */
  unsigned short Dac[NDAC];        /* 12 bits DACs */
  unsigned short OutputRegister;   /* 16 bits register */
  unsigned short MinOrange[NADC], MaxOrange[NADC];
  unsigned short MinRed[NADC],    MaxRed[NADC];
  unsigned int ADCinUSE;           /* On / Off */
} MONIT_PARAMS;


typedef struct {            /* Monitor parameters for PMTs */
  unsigned short Dac[NPMT];        /* 12 bits DACs */
  unsigned short MinOrange[NPMT], MaxOrange[NPMT];
  unsigned short MinRed[NPMT],    MaxRed[NPMT];
} PMT_PARAMS;


typedef struct {
  FE_PARAMS  FeParams;
  ACQ_PARAMS AcqParams;
  GPS_PARAMS GpsParams;
  MONIT_PARAMS MonitParams;
} CONFIG;  


#if defined (_OS9000)
#ifdef _CONFIG_MAIN_

char *ConfigModName= "Config";

CONFIG   *CfData;
mh_com   *CfHeader;

#else 

extern char *ConfigModName;
extern CONFIG   *CfData;
extern mh_com   *CfHeader;

#endif
#endif
#endif





