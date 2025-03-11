#ifndef __PmLSX__
#define __PmLSX__

/*
 * Command list understood by LSx
 */
#include "commandlist.h"

/*
 * Macros definitions for LSx API
 */
#include "packet.h"

#define DEBUG_PACKET_HANDLER			0
#define DEBUG_SOCKET_PACKET_OUTPUT		0
#define DEBUG_PACKET_INTERPRETER		0

struct PacketStructure 
{
  char RxPreamble[MAX_RX_PREAMBLE_LENGTH];
  int RxPreambleCounter;
  int MsgState;
  int Length;
  unsigned char Type;
  unsigned char BSUID;
  unsigned char Data[MAX_DATA_LENGTH];
  int DataBufferCounter;
  unsigned char CRCData[MAX_DATA_LENGTH];
  int CRCDataCounter;
  unsigned char CRC[CRC_LENGTH];
  int CRCCounter;
  int SocketNumber;
  unsigned char PacketBackup[MAX_DATA_LENGTH];
  int PacketBackupCounter;
};

#endif
