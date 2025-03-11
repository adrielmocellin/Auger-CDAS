#ifndef _SPMT_PROT_H_
#define _SPMT_PROT_H_

#include <stdint.h>
/* structures used to communicate between station and server related with
   the small PMT.

   At this point, the idea is to transfer a complete trace plus a histogram.

   The histogram may not be transfered, in the case the histogram have
   already been transfered a little before (maybe exactly the same histogram).
*/

#define SPMT_MSG_VERSION 1

#define SPMT_MAX_PACKET_SIZE 10240

struct spmt_msg_h
{
  uint16_t version;
  uint16_t nevt;    /*just to track if it have lost event, it would
		      return a simple counter*/
  uint16_t size_comp;    /*real buffer size */

  uint16_t size_uncomp;  /*size of the buffer, uncompressed */
};

#define SPMT_PROT_BUFF_SIZE_MAX 8192
struct buff_str
{
  int used;
  char dat[SPMT_PROT_BUFF_SIZE_MAX];
};


struct spmt_prot_evt_str
{
  uint32_t sec,ticks,nevt;
  uint32_t AREA_PEAK[6];//0,1,2: PMT1, PMT2, PMT3; 3: sPMT; 4,5: SSD LG,HG;
};

struct spmt_prot_muon
{
  uint32_t StartSec;
  uint16_t charge[4];

};

struct spmt_prot_add_info
{
  int32_t t_start;
  int16_t th[3];//threshold of the three PMTs.
  int16_t mask;
  int16_t r[3];
  int16_t vem[3];
};


enum buff{
  espmt_prot_EVT=1,
  espmt_prot_MUON_INFO,
  espmt_prot_DAQ_INFO,
};
#endif
