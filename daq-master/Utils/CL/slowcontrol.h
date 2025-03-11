#ifndef _SLOWLIB_H__
#define _SLOWLIB_H__

/******************************************

  $Author: castera $
  $Date: 2001/05/22 16:21:59 $
  $Revision: 1.3 $

  $Log: slowcontrol.h,v $
  Revision 1.3  2001/05/22 16:21:59  castera
  sync with CdF files;AC
   Modified Files:
   	CL.h central_local.h monitor.h run_config.h slowcontrol.h
   Added Files:
   	gpsstatus.h latlong.h
   ----------------------------------------------------------------------

  Revision 1.3  2001/03/29 08:07:09  guglielm
  Cosmetics

  Revision 1.2  2001/03/29 07:11:02  guglielm
  Updated the function prototypes list (lgg)


******************************************/

/******************************************
 * possible software errors on ADC use
 ******************************************/
typedef enum
{
  ADCError_first = 0,

  ADCError_none = ADCError_first,
  ADCError_state,
  ADCError_busy,
  ADCError_voie,

  ADCError_last = ADCError_voie,
  ADCError_max

} ADCErrors;

#define ADC_MUX 4
#define ADC_MUXVOIES 8
#define ADC_NBVOIE ADC_MUX * ADC_MUXVOIES
#define ADC_LASTVOIE ADC_NBVOIE - 1


/******************************************
 * possible ADC states
 ******************************************/
typedef enum
{
  ADCState_first = 0,

  ADCState_awake = ADCState_first,
  ADCState_nap,
  ADCState_sleep,

  ADCState_last = ADCState_sleep,
  ADCState_max

} ADCState;

/******************************************
 * adresses needed to change ADC states
 ******************************************/
#define ADCState_adresses \
	SCAD0, \
	SCAD1, \
	SCAD2


/******************************************
 * DAC
 ******************************************/
#define DAC_CHANNELS 4

/******************************************
 * possible software errors on DAC use
 ******************************************/
typedef enum
{
  DACError_first = 0,

  DACError_none = DACError_first,
  DACError_enum,
  DACError_channel,

  DACError_last = DACError_channel,
  DACError_max

} DACErrors;


/******************************************
 * DACs enum
 ******************************************/
typedef enum
{
  DACEnum_first = 0,

  DACEnum_0 = DACEnum_first,
  DACEnum_1,

  DACEnum_last = DACEnum_1,
  DACEnum_max

} DACEnum;

#define DAC_NBVOIE DAC_CHANNELS * DACEnum_max


/******************************************
 * prototypes
 ******************************************/

int is_ADC_done();
void wait_10micsec();
void wait_10millis() ;
ADCErrors get_ADC_error();
unsigned int set_ADC_state( ADCState state );
unsigned int nap_ADC();
unsigned int sleep_ADC();
unsigned int awake_ADC();
unsigned int read_nap_ADC( unsigned int voie );
unsigned int read_sleep_ADC( unsigned int voie );
unsigned int read_awake_ADC( unsigned int voie );
int is_ADC_sleeping();
unsigned int read_ADC( unsigned int voie, ADCState state );

ADCErrors ReadADC( unsigned short *pchan, unsigned int InUse /* , int IsTPC */ );

DACErrors get_DAC_error();
DACErrors mV_write_DAC( DACEnum dac, int channel, unsigned int val );
DACErrors raw_write_DAC( DACEnum dac, int channel, unsigned int val );
unsigned int mV_readback_DAC( DACEnum dac, int channel );
unsigned int raw_readback_DAC( DACEnum dac, int channel );

DACErrors ReadDAC ( unsigned short *pchan, unsigned char InUse /* , int IsTPC */ );
DACErrors WriteDAC( unsigned short *pchan, unsigned char InUse /*, int IsTPC */ );

unsigned int read_digital_input();
unsigned int readback_digital_output();
void write_digital_output( unsigned int val );

unsigned short WriteRegister( unsigned short preg );
unsigned short ReadRegister ();
unsigned short ReadBackRegister() ;
unsigned short OrWriteRegister( unsigned short value ) ;
unsigned short AndWriteRegister( unsigned short value ) ;
unsigned short MaskWriteRegister(unsigned short value, unsigned short mask) ;

#endif
