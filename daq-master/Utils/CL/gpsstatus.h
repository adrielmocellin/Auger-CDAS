#if !defined(_GPSTATUS_H_)

#define _GPSTATUS_H_

#include "latlong.h"

typedef enum {
  GPS_STATUS_OK,
  GPS_STATUS_SO_SO,
  GPS_STATUS_BAD
} GPS_STATUSES ;

/* For the GPS sawtooth correction */
#define MAX_SAWTOOTH 32
typedef struct {
  unsigned int temps ;
  char sawtooth ;
} SAWTOOTH ;

typedef struct {
  int in, out, max ; /* Index sur la table circulaire */
  SAWTOOTH table[MAX_SAWTOOTH] ;
} SAWTOOTH_TABLE ;


/* GPS part in run_config.h */
typedef struct {            /* Position for hold_position    */ 
  LAT_LONG Lat ,              /* latitude in deg.min.sec */
    Long ;                  /* Longitude ... */
  float Height ;
  unsigned char Htype    ;  /* GPS ellips. reference         */
} POSIT_H;

#endif
