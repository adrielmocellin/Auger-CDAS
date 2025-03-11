
#include "crc.h"
#include "PmClientConnection.h"
#include "PmServer.h"
#include "PmLog.h"

/*Fernando Contreras*/
/*FILE * FC_file;
int FC_counter;
unsigned char FC_etx;*/
/*End Fernando Contreras*/


/*-----------------------------------------------------------------------*/
/* class PmConnection.                                                   */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmConnection::PmConnection ()
/*-----------------------------------------------------------------------*/
{
  // Services Initialization
  for (int i = 0; i < __MAXSERVICESBYCONNECTION__; i++)
    _pmService[i] = NULL;
  _whichService = 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmConnection::DiscardMess (int &sock, int &len)
/*-----------------------------------------------------------------------*/
{
  int n = 0;
  longWord toto;
  while (n != len)
    n += read (sock, (void *) &toto, sizeof (longWord));
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmStdHeader *
PmConnection::_ReadPmStdHeaderFromSocket (int &sock)
/*-----------------------------------------------------------------------*/
{
  int nb = 0, len, dataLen;
  PmStdHeader *pmStdHeaderP;
  longWord temp;

  dataLen = sizeof (PmStdHeader);
  len = sizeof (longWord);

  assert ( pmStdHeaderP = new PmStdHeader () ) ; // RR +

  nb = read (sock, (void *) &temp, len);
  pmStdHeaderP->length = ntohl (temp);
  nb += read (sock, (void *) &temp, len);
  pmStdHeaderP->protocolVersion = ntohl (temp);
  nb += read (sock, (void *) &temp, len);
  pmStdHeaderP->type = ntohl (temp);
  nb += read (sock, (void *) &temp, len);
  pmStdHeaderP->hLength = ntohl (temp);

  if (nb != dataLen)
    {
      delete pmStdHeaderP;
      return NULL;
    }

  return pmStdHeaderP;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors
PmConnection::_CheckTrailerFromSocket (int &sock)
/*-----------------------------------------------------------------------*/
{
  longWord temp;
  read (sock, (void *) &temp, sizeof (longWord));
  return (ntohl (temp) != PmMESSAGESENDMARKER) ? CDAS_TRAILER_BAD : CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmConnection::InitialisePacketHandler (void)
/* DESCRIPTION:
 *   This function initialises the ForwardPM global array of
 *   packet structures. This function must be called before
 *   the packet handler can be used.
 *
 * PARAMS:
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  // initialise BSU packet handler variables
  BSUPacket.MsgState = MSG_STATE_IDLE;	// reset the message state
  strncpy (BSUPacket.RxPreamble, RX_PREAMBLE, RX_PREAMBLE_LENGTH);	// copy the preamble
  BSUPacket.RxPreambleCounter = 0;
  BSUPacket.DataBufferCounter = 0;
  BSUPacket.CRCDataCounter = 0;
  BSUPacket.CRCCounter = 0;
  BSUPacket.SocketNumber = 0;
  BSUPacket.PacketBackupCounter = 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
unsigned int
CRC_32bit (gs_byte * message, word length_in_bytes)
/* DESCRIPTION:
 *   This function initialises the ForwardPM global array of
 *   packet structures. This function must be called before
 *   the packet handler can be used.
 *
 * PARAMS:
 *   unsigned char *message = a pointer to the start memory
 *                            address of the message.
 *   unsigned int length_in_bytes = the length of the message in bytes.
 *
 * RETURN VALUE:
 *   unsigned long = the CRC code word that resulted from
 *                   the processing of the message.
 */
/*-----------------------------------------------------------------------*/
{
  unsigned int LUT_Byte, CRC_reg = 0xffffffffUL;	/* Initialise the CRC  */
  unsigned int count;		/* register to zero    */

  for (count = 0; count < length_in_bytes; count++)
    {/* For every byte in   *//* the message do      */
      LUT_Byte = (CRC_reg & 0xFF) ^ (unsigned long) *message++;	/* Bit wise EXOR */
      /* the lower byte of CRC */
      /* register with a message byte */
      CRC_reg = CRC_reg >> 8;	/* Right shift the CRC */
      /* register by 8 bits  */
      CRC_reg = (unsigned int) (CRC_reg ^ LUT_CRC_32[LUT_Byte]);	/* Bit wise EXOR CRC register */
    }/* with the value from the LUT */
  CRC_reg = CRC_reg ^ 0xffffffffUL;
  return (CRC_reg);
}

/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
int
SendBSUPacket (const int length,
			     const char type,
			     const gs_byte * data,
			     const int BSUStationID,
			     const int SocketDescriptor)
/* DESCRIPTION:
 *   Packet transmitter.  Function sends a packet to the BSU using
 *   the socket interface.
 *
 * PARAMS:
 *   length = length of data part of the message
 *   type = Message type
 *   data = pointer to data
 *   BSUStationID
 *   SocketDescriptor
 *
 * RETURN VALUE:
 *   number of bytes written to the FIFO or -1 if the write 
 *   operation fails. (see errno for details).
 */
/*-----------------------------------------------------------------------*/
{
  unsigned char MessageBuffer[MAX_DATA_LENGTH];
  char crc_fix;
  int crc_return;
  int MessageByteCounter = TX_PREAMBLE_LENGTH;	// set message counter to length of preamble
  int i;
  int BytesWritten;


#if DEBUG_SOCKET_PACKET_OUTPUT
  static int PacketCounter = 0;
  char DebugMessage[200];
  PmLogPrint(IKDEBUG,"Sending packet - %d", PacketCounter++);
#endif

  strncpy ((char *) MessageBuffer, TX_PREAMBLE, TX_PREAMBLE_LENGTH);	// copy preamble into buffer
  MessageBuffer[MessageByteCounter++] = (unsigned char) length + DATA_LENGTH_MODIFIER + 1;	// place length into crc buffer (+1 for BSUID)
  MessageBuffer[MessageByteCounter++] = type;	// place type into crc buffer
  MessageBuffer[MessageByteCounter++] = BSUStationID;	// add BSU ID

  if (data != NULL && length != 0)	// if data to send place in buffer
    {
      for (i = 0; i < length; i++) MessageBuffer[MessageByteCounter++] = data[i];
    }

  crc_return = CRC_32bit (&MessageBuffer[TX_PREAMBLE_LENGTH], (length + 3));	// calculate CRC for data, length, type and ID chars

  for (i = 0; i < CRC_LENGTH; i++)
    {
      crc_fix = (unsigned char) ((crc_return >> ((8 * (CRC_LENGTH - 1)) - (i * 8))) & 0x000000ff);
      MessageBuffer[MessageByteCounter++] = crc_fix;	// add CRC
    }

  MessageBuffer[MessageByteCounter++] = (unsigned char) ETX;


//  SEND MessageBuffer here with a count of MessageByteCounter
#if DEBUG_SOCKET_PACKET_OUTPUT
  PmLogPrint(IKDEBUG,"Sending message : ");
  strncpy ((char *) DebugMessage, (char *) MessageBuffer, TX_PREAMBLE_LENGTH);
  DebugMessage[TX_PREAMBLE_LENGTH] = '\0';
  PmLogPrint(IKDEBUG,"%s,", DebugMessage);
  PmLogPrint(IKDEBUG,"%d,", MessageBuffer[TX_PREAMBLE_LENGTH]);
  PmLogPrint(IKDEBUG,"%c,", MessageBuffer[TX_PREAMBLE_LENGTH + 1]);
  for (i = 0; i < length + 5; i++)
  {
		if ( i == 0 ) PmLogPrint(IKDEBUG,"bsuid=0x%02x ", (int) MessageBuffer[TX_PREAMBLE_LENGTH + 2 + i]);
		else if ( i == 8 ) PmLogPrint(IKDEBUG,"mode=0x%02x ", (int) MessageBuffer[TX_PREAMBLE_LENGTH + 2 + i]);
		else if ( i == 3 ) PmLogPrint(IKDEBUG,"pcknum=0x%02x ", (int) MessageBuffer[TX_PREAMBLE_LENGTH + 2 + i]);
    else PmLogPrint(IKDEBUG,"0x%02x ", (int) MessageBuffer[TX_PREAMBLE_LENGTH + 2 + i]);
	}
#endif

  BytesWritten = write (SocketDescriptor, MessageBuffer, MessageByteCounter);
  if (BytesWritten == -1 && errno == EPIPE)
    {
      // close socket here
      PmLogPrint(IKERROR,"Failed to write bytes to socket, server closed connection");
    }

#if DEBUG_TIMESLOT_PACKETS
  if ((type == CONTROL) && (data[0] == 'i'))
    PmLogPrint(IKDEBUG,"sending timeslot info packet %d on scoket desc. : %d (Written : %d)",
       TimeslotPacketCounter++, SocketDescriptor, BytesWritten);
#endif
  // send the message and return
  return (BytesWritten);
}



/*-----------------------------------------------------------------------*/
/* class PmBSUConnection.                                                */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmBSUConnection::PmBSUConnection (PmStdConnection & conn, int sock)
/*-----------------------------------------------------------------------*/
{
  // Initialization of properties
  _name = conn.pmStdHeader.type;
  _id = conn.id;
  _socket = sock;
  _version = conn.pmStdHeader.protocolVersion;
  _idService = conn.pmStdHeader.type;

  /* Initialisation of implicite services. */
  _lsBlocksServiceP = new PmLSBLOCKSServices ();
  _broadcastServiceP = new PmBROADCASTServices ();

  /* To manage the timing output. */
  _current_time = _last_time = 0;

  /* Count the number of messages received from radio. */
  _count_radio = 0;

  /* Initialize variables */
  _uploadRunning = PmNO;

  // VT - Call packet handler initialisation function
  InitialisePacketHandler ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBSUConnection::RemoveClient ()
/*-----------------------------------------------------------------------*/
{
  delete _lsBlocksServiceP;
  delete _broadcastServiceP;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBSUConnection::TrashBadLsBlock (char *data)
/*-----------------------------------------------------------------------*/
{
  FILE *f;
  f = fopen (PMBADDATA, "a");
  fwrite (data, ntohl (*((longWord *) data)), 1, f);
  fwrite ("iiiiiiiiiiiiiiii", 16, 1, f);
  fclose (f);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors 
PmBSUConnection::GetInput()
/* DESCRIPTION:
 *   Using the LSx routine for handling data from BSU
 *   Getting raw data from BSU's object socket
 *
 * PARAMS:
 * RETURN VALUE:
 *   cdasErrors = CDAS_CONNECT_ERROR
 *                CDAS_TRAILER_BAD
 *                CDAS_INPUTS_ERROR
 *                CDAS_SUCCESS
 */
/*-----------------------------------------------------------------------*/
{
  unsigned char PacketBuffer[200];
  int BytesRead;

  if ((BytesRead = read (_socket, PacketBuffer, sizeof (PacketBuffer))) <= 0)
    return CDAS_CONNECT_ERROR;

  // Processing the BSUPacket structure
  return HandleBSUPacket (BytesRead, PacketBuffer);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors 
PmBSUConnection::GetInputTest() // od
/* DESCRIPTION:
 *   Using the LSx routine for handling data from BSU
 *   Getting raw data from BSU's object socket
 *
 * PARAMS:
 * RETURN VALUE:
 *   cdasErrors = CDAS_CONNECT_ERROR
 *                CDAS_TRAILER_BAD
 *                CDAS_INPUTS_ERROR
 *                CDAS_SUCCESS
 */
/*-----------------------------------------------------------------------*/
{
  unsigned char PacketBuffer[200];
  int BytesRead=0,n,nBytesInMess;
  unsigned char *tmpP=PacketBuffer;

  int size = RX_PREAMBLE_LENGTH+1;
  while (size>0)
    {
      n = read (_socket, tmpP, size);
      BytesRead += n;
      switch(n) 
	{
	case -1 :
	  IkSevereSend ("Read() error : %s",strerror(errno));
	  PmLogPrint (IKERROR,"Read() error : %s\n",strerror(errno));
	  return CDAS_CONNECT_CLOSED;
	case 0 :
	  return CDAS_CONNECT_CLOSED;
	default :
	  tmpP += n;
	  size -= n;
	  break;
	} 
    }

  nBytesInMess = (int)PacketBuffer[RX_PREAMBLE_LENGTH];
  static int ii=0;
  if(ii++<30) PmLogPrint(IKINFO,"nBytesInMess %d",nBytesInMess);
  if(nBytesInMess>MAX_MSG_LENGTH || nBytesInMess<MIN_MSG_LENGTH)
    {
      IkWarningSend("Incorrect packet received from %d, closing connection",_socket);
      PmLogPrint(IKERROR,"Incorrect packet received from %d, closing connection",_socket);
      return CDAS_CONNECT_CLOSED;
    }

  size = nBytesInMess; 
  while (size>0)
    {
      n = read (_socket, tmpP, size);
      BytesRead += n;
      switch(n) 
	{
	case -1 :
	  IkSevereSend ("Read() error : %s",strerror(errno));
	  PmLogPrint (IKERROR,"Read() error : %s\n",strerror(errno));
	  return CDAS_CONNECT_CLOSED;
	case 0 :
	  return CDAS_CONNECT_CLOSED;
	default :
	  tmpP += n;
	  size -= n;
	  if (size>0) {
	    PmLogPrint(IKINFO,"PmBSUConnection::GetInputTest: size != 0 !!! : %d %d %d %d",size,n,nBytesInMess,BytesRead);
	  }
	  break;
	} 
    }

  // Processing the BSUPacket structure
  return HandleBSUPacket (BytesRead, PacketBuffer);
}

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
void
PmBSUConnection::GetInput (IkMessage * ikm, char *mode, int type)
/* DESCRIPTION:
 *   Fetch IkMessage and process message in a certain format
 *   before sending it
 *
 * PARAMS:
 *   IkMessage* = pointer the object IkMessage
 *   mode = mode of transfer
 *   type = type of message
 *          if IKLSLOGREQ, it's an upload request
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  if (type == IKLSLOGREQ) // UPLOADing request by a BSU
    theUPLOADServices ()-> WakeUp (ikm, (int) _id, _socket);
  else
    _broadcastServiceP->GetInput (ikm, mode, type);
}

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
int 
PmBSUConnection::_ProcessLsFrame (LsFrameP Data)
/* DESCRIPTION:
 *   Called by BSUPacketInterpreter on DATA type
 *   Link the input LS with it's assigned BSU in the routing table
 *
 * PARAMS:
 *   IkMessage* = pointer the object IkMessage
 *   mode = mode of transfer
 *   type = type of message
 *          if IKLSLOGREQ, it's an upload request
 *
 * RETURN VALUE:
 *   0 for errors and 1 for success
 */
/*-----------------------------------------------------------------------*/
{
  int lsId = Data->id;

  if (lsId > __MAXLSID__)
    {
      IkSevereSend ("Received data from LS %d, id greater than 1600: discarding", lsId);
      return 0; // RR +
    }

  // Write LsFrame on disk (see PmConst.h for the path)
  if (theStationDataBase ()->IsRequestedByTrashService (lsId))
    theLTRASHServices ()->GetInput (Data, lsId);

  // Link the local station with this BSu connection and
  // update the BSU routing table
  if (theStationDataBase ()->LinkLsWithBSu (lsId, (int) _id, _socket) == -1)
    {
      IkSevereSend ("failed to link Ls %d from BSU %d", lsId, _id);
      PmLogPrint (IKERROR,"GetInput : failed to link Ls %d from BSU %d", lsId, _id);
      return 0; // RR +
    }

  // Preform data from the current BSU
  _lsBlocksServiceP->GetLsFrame (*Data, (int) _id);
  return 1;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBSUConnection::Process ()
/* DESCRIPTION:
 *   Builds the message stored during input process and prepares to be sent 
 *   to a specific client
 *
 * PARAMS:
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  int forMe = (int) _id;

  // check state of upload
  if ((_uploadRunning = theUPLOADServices ()->State (forMe)) == PmYES)
    {
      PmUPLOADMessage *upload;
      // ready to send next part of message ?
      if ((upload = theUPLOADServices ()->CheckMessage (forMe)))
	{
	  _broadcastServiceP->GetInput (*upload);
	  delete upload;
	}
      _uploadRunning = theUPLOADServices ()->State (forMe);
    }
  // Read built outputs
  _broadcastServiceP->BuildOutput ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors 
PmBSUConnection::SendOutput ()
/* DESCRIPTION:
 *   Called by 'OutputToClients' process
 *   Sends data stored during the Listening process
 *
 * PARAMS:
 * RETURN VALUE:
 *   cdasErrors = CDAS_SUCCESS
 *                CDAS_CONNECT_CLOSED
 *                CDAS_MALLOC_ERROR
 */
/*-----------------------------------------------------------------------*/
{
  time (&_current_time);

  if (_current_time > (_last_time + 1))	// no more than one message per second
    {
      for (int mess = 0; mess < __MAXNUMBEROFMESSPERSEC__; mess++)
	{
	  if (_broadcastServiceP->listOfOutput_.size ())
	    {
	      cdasErrors error = _SendData ();
	      _broadcastServiceP->ResetOutput ();
	      return error;
	    }
	}
      _last_time = _current_time;
    }

  return CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors 
PmBSUConnection::_SendData ()
/* DESCRIPTION:
 *   Writes data to the socket of the uLSx
 *
 * PARAMS:
 * RETURN VALUE:
 *   cdasErrors = CDAS_SUCCESS
 *                CDAS_CONNECT_CLOSED
 *                CDAS_MALLOC_ERROR
 */
/*-----------------------------------------------------------------------*/
{
  static unsigned char data[__MAXBYTESTOWRITETOBSU__];
  longWord length_data;

  if ((length_data = _broadcastServiceP->listOfOutput_[0].length) > __MAXBYTESTOWRITETOBSU__ ) // RR +
    {
      IkSevereSend ("Impossible to write %d bytes to BSu %d, default malloc is %d",
	 length_data, _id, __MAXBYTESTOWRITETOBSU__);
      PmLogPrint (IKERROR,"_SendData : Impossible to write %d bytes to BSu %d, default malloc is %d",
	 length_data, _id, __MAXBYTESTOWRITETOBSU__);
      return CDAS_MALLOC_ERROR;
    }

#if PMDEBUG
PmLogPrint(IKDEBUG,"*  Entering for sending data to BSU client (%d) socket=%d...",(int)_id,_socket);
#endif
  // Send data to the BSU channel
  memcpy (data, _broadcastServiceP->listOfOutput_[0].output, length_data);
  int bytes_written = SendBSUPacket((int) length_data, DATA, (const gs_byte *)(data + 1), (int) _id, _socket);	// RR

  if (bytes_written <= 0)
    {
      IkSevereSend ("Impossible to write %d bytes to BSu %d, FIFO fails!", length_data, _id);
      PmLogPrint (IKERROR,"_SendData : Impossible to write %d bytes to BSu %d, FIFO fails!", length_data, _id);
      return CDAS_CONNECT_CLOSED;
    }
  struct timeval tv;
  struct timezone tz;
  gettimeofday (&tv, &tz);
  PmLogPrint(IKINFO,"BSUSending to %d: time before sleep %d.%d",_id,tv.tv_sec,tv.tv_usec);
  
  //  usleep(100000); // RR + FIX UP THIS DELAY PLEASE VANCE!
  return CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors
PmBSUConnection::HandleBSUPacket (const int Length, const unsigned char *Data)
/* DESCRIPTION:
 *   Main code ripped off from the packet handler.  Function
 *   forwards packets to the NMPC from the BSx after first checking
 *   that the packet is of the correct format.  This ensures that only
 *   good packets are forwarded and that a valid BSU ID can be
 *   be used to open a socket if the unit has not already done so.
 *   A complete message MUST be transferred to this function in
 *   one go.  Part messages are not valid.
 *
 * PARAMS:
 *   Length = length of message to process
 *   Data = pointer to data
 *
 * RETURN VALUE:
 *   cdasErrors = CDAS_SUCCESS
 *                CDAS_CONNECT_CLOSED
 *                CDAS_INPUTS_ERROR
 *                CDAS_TRAILER_BAD
 */
/*-----------------------------------------------------------------------*/
{
  int DataCounter = 0;
  unsigned int CRCTest;
  unsigned int CRC;
  char MessageString[100];
  cdasErrors ret = CDAS_SUCCESS;

#if DEBUG_PACKET_HANDLER
  sprintf (MessageString, "Packet Handler : Processing = %d characters\n", Length);
  PmLogPrint(IKDEBUG,"%s", MessageString);
  PmLogPrint(IKDEBUG,"Received : ");
  PmLogPrint(IKDEBUG,"%s", Data);
#endif

  if (Length == 0)		// check the function has data to process
    return CDAS_CONNECT_CLOSED;

  do
    {
      if (BSUPacket.MsgState == MSG_STATE_IDLE)	// if message state idle
	{			// length check for preamble
	  if (Data[DataCounter] == BSUPacket.RxPreamble[BSUPacket.RxPreambleCounter])
	    {
	      BSUPacket.MsgState = MSG_STATE_PREAMBLE;	// if match found change message state
	      BSUPacket.RxPreambleCounter++;
	      BSUPacket.PacketBackup[BSUPacket.PacketBackupCounter++] =
		Data[DataCounter];
#if DEBUG_PACKET_HANDLER
PmLogPrint(IKDEBUG,"Packet Handler : 1st match found");
#endif
	    }
	  DataCounter++;	// increment rx counter match or not
	}
      else if (BSUPacket.MsgState == MSG_STATE_PREAMBLE)
	{
#if DEBUG_PACKET_HANDLER
	  PmLogPrint(IKDEBUG,"Comparing %c with Rx:%c", Data[DataCounter], BSUPacket.RxPreamble[BSUPacket.RxPreambleCounter]);
#endif
	  if (Data[DataCounter] == BSUPacket.RxPreamble[BSUPacket.RxPreambleCounter])
	    {
	      BSUPacket.RxPreambleCounter++;
	      if (BSUPacket.RxPreambleCounter == RX_PREAMBLE_LENGTH)
		{
		  BSUPacket.RxPreambleCounter = 0;	// on good preamble reset counter
		  BSUPacket.MsgState = MSG_STATE_LENGTH;	// set message state
#if DEBUG_PACKET_HANDLER
		  PmLogPrint(IKDEBUG,"Packet Handler : preamble match found");
#endif
		}
	    }
	  else			// else if bad char
	    {
	      BSUPacket.MsgState = MSG_STATE_IDLE;	// reset message state
	      BSUPacket.RxPreambleCounter = 0;	// and counter
	      BSUPacket.PacketBackupCounter = 0;
	      PmLogPrint (IKERROR,"Packet Handler : preamble failed after first match found");
	      ret = CDAS_INPUTS_ERROR;
	    }
	  DataCounter++;
	}
      else if (BSUPacket.MsgState == MSG_STATE_LENGTH)
	{
	  BSUPacket.Length = (unsigned int) Data[DataCounter];
	  BSUPacket.PacketBackup[BSUPacket.PacketBackupCounter++] =
	    Data[DataCounter];
	  if (BSUPacket.Length <= MAX_MSG_LENGTH
	      && BSUPacket.Length >= MIN_MSG_LENGTH)
	    {
	      BSUPacket.CRCData[BSUPacket.CRCDataCounter] = (unsigned int) Data[DataCounter];	// add length into crc check
	      BSUPacket.CRCDataCounter++;
	      BSUPacket.MsgState = MSG_STATE_TYPE;
#if DEBUG_PACKET_HANDLER
	      sprintf (MessageString, "Message length = %d\n", BSUPacket.Length);
	      PmLogPrint(IKDEBUG,"%s", MessageString);
#endif
	    }
	  else			// else if bad length
	    {
	      BSUPacket.MsgState = MSG_STATE_IDLE;	// reset message state
	      BSUPacket.RxPreambleCounter = 0;	// and counter
	      BSUPacket.PacketBackupCounter = 0;
	      sprintf (MessageString, "BAD message length = %d", BSUPacket.Length);
	      PmLogPrint(IKERROR,"%s", MessageString);
	      ret = CDAS_INPUTS_ERROR;
	    }
	  DataCounter++;
	}
      else if (BSUPacket.MsgState == MSG_STATE_TYPE)
	{
	  BSUPacket.Type = Data[DataCounter];
	  BSUPacket.CRCData[BSUPacket.CRCDataCounter] = Data[DataCounter];	// add type into crc check
	  BSUPacket.PacketBackup[BSUPacket.PacketBackupCounter++] = Data[DataCounter];
	  BSUPacket.CRCDataCounter++;
	  BSUPacket.MsgState = MSG_STATE_BSUID;
	  DataCounter++;
#if DEBUG_PACKET_HANDLER
	  sprintf (MessageString, "message type = '%c'\n", BSUPacket.Type);
	  PmLogPrint(IKDEBUG,"%s", MessageString);
#endif
	}
      else if (BSUPacket.MsgState == MSG_STATE_BSUID)
	{
	  BSUPacket.BSUID = Data[DataCounter];
	  BSUPacket.CRCData[BSUPacket.CRCDataCounter] = Data[DataCounter];	// add BSU ID into crc check
	  BSUPacket.PacketBackup[BSUPacket.PacketBackupCounter++] = Data[DataCounter];
	  BSUPacket.CRCDataCounter++;
	  BSUPacket.MsgState = MSG_STATE_DATA;
	  DataCounter++;
#if DEBUG_PACKET_HANDLER
	  sprintf (MessageString, "BSUID = %d\n", BSUPacket.BSUID);
	  PmLogPrint(IKDEBUG,"%s", MessageString);
#endif
	}
      else if (BSUPacket.MsgState == MSG_STATE_DATA)
	{
	  if (BSUPacket.Length - DATA_LENGTH_MODIFIER - 1 > BSUPacket.DataBufferCounter)	// check data to save
	    {
#if DEBUG_PACKET_HANDLER
	      sprintf (MessageString,
		       "data buffer count : %d, CRC buffer counter : %d, Packet backup counter : %d \n",
		       BSUPacket.DataBufferCounter, BSUPacket.CRCDataCounter,
		       BSUPacket.PacketBackupCounter);
	      PmLogPrint(IKDEBUG,"%s", MessageString);
#endif
	      BSUPacket.Data[BSUPacket.DataBufferCounter] = Data[DataCounter];
	      BSUPacket.CRCData[BSUPacket.CRCDataCounter] = Data[DataCounter];	// add data into crc check
	      BSUPacket.PacketBackup[BSUPacket.PacketBackupCounter++] = Data[DataCounter];
	      BSUPacket.CRCDataCounter++;
	      BSUPacket.DataBufferCounter++;
	      DataCounter++;
	    }
	  else
	    {
	      BSUPacket.MsgState = MSG_STATE_CRC;
	    }
	}
      else if (BSUPacket.MsgState == MSG_STATE_CRC)
	{
#if DEBUG_PACKET_HANDLER
	  sprintf (MessageString, "Starting CRC with CRC counter at  = %d\n",
		   BSUPacket.CRCCounter);
	  PmLogPrint(IKDEBUG,"%s", MessageString);
#endif
	  BSUPacket.CRC[BSUPacket.CRCCounter] = Data[DataCounter];
	  DataCounter++;
	  BSUPacket.PacketBackup[BSUPacket.PacketBackupCounter++] = Data[DataCounter];
	  BSUPacket.CRCCounter++;
	  if (BSUPacket.CRCCounter == CRC_LENGTH)
	    {
	      BSUPacket.MsgState = MSG_STATE_ETX;
	      BSUPacket.CRCCounter = 0;
	    }
	}
      else if (BSUPacket.MsgState == MSG_STATE_ETX)
	{
	  BSUPacket.PacketBackup[BSUPacket.PacketBackupCounter++] = Data[DataCounter];
	  if (Data[DataCounter] == (unsigned char) ETX)	// check ETX
	    {			// good ETX so check CRC
#if DEBUG_PACKET_HANDLER
	      PmLogPrint(IKDEBUG,"Good ETX");
#endif
	      CRC = BSUPacket.CRC[0] << 24;
	      CRC |= BSUPacket.CRC[1] << 16;
	      CRC |= BSUPacket.CRC[2] << 8;
	      CRC |= BSUPacket.CRC[3];

	      CRCTest = CRC_32bit (BSUPacket.CRCData, BSUPacket.CRCDataCounter) - CRC;

	      if (!CRCTest)
		{
#if DEBUG_PACKET_HANDLER
		  PmLogPrint(IKDEBUG,"Good CRC");
#endif
		  
		  /* Fernando Contreras */
		  /*if (FC_counter == 10000) {
		  	FC_file = fopen("/Raid/tmp/FC_Packet.bin","ab");
		  	fwrite(&BSUPacket.RxPreamble, 1, 7, FC_file);
		  	fwrite(&BSUPacket.Length, 1, sizeof(BSUPacket.Length), FC_file);
		  	fwrite(&BSUPacket.Type, 1, sizeof(BSUPacket.Type), FC_file);
		  	fwrite(&BSUPacket.BSUID, 1, sizeof(BSUPacket.BSUID), FC_file);
		  	fwrite(&BSUPacket.Data, 1, BSUPacket.Length - DATA_LENGTH_MODIFIER - 1, FC_file);
		  	fwrite(&BSUPacket.CRC, 1, 4, FC_file);
			FC_etx = (unsigned char)ETX;
		  	fwrite(&FC_etx, 1, 1, FC_file);
			fclose(FC_file);
			FC_counter = 0;
		  }
		  FC_counter++;*/
		  /* End Fernando Contreras */
		  
		  if ( !BSUPacketInterpreter (&BSUPacket) ) ret = CDAS_INPUTS_ERROR; // RR +

		}
	      else
		{
		  PmLogPrint(IKERROR,"BAD CRC");
		  ret = CDAS_TRAILER_BAD;
		}
	      BSUPacket.DataBufferCounter = 0;	// on good or bad message set counters
	      BSUPacket.MsgState = MSG_STATE_IDLE;
	      BSUPacket.CRCCounter = 0;
	      BSUPacket.CRCDataCounter = 0;
	      BSUPacket.PacketBackupCounter = 0;
	      DataCounter++;
	    }
	  else
	    {
	      BSUPacket.DataBufferCounter = 0;	// on good or bad message set counters
	      BSUPacket.MsgState = MSG_STATE_IDLE;
	      BSUPacket.CRCCounter = 0;
	      BSUPacket.CRCDataCounter = 0;
	      BSUPacket.PacketBackupCounter = 0;
	      DataCounter++;
	    }
	}
      else
	{
	  PmLogPrint(IKERROR,"BAD ETX");
	  ret = CDAS_TRAILER_BAD;
	}
    }
  while (DataCounter < Length);

  return ret;
}

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
int
PmBSUConnection::BSUPacketInterpreter (struct PacketStructure *Packet)
/* DESCRIPTION:
 *   Main code ripped off from the packet handler.  Function
 *   interprets the message from the BSU and acts on it accourdingly.
 *
 * PARAMS:
 *   PacketStructure* = pointer to the packet structure used for uLSx protocol
 *
 * RETURN VALUE:
 *   = 1, if everything is fine and 0 if errors.
 */
/*-----------------------------------------------------------------------*/
{
  int ret = 1;
  static int nb_comms_packet=0;

  switch (Packet->Type)
    {
    case DATA:			// For CDAS tasks
      LsFrameP frameP;
      frameP = (LsFrameP) & Packet->Data[4];	// offset : length + frame# + reserved + reserved
      frameP->id = ntohs (frameP->id); // source coming @ = LS id
#if DEBUG_PACKET_INTERPRETER
      PmLogPrint(IKDEBUG,"Data Packet Treating frame no=%d on ls[%d] connected on BSU=%d", frameP->frameNumber, frameP->id, Packet->BSUID);
#endif
      ret = _ProcessLsFrame (frameP);
      break;

    case DATA_ACKNOWLEDGE:
#if DEBUG_PACKET_INTERPRETER
      static int PacketCounter = 0;
      PmLogPrint(IKDEBUG,">>>Data acknowledge from BSU(%d) - #%d", Packet->BSUID,PacketCounter++);
#endif
      break;

    case CONTROL:
      nb_comms_packet++;
      if (nb_comms_packet%10000==1) PmLogPrint(IKDEBUG,"%d comms monitoring data packet seen",nb_comms_packet);
#if DEBUG_PACKET_INTERPRETER
      PmLogPrint(IKDEBUG,"Comms monitoring data packet - Ignore");
#endif
      break;

    case DATA_LIMIT_EXCEEDED:
      PmLogPrint(IKDEBUG,"Too much data sent to BSU! Calm down, Antoine!");
      break;


    default:
      {
#if DEBUG_PACKET_INTERPRETER
      PmLogPrint(IKDEBUG,"Could not find packet interpreter message match for : %c", Packet->Type);
#endif
      }
    };
  return ret;
}


/*-----------------------------------------------------------------------*/
/* class PmCDASConnection.                                               */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmCDASConnection::PmCDASConnection (PmStdConnection & conn, int sock)
/* DESCRIPTION:
 *   Object connection for CDAS client, assigning services requested by the
 *   current client connected on the socket.
 *
 * PARAMS:
 *   PmStdConnection* = pointer to the connection structure
 *   sock = the current socket
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  // Properties initialization
  _name = conn.pmStdHeader.type;
  _socket = sock;
  _version = conn.pmStdHeader.protocolVersion;
  _idService = conn.pmStdHeader.type;

  // Assigning client for output data service
  for (unsigned int i = 0; i < conn.service.size (); i++)
    {
      switch (conn.service[i])
	{
	case PmCTID:
	case PmT2:
	  _pmService[i] = (PmT2Services *) theT2Services ();
	  _pmService[i]->IAmClientForOutput ();
	  break;
	case PmT2Fast:
	  _pmService[i] = (PmT2FastServices *) theT2FastServices ();
	  _pmService[i]->IAmClientForOutput ();
	  break;
	case PmEBID:
	case PmEVENT:
	  _pmService[i] = (PmEVENTServices *) theEVENTServices ();
	  _pmService[i]->IAmClientForOutput ();
	  break;
	case PmCBID:
	case PmCALIB:
	  _pmService[i] = (PmCALIBServices *) theCALIBServices ();
	  _pmService[i]->IAmClientForOutput ();
	  break;
	case PmMOID:
	case PmMoSdRaw:
	  _pmService[i] = (PmMoSdRawServices *) theMoSdRawServices ();
	  _pmService[i]->IAmClientForOutput ();
	  break;
	case PmSpmt:
	  _pmService[i] = (PmSpmtSdServices *) theSpmtSdServices ();
	  _pmService[i]->IAmClientForOutput ();
	  break;
	default:
	  IkWarningSend ("Service unknown.");
	  return;
	}
      _whichService++;
    }
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmCDASConnection::RemoveClient ()
/*-----------------------------------------------------------------------*/
{
  for (int i = 0; i < _whichService; i++)
    _pmService[i]->ForgetMe ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors PmCDASConnection::SendOutput ()
/* DESCRIPTION:
 *   Sends data stored by each services during the Listening process
 *
 * PARAMS:
 * RETURN VALUE:
 *   cdasErrors = CDAS_SUCCESS
 *                CDAS_CONNECT_CLOSED
 *                CDAS_MALLOC_ERROR
 */
/*-----------------------------------------------------------------------*/
{
  for (int i = 0; i < _whichService; i++)
    {// For each service...
#if PMDEBUG_VERBOSE
PmLogPrint(IKDEBUG,"*  Entering for sending data to CDAS services...");
#endif
      for (unsigned int j = 0; j < _pmService[i]->Outputs.size (); j++)
	{// Process data to be sent
	  cdasErrors error = _SendData (i, j);
	  if (error != CDAS_SUCCESS) return error;
	}
    }
  return CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors 
PmCDASConnection::_SendData (int service, int position)
/* DESCRIPTION:
 *   Writes data to the socket of the CDAS client, of the assigned service
 *
 * PARAMS:
 *   service = the assigned service requested by a LS
 *   position = the position of the raw data
 *
 * RETURN VALUE:
 *   cdasErrors = CDAS_SUCCESS
 *                CDAS_CONNECT_CLOSED
 *                CDAS_MALLOC_ERROR
 */
/*-----------------------------------------------------------------------*/
{
  static char data[__MAXBYTESTOWRITE__];
  longWord length_data;

  if ((length_data = _pmService[service]->Outputs[position].length)
      > (__MAXBYTESTOWRITE__ - sizeof (longWord) - PmSTDHEADERLENGTH))
    {
      IkSevereSend ("Impossible to write %d bytes, default malloc is %d",
	 length_data, __MAXBYTESTOWRITE__);
      PmLogPrint (IKERROR,"Impossible to write %d bytes, default malloc is %d",
		  length_data, __MAXBYTESTOWRITE__);
      return CDAS_MALLOC_ERROR;
    }

  // Building the PmStdHeader and the PmTrailer
  PmStdHeader pmStdHeader (length_data, _pmService[service]->TypeOfService);
  longWord trailer_addresse = PmSTDHEADERLENGTH + length_data;
  PmTrailer pmTrailer (PmMESSAGESENDMARKER);

  // Copy raw data for the streaming send
  memcpy (data, (void *) &pmStdHeader, PmSTDHEADERLENGTH);
  memcpy (data + PmSTDHEADERLENGTH, _pmService[service]->Outputs[position].output, length_data);
  memcpy (data + trailer_addresse, (void *) &(pmTrailer.id), sizeof (longWord));

  longWord bytes_written = write (_socket, data, ntohl (pmStdHeader.length));

  if (bytes_written != ntohl (pmStdHeader.length))
    {
      PmLogPrint (IKERROR,"_SendData : connection is broken on socket=%d",_socket);
      return CDAS_CONNECT_CLOSED;
    }

#if PMDEBUG
  PmLogPrint(IKDEBUG,"*  Wrote %d bytes to CDAS service=%d on socket=%d", (int)bytes_written,_pmService[service]->TypeOfService, _socket);
#endif

  return CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmDOWNLOADConnection.                                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmDOWNLOADConnection::PmDOWNLOADConnection (PmStdConnection & conn, int sock)
/*-----------------------------------------------------------------------*/
{
  // Properties initialization
  _name = conn.pmStdHeader.type;
  _socket = sock;
  _version = conn.pmStdHeader.protocolVersion;
  _idService = conn.pmStdHeader.type;

  // Service initialization
  assert (_downloadServicesP = new PmDOWNLOADServices ()); // RR +

  // To manage the timing output
  _current_time = _last_time = 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmDOWNLOADConnection::~PmDOWNLOADConnection ()
/*-----------------------------------------------------------------------*/
{
  _sockets.erase (_sockets.begin(), _sockets.end()); // RR +
}
	
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmDOWNLOADConnection::RemoveClient ()
/*-----------------------------------------------------------------------*/
{
  delete _downloadServicesP;
  _downloadServicesP = NULL;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors 
PmDOWNLOADConnection::GetInput ()
/* DESCRIPTION:
 *   Builds the Pm message block structure
 *   Links ls with the assigned bsu if the Pm status is OK
 *   Gets input data from client
 *   Stores data before outputing process
 *
 * PARAMS:
 * RETURN VALUE:
 *   cdasErrors = CDAS_SUCCESS
 *                CDAS_UNSUPPORTED_VERSION
 *                CDAS_CONNECT_CLOSED
 *                CDAS_INVALID
 *                CDAS_INPUTS_ERROR
 *                CDAS_TRAILER_BAD
 */
/*-----------------------------------------------------------------------*/
{
  int msgLen, dataLen, headerLen;
  PmStdHeader * pmStdHeaderP = NULL;
  cdasErrors ret = CDAS_SUCCESS;

  // Reading head protocol
  pmStdHeaderP = _ReadPmStdHeaderFromSocket (_socket);
  if (!pmStdHeaderP)
    return CDAS_CONNECT_CLOSED;

  msgLen = pmStdHeaderP->length;
  headerLen = pmStdHeaderP->hLength;
  dataLen = msgLen - headerLen - sizeof (longWord);

  longWord version, l_temp;
 
  /* First, check the version of the message. */
  dataLen -= read (_socket, (void *) &l_temp, sizeof (longWord));
  version = ntohl (l_temp);

  switch (version)
    {
    case 1:
      break;
    default:			// RR - temporary
      IkWarningSend ("Download GetInput: unknown version received");
      PmLogPrint (IKERROR,"Download GetInput: unknown version received");
      DiscardMess (_socket, dataLen);
      return CDAS_UNSUPPORTED_VERSION;
    }

  char * data;
  if (!(data = new char[dataLen]))
    {
      IkSevereSend ("Impossible to allocate space for downloading message");
      PmLogPrint (IKERROR,"GetInput : Impossible to allocate space for downloading message");
      return CDAS_MALLOC_ERROR;	// cdas malloc error
    }

  int read_dwnl=read (_socket, data, dataLen);
  dataLen -= read_dwnl;

  if (dataLen != 0)
    {
      dataLen -= read (_socket, &data[read_dwnl], dataLen);
      if (dataLen != 0) {
      IkWarningSend ("Download GetInput : fails on reading the entire data");
      PmLogPrint (IKERROR,"Download GetInput : fails on reading the entire data");
      delete[]data;
      return CDAS_INPUTS_ERROR;
      }
    }
/*
  // Read the raw data sent by the client
  dataLen -=read (_socket, data, dataLen);

  if (dataLen != 0)
    {
      IkWarningSend ("Download GetInput : fails on reading the entire data");
      PmLogPrint (IKERROR,"Download GetInput : fails on reading the entire data");
      delete[]data;
      return CDAS_INPUTS_ERROR;
    }
*/
  if (_CheckTrailerFromSocket (_socket) != CDAS_SUCCESS)
    {
      IkWarningSend ("Received bad trailer from download client !");
      PmLogPrint (IKERROR,"Received bad trailer from download client !");
      delete[]data;
      return CDAS_TRAILER_BAD;
    }

  // Link Ls with the assigned BSu
  if ((_downloadServicesP->GetStatusMess ()) == PmYES) {
    ret = _LinkLsWithBSuAndSocket (data);
  }

  // Call the service assigned to do this stuff!
  if ( ret == CDAS_SUCCESS ) _downloadServicesP->GetInput (data);

  delete[]data;
  return ret;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors
PmDOWNLOADConnection::_LinkLsWithBSuAndSocket (char *data)
/* DESCRIPTION:
 *   Building for the current object, a list of BSU sockets according
 *   the mode of transfer requested
 *
 * PARAMS:
 *   char * data = raw data to be sent in the download sequence 
 *
 * RETURN VALUE:
 *   cdasErrors = CDAS_SUCCESS
 *                CDAS_CONNECT_CLOSED
 */
/*-----------------------------------------------------------------------*/
{
  longWord *lP = (longWord *) data;
  longWord numberOfLs = ntohl (*lP++);

  if (numberOfLs == 0) // Broadcast mode
    _sockets = theStationDataBase ()->GetListOfSockets ();
  else if (numberOfLs == 1) // List mode
    {
      int lsId = ntohl (*lP);
      // Careful! lsId == -1, applications who is using Download should fix their
      // list of stations correctly!!
      if (lsId <= 0) return CDAS_CONNECT_CLOSED;
      int sock = theStationDataBase ()->GetSocketOfBsByLsId (lsId);
      _sockets.push_back (sock);
    }
  else
    {
      IkSevereSend("Can't download in mode list ! Only single list or broadcast.");
      PmLogPrint(IKERROR,"Can't download in mode list ! Only single list or broadcast.");
      return CDAS_CONNECT_CLOSED;
    }
  return CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors
PmDOWNLOADConnection::SendOutput ()
/*-----------------------------------------------------------------------*/
{
  cdasErrors error = CDAS_SUCCESS;
  if (_downloadServicesP->listOfOutput_.size ())
    {
      error = _SendData ();
      _downloadServicesP->ResetOutput ();
    }
  return error;
}

/*-----------------------------------------------------------------------*/

#if USING_THREAD
/*-----------------------------------------------------------------------*/
void * _thread_SendBSUPacket(void *arg)
{
  struct structthread * stthread = (struct structthread *) arg;

  fprintf(stdout,"   ** Thread BSU(%d) is running for a download send\n",stthread->bsId);

  int exitvalue = SendBSUPacket (stthread->length_data, DATA, 
				 (const unsigned char *) (stthread->data + 1), 
				 stthread->bsId, stthread->socket) ;
	
  return (void*) exitvalue;
}
/*-----------------------------------------------------------------------*/
#endif

/*-----------------------------------------------------------------------*/
cdasErrors PmDOWNLOADConnection::_SendData ()
/* DESCRIPTION:
 *   Sends the stored data to the list of BSU clients, by slices
 *
 * PARAMS:
 * RETURN VALUE:
 *   cdasErrors = CDAS_SUCCESS
 *                CDAS_MALLOC_ERROR
 *                CDAS_CONNECT_CLOSED
 */
/*-----------------------------------------------------------------------*/
{
  static char data[__MAXBYTESTOWRITETOBSU__];
  longWord length_data;
	
    if ((length_data = _downloadServicesP->listOfOutput_[0].length) > __MAXBYTESTOWRITETOBSU__ ) // RR +
    {
      PmLogPrint (IKERROR,"Impossible to write %d bytes a BSu default malloc is %d",
	 length_data, __MAXBYTESTOWRITETOBSU__);
      IkSevereSend ("Impossible to write %d bytes a BSu default malloc is %d",
	 length_data, __MAXBYTESTOWRITETOBSU__);
      return CDAS_MALLOC_ERROR;
    }

#if PMDEBUG
PmLogPrint(IKDEBUG,"*  Entering for sending data in Download mode...");
#endif
  // Copy the first element of output list to the stream data
  memcpy (data, _downloadServicesP->listOfOutput_[0].output, length_data);

  // Send the same data to all the BSu by broadcasting
  unsigned int i,j;

#if USING_THREAD // Parallel mode sending with threads
  if ( sysconf(_SC_THREADS) == -1 ) // Are threads supported???
  {
    PmLogPrint(IKFATAL,"Sorry but threads are not supported in this system...");
    IkFatalSend("Sorry but threads are not supported in this system...so no download can be done!");
    return CDAS_CONNECT_CLOSED;
  }

  // Filled up my structure for my threads
  memcpy (sthread.data, data, length_data);
  sthread.length_data = length_data;

  unsigned int idt;
  int reterror;

  j = 0;
  // Launching my threads for parallel BSU processes
  for (idt = 0; idt < _sockets.size (); idt++)
    {
      int socket = _sockets[idt];
      
      sthread.bsId = thePmServer ()-> _FindBSIdBySocket (socket);
      sthread.socket = socket;
      if ( (reterror = pthread_create(&v_threadID[idt], NULL, _thread_SendBSUPacket, (void*)&sthread)) != 0 )
	{
	  PmLogPrint(IKERROR,"pthread_create() error: %s",strerror(reterror));
	  IkFatalSend("Error on creating a thread on BSU(%d)",sthread.bsId);
	}
    }
  
  // Waiting till all my threads are dead
  for (i = j = 0; i < _sockets.size (); i++)
    {
      void * exit_status;
      pthread_join(v_threadID[i], &exit_status);
      fprintf(stdout,"   -- Thread BSU(%d) dies...\n",thePmServer ()-> _FindBSIdBySocket (_sockets[i]));
      if ( ((int*) exit_status) <= 0 )
	{
	  _sockets.erase (_sockets.begin () + i);
	  thePmServer ()->_RemoveClient (_sockets[i]);
	  thePmServer ()->_CloseConnection (_sockets[i]);
	  j++;
	}
    }
#else // Loop mode sending
  for (i = j = 0; i < _sockets.size (); i++)
    {
      int socket = _sockets[i];
      int bsId = thePmServer ()-> _FindBSIdBySocket (socket);
      if (SendBSUPacket ((int) length_data, DATA, (const unsigned char *) (data + 1), bsId, socket) <= 0)	// RR +
	{// Delete socket unused...
	  _sockets.erase (_sockets.begin () + i);
	  thePmServer ()->_RemoveClient (socket);
	  thePmServer ()->_CloseConnection (socket);
	  j++;
	}
      struct timeval tv;
      struct timezone tz;
      gettimeofday (&tv, &tz);
      PmLogPrint(IKINFO,"DownloadSending: time before sleep %d.%d",tv.tv_sec,tv.tv_usec);

      //usleep(10); // RR ! FIX UP THIS DELAY PLEASE VANCE!
    }
#endif
  // Close DOWNLOAD Connection only when all sockets seems to be closed!
  return (j == i) ? CDAS_CONNECT_CLOSED : CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------*/
