/*****************************************************************************
 *
 *     This file contains all of the structures used to represent the SD data.
 *
 *****************************************************************************
 */
#ifndef __MoSDData__
#define __MoSDData__

//#include "MoStore.h"
#include "monitor.h"

typedef enum sddatumtype {
  SECOND = 0,
  ITEMNB,
  SUMMARY,
  ADC1, ADC2, ADC3, ADC4, ADC5, ADC6, ADC7, ADC8,
  ADC9, ADC10, ADC11, ADC12, ADC13, ADC14, ADC15, ADC16,
  ADC17, ADC18, ADC19, ADC20, ADC21, ADC22, ADC23, ADC24,
  ADC25, ADC26, ADC27, ADC28, ADC29, ADC30, ADC31, ADC32, 
  INPUTREGISTER,
  DAC1, DAC2, DAC3, DAC4, DAC5, DAC6, DAC7, DAC8,
  OUTPUTREGISTER,
  ALLT
} SDDATANAME;


#define NUMSDVALUES 32
#define NUMSDS 0x1fff  /* NOTE: this is defined in BsServices.h, how can we map
			  this to 1600 ??? */
#define VALID 1

typedef struct sdmon {
  short int inRun;
  short int valid;
  MONITOR monitor;
  double adcPhysValues[NADC];
} SDMON;

#endif
