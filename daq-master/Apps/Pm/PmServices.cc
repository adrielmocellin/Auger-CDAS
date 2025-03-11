
#include "PmServices.h"
#include "PmLog.h"

/* XB XB XB */
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
/* XB XB XB */

extern string RcName;

/*-----------------------------------------------------------------------*/
/* class PmServices.                                                     */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServices::GetInput (const LsMessReference & mess)
/*-----------------------------------------------------------------------*/
{
  if (!_CountClients)
    return;

  BuildOutput (mess);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServices::BuildOutput (const LsMessReference & mess)
/* DESCRIPTION:
 *   Builds a raw data to be sent
 *
 * PARAMS:
 *   LsMessReference = the data source
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  int len_lw = sizeof (longWord) /* version */ , len;

  PmServicesOutput out;

  /* the htonl() & co are done in this constructor */
  _PmLsRawMessage tmp (mess);
  len = ntohl (tmp.mess.length) + 4;

  if (!(out.output = (char *) malloc (len + len_lw)))
    {
      IkSevereSend ("Not enough space to build output !");
      PmLogPrint ((char *)IKERROR,(char *)"BuildOutput : not enough space to build output !");
      return;
    }

  /* Pm queue infos */
  out.length = len + len_lw;

  // Fill Pm LsMessage Header in output buffer
  longWord *lP = (longWord *) out.output;
  *lP++ = htonl (1);// version
  *lP = tmp.lsId;
  
  // Fill Pm LsMessage Header in output buffer
  lP = (longWord *) out.output;
  lP += 2;
  *lP++ = tmp.mess.length;
  *lP++ = tmp.mess.type;
  *lP = tmp.mess.version;

  // Fill the data
  lP = (longWord *) out.output;
  lP += 5;
  char *cP = (char *) lP;
  memcpy (cP, tmp.mess.data, len - 16);

  /* fill list of outputs */
  Outputs.push_back (out);

}

/*-----------------------------------------------------------------------*/
void
PmServices::ResetOutput ()
/*-----------------------------------------------------------------------*/
{
  for (unsigned int i = 0; i < Outputs.size (); i++)
    {
      free (Outputs[i].output);
    }
  Outputs.clear ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServices::IAmClientForOutput ()
/*-----------------------------------------------------------------------*/
{
  _CountClients++;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServices::ForgetMe ()
/*-----------------------------------------------------------------------*/
{
  _CountClients--;

  if (!_CountClients) ResetOutput ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmLSBLOCKSServices.                                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmLSBLOCKSServices::GetLsFrame (const LsFrame & frame, int bsId)
/* DESCRIPTION:
 *   Processes the raw data coming from LS
 *   Forwards to the right service
 *
 * PARAMS:
 *   LsFrame = raw data from LS
 *   bsId
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  word n_bytes_in_mess;
  int posInData = 0;
  //int paddingBytes;
  LsMessReference *ret_saucisson = NULL;

  for (int i = 0; i < (int) frame.numberOfMessages; i++)
    {
      n_bytes_in_mess = (word) frame.data[posInData];
      /* constructor to build one local station message, with its id */
      
      //LsMessReference lsMessRef ((int) frame.id, bsId,(*((LsRawMessageP)(frame.data + posInData))));
      int BsId_int = (int)bsId; // Added FC 15-12-08
      int FrameId_int = (int)frame.id; // Added FC 15-12-08
      LsMessReference lsMessRef (FrameId_int, BsId_int,(*((LsRawMessageP)(frame.data + posInData))));      


      if (lsMessRef.rawData == NULL)
	{
	  IkSevereSend ("Received bad LsFrame from LS %d, with length %d !", frame.id, n_bytes_in_mess);
	  PmLogPrint ((char *)IKERROR,(char *)"Received bad LsFrame from LS %d, with length %d !", frame.id, n_bytes_in_mess);
	  return;
	}

      /* look at the completion, to know if we must store the partial message or process one complete */
      switch (lsMessRef.completion)
	{
	case COMPLETION_FIRST:
	case COMPLETION_NEXT:
	case COMPLETION_LAST:
	  if ((ret_saucisson = _AddPartialMessage (lsMessRef)))
	    {
	      _ProcessMess (*ret_saucisson);
	      free (ret_saucisson->rawData);
	      delete ret_saucisson;
	      // RR - _FreeRetSaucisson (ret_saucisson);
	    }
	  break;
	case COMPLETION_ALL:
	  _ProcessMess (lsMessRef);
	  free (lsMessRef.rawData);
	  break;
	default:
	  IkSevereSend ("Unknown completion received !! hmmm...seems not to be very good");
	  PmLogPrint ((char *)IKERROR,(char *)"Unknown completion received !!");
	  break;
	}

      /* next message */
      posInData += n_bytes_in_mess + 1;	// the field length doesn't include itself

    }

}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
LsMessReference *
PmLSBLOCKSServices::_AddPartialMessage (LsMessReference & lsMessRef)
/*-----------------------------------------------------------------------*/
{
  int stream = _FindStream (lsMessRef.type);
  if (stream == -1)
    {
      PmLogPrint ((char *)IKERROR,(char *)"PmLSBLOCKSServices:: stream == -1 !!");
      IkWarningSend ("Received message in unknown stream (type = %d) from %d (%d).",
	 lsMessRef.type, lsMessRef.lsId, lsMessRef.bsId);
      return NULL;
    }

  /* check the lsId isn't greater than the max value defined for the array */
  int lsId = lsMessRef.lsId;
  int messNumber = lsMessRef.messNumber;
  int index;
  LsMessReference *ret_saucisson = NULL;

  /* search previous parts of this message */
  index = _FindIndexByNumberMessAndDiscardIncompleteMess (lsId, messNumber, stream);

  /* insert rawMess in the list, and check if it's complete */
  ret_saucisson = _InsertRawMessAndBuildCompleteMess (index, stream, lsMessRef);

  /* if ret_saucisson != NULL, build lsMessRef 'output' , and clean memory */
  if (ret_saucisson)
    {
      /* could be done in _InsertRawMessAndBuildCompleteMess */
      _EraseCompleteMessage (lsId, stream, index);
      return ret_saucisson;
    }

  /* else : message non complete */
  return NULL;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmLSBLOCKSServices::_FindStream (const int &type)
/*-----------------------------------------------------------------------*/
{
  int ret;
  switch (type)
    {
      /* from Control */
    case M_READY:
    case M_RUN_START_ACK:
    case M_RUN_PAUSE_ACK:
    case M_RUN_CONTINUE_ACK:
    case M_RUN_STOP_ACK:
    case M_CONFIG_SET_ACK:
    case M_BAD_SEQUENCE:
    case M_BAD_VERSION:
    case M_MSG_ERROR:
    case M_DOWNLOAD_ACK:
    case M_OS9_CMD_ACK:
    case M_MODULE2FLASH_ACK:
    case M_LOG_SEND:
    case M_GEN_MSG:
    case M_UNKNOWN:
      ret = ControlStream;
      break;
      /* from Event */
    case M_T3_EVT:
    case M_T3_MUONS:
      ret = EventStream;
      break;
      /* from T2 */
    case M_T2_YES:
      return T2Stream;
      break;
      /* from CALMON */
    case M_MONIT_SEND:
    case M_MONIT_REQ_ACK:
    case M_CALIB_SEND:
    case M_CALIB_REQ_ACK:
      ret = CalmonStream;
      break;
/* from GPS */// later ?????
   case M_CALIB_SPMT:
     ret = GPSStream;
     break;
    default:
      ret = -1;
    }
  return ret;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmLSBLOCKSServices::_DiscardPartialMessage (int lsId, int index, int stream)
/*-----------------------------------------------------------------------*/
{
  IkWarningSend ("Discarding incomplete message from Ls %d, stream %d", lsId, stream);
  PmLogPrint ((char *)IKINFO,(char *)"Discarding incomplete message from Ls %d, stream %d", lsId, stream);
  list < LsMessReference >::iterator it = _partial_message[lsId][stream][index].begin ();
  for (unsigned int i = 0; i < _partial_message[lsId][stream][index].size (); i++)
    {
      free ((*it).rawData);
      it++;
    }
  _partial_message[lsId][stream][index].clear ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmLSBLOCKSServices::_SearchIncompleteMessage (int lsId, int number, int stream)
/* DESCRIPTION:
 *   No more than 3 messages at a time for a same stream 
 *   for a given local station   
 *
 * PARAMS:
 *   lsId
 *   number
 *   stream
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  list < LsMessReference >::iterator it;

  for (int index = 0; index < __MAXINDEX__; index++)
    {
      if (_partial_message[lsId][stream][index].size ())
	{
	  it = _partial_message[lsId][stream][index].begin ();
	  if (abs (number - (*it).messNumber) > 2)
	    {
	      if (abs (number - (*it).messNumber) < 62)
		{
		  _DiscardPartialMessage (lsId, index, stream);
		  return index;
		}
	    }
	}
    }
  return -1;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmLSBLOCKSServices::_FindIndexByNumberMessAndDiscardIncompleteMess (int lsId,
								    int number, int stream)
/* DESCRIPTION:
 *   There is currently 3 lists of sliced messages permitted for each stream on 
 *   a local station, this worst situation could happen in a nightmare where 3 
 *   monitoring messages are sliced in 2 slices and sent in 5 seconds, and reversed
 *   by the radios - 5 seconds is the maximum to get this kind of reversed messages.
 *   This can happen more simply after a reboot of a LS.
 *
 * PARAMS:
 *   lsId
 *   number
 *   stream
 *
 * RETURN VALUE:
 *   0 if nothing found or index for new message
 */
/*-----------------------------------------------------------------------*/
{
  int index;
  list < LsMessReference >::iterator it, first, end;

  /* search if this message is already in a list, or if
     it's possible to begin another one */
  for (index = 0; index < __MAXINDEX__; index++)
    {
      if (_partial_message[lsId][stream][index].size ())
	{
	  first = _partial_message[lsId][stream][index].begin ();
	  end = _partial_message[lsId][stream][index].end ();
	  if ((it = find_if (first, end, equal_mess_number (number))) != end)
	    return index;
	}
      else
	return index;		// new message
    }

  /* search then incomplete message according to radio's criterious */
  if ((index = _SearchIncompleteMessage (lsId, number, stream)) != -1)
    return index;

  /* the previous return should always be done, but just in case, try 
     to find some place, discard older incomplete message */
  _DiscardPartialMessage (lsId, 0, stream);

  return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
LsMessReference *
PmLSBLOCKSServices::_InsertRawMessAndBuildCompleteMess (int index,
							int stream, LsMessReference &mess)
/* DESCRIPTION:
 *   Everything is automatic for the insertion with the 
 *   predicats defined in PmServices.h .
 *
 * PARAMS:
 *   index
 *   stream
 *   LsMessReference
 *
 * RETURN VALUE:
 *   LsMessReference *
 */
/*-----------------------------------------------------------------------*/
{
  int lsId = mess.lsId;
  int slice = mess.slice;
  LsMessReference *ret_saucisson = NULL;

  list < LsMessReference >::iterator it,
    first = _partial_message[lsId][stream][index].begin (),
    end = _partial_message[lsId][stream][index].end ();

  /* number slice is only 6 bits */
  if ( _ResetSliceNumber (lsId, stream, index, slice) )
    {
      int size=_partial_message[lsId][stream][index].size();
      while (abs(size-slice)>10) {
        slice += 64;
        mess.slice += 64;
	if (slice>4096) slice=mess.slice=size; // protection in case algorithm goes crazy...
      }
    }

  /* insert partial message */
  it = find_if (first, end, sup_slice_number (slice));
  _partial_message[lsId][stream][index].insert (it, mess);

  /* check completeness of total message */
  if ((_partial_message[lsId][stream][index].back ()).completion ==
      COMPLETION_LAST)
    {
      if (((_partial_message[lsId][stream][index].back ()).slice + 1) ==
	  (int) _partial_message[lsId][stream][index].size ())
	{
	  ret_saucisson =
	    _BuildMessageWithCompleteRawMessage (lsId, stream, index);
	  return ret_saucisson;
	}
    }

  return NULL;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmLSBLOCKSServices::_ResetSliceNumber (const int lsId, const int stream, 
                                       const int index, const int slice)
/* DESCRIPTION:
 *   The value of __min_received_slices_to_reset__ is chosen from the guaranty of 
 *   radio's to don't reverse messages during more than 5 seconds, and the only 
 *   messages which can arrived in n > 64 slices are T3's, sending during 
 *   n seconds consecutively.
 *
 * PARAMS:
 *   lsID
 *   stream
 *   index
 *   slice
 *
 * RETURN VALUE:
 *   0 or 1
 */
/*-----------------------------------------------------------------------*/
{
  unsigned int size = _partial_message[lsId][stream][index].size ();
  list < LsMessReference >::iterator it,
    first = _partial_message[lsId][stream][index].begin (),
    end = _partial_message[lsId][stream][index].end ();

  /* it == end if this number slice already exits */
  it = find_if (first, end, equal_slice_number (slice));
  if (it != end && size > __MINRECEIVEDSLICESTORESET__)
    return 1;

  return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
LsMessReference *
PmLSBLOCKSServices::_BuildMessageWithCompleteRawMessage (int lsId,
							 int stream, int index)
/*-----------------------------------------------------------------------*/
{
  LsMessReference *ret = NULL;
  list < LsMessReference >::iterator it = _partial_message[lsId][stream][index].begin ();
  unsigned int needed = 0, total = 0;

  if (!(ret = new LsMessReference ()))
    {
      IkSevereSend ("Unable to create a LsMessReference !!");
      PmLogPrint ((char *)IKERROR,(char *)"_BuildMessageWithCompleteRawMessage : unable to create a LsMessReference !!");
      _EraseCompleteMessage (lsId, stream, index);
      return NULL;
    }

  /* rebuilding header */
  ret->type = (*it).type;
  ret->lsId = (*it).lsId;
  ret->bsId = (*it).bsId;
  ret->version = (*it).version;
  ret->messNumber = (*it).messNumber;
  ret->slice = 0;		// rien à f...
  ret->completion = 1;		// tests
  ret->rawData = NULL;		// crucial in order to use realloc

  /* pasting data */
  for (unsigned int i = 0; i < _partial_message[lsId][stream][index].size ();
       i++)
    {
      if (!(needed = (*it).length))
	{
	  it++;
	  continue;
	}
      total += needed;
      if (!(ret->rawData = (char *) realloc (ret->rawData, total)))
	{
	  IkSevereSend ("Unable to realloc a LsMessReference !!");
	  PmLogPrint ((char *)IKERROR,(char *)"_BuildMessageWithCompleteRawMessage : unable to realloc a LsMessReference !!");
	  _EraseCompleteMessage (lsId, stream, index);
	  return NULL;
	}
      memcpy (ret->rawData + total - needed, (*it).rawData, needed);
      it++;
    }

  ret->length = total;

  return ret;
}

	/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmLSBLOCKSServices::_EraseCompleteMessage (int lsId, int stream, int index)
/*-----------------------------------------------------------------------*/
{
  list < LsMessReference >::iterator it =
    _partial_message[lsId][stream][index].begin ();

  /* erase list and delete memory */
  for (unsigned int i = 0; i < _partial_message[lsId][stream][index].size ();
       i++)
    {
      free ((*it).rawData);
      it++;
    }
  _partial_message[lsId][stream][index].clear ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmLSBLOCKSServices::_ProcessMess (const LsMessReference & lsMess)
/* DESCRIPTION:
 *   For each LS Message read sliced correctly, we forward to the service
 *   which is assigned for the stuff
 *
 * PARAMS:
 *   LsMessReference
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  // Message type received by the LS
  switch (lsMess.type)
    {
    case M_READY:
    case M_RUN_START_ACK:
    case M_RUN_STOP_ACK:
    case M_RUN_PAUSE_ACK:
    case M_RUN_CONTINUE_ACK:
    case M_OS9_CMD_ACK:
      _JustNotifyRC (lsMess);
      break;
    case M_T2_YES:
      theT2Services ()->GetInput (lsMess);
      break;
    case M_T3_EVT:
    case M_T3_MUONS:
      theEVENTServices ()->GetInput (lsMess);
      break;
    case M_CALIB_SEND:
    case M_CALIB_REQ_ACK:
      theCALIBServices ()->GetInput (lsMess);
      break;
    case M_MONIT_SEND:
    case M_MONIT_REQ_ACK:
      theMoSdRawServices ()->GetInput (lsMess);
      break;
    case M_CALIB_SPMT:
      theSpmtSdServices ()->GetInput (lsMess);
      break;
    case M_DOWNLOAD_ACK:
      _JustNotifyRC (lsMess);
      break;
    case M_CONFIG_SET_ACK:
    case M_MODULE2FLASH_ACK:
    case M_SET_PARAM_ACK:
      _ProcessNotDefined (lsMess);
      break;
    case M_LOG_SEND:
      theUPLOADServices ()->GetInput (lsMess);
      break;
    case M_GEN_MSG:
      _ProcessGenMess (lsMess);
      break;
    case M_BAD_SEQUENCE:
    case M_BAD_VERSION:
    case M_MSG_ERROR:
    case M_UNKNOWN:
      _ProcessError (lsMess);
      break;
    default:
      fprintf(stdout,"Unknown message type received from LS[%d]!!!!!!\n",lsMess.lsId);
      IkSevereSend("Unknown message type received from LS[%d]!!!!!!\n",lsMess.lsId);
      break;
    }
}

/*-----------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/
void
PmLSBLOCKSServices::_ProcessNotDefined (const LsMessReference & lsMess)
/*------------------------------------------------------------------------*/
{
  IkInfoSend ("received type not defined %d, version %d, length %d from %d",
	      lsMess.type, lsMess.version, lsMess.length, lsMess.lsId);
}

/*------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmLSBLOCKSServices::_ProcessError (const LsMessReference & lsMess)
/*-----------------------------------------------------------------------*/
{
  IkInfoSend ("received type %d, version %d, length %d, from %d : %s",
	      lsMess.type, lsMess.version, lsMess.length, lsMess.lsId,
	      lsMess.rawData);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmLSBLOCKSServices::_ProcessGenMess (const LsMessReference & lsMess)
/*-----------------------------------------------------------------------*/
{
  IkLsGenMess ikm;
  ikm.msg = strndup (lsMess.rawData, lsMess.length);
  ikm.StationId = lsMess.lsId;
  ikm.send (LOG);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmLSBLOCKSServices::_JustNotifyRC (const LsMessReference & lsMess)
/*-----------------------------------------------------------------------*/
{
  longWord *l_temp;
  word *s_temp;

  switch (lsMess.type)
    {
    case M_READY:
      {
	IkLsReady ikm;
	short toto;
	unsigned int titi;
	char tutu[__MAXFILENAME__];
	switch (lsMess.version)
	  {
	  case 0:
	  case 1:
	    s_temp = (word *) lsMess.rawData;
	    ikm.StationId = ntohs (*s_temp++);
	    toto = ntohs (*s_temp++);
	    ikm.ConfigFromFlash = toto & 1;
	    ikm.RunEnable = toto >> 7 & 1;
	    l_temp = (longWord *) s_temp;
	    ikm.ConfigVersion = ntohl (*l_temp++);
	    ikm.X = ntohl (*l_temp++);
	    ikm.Y = ntohl (*l_temp++);
	    ikm.Z = ntohl (*l_temp);
	    ikm.send (RcName);
	    break;
	  case 2:
	    s_temp = (word *) lsMess.rawData;
	    ikm.BsId = lsMess.bsId;
	    ikm.StationId = ntohs (*s_temp++);
	    toto = *s_temp++;
	    ikm.ConfigFromFlash = toto & 1;
	    ikm.RunEnable = toto >> 7 & 1;
	    ikm.PowerOn = toto >> 15 & 1;
	    l_temp = (longWord *) s_temp;
	    titi = ntohl (*l_temp++);
	    sprintf (tutu, "V%uR%uB%uP%u", (titi >> 24) & 0xFF,
		     (titi >> 16) & 0xFF, (titi >> 8) & 0xFF, titi & 0xFF);
	    ikm.SoftVersion = tutu;
	    ikm.ConfigVersion = ntohl (*l_temp++);
	    ikm.X = ntohl (*l_temp++);
	    ikm.Y = ntohl (*l_temp++);
	    ikm.Z = ntohl (*l_temp++);
	    ikm.CurrentTime = ntohl (*l_temp);
	    ikm.send (RcName);
	    break;
	  default:
	    IkWarningSend ("Received M_READY message in an unknown version !");
	    break;
	  }
	break;
      }
    case M_RUN_START_ACK:
      {
	IkLsStartAck ikm;
	ikm.StationId = lsMess.lsId;
	ikm.send (RcName);
	break;
      }
    case M_RUN_STOP_ACK:
      {
	IkLsStopAck ikm;
	ikm.StationId = lsMess.lsId;
	ikm.send (RcName);
	break;
      }
    case M_RUN_PAUSE_ACK:
      {
	IkLsPauseAck ikm;
	ikm.StationId = lsMess.lsId;
	ikm.send (RcName);
	break;
      }
    case M_RUN_CONTINUE_ACK:
      {
	IkLsContAck ikm;
	ikm.StationId = lsMess.lsId;
	ikm.send (RcName);
	break;
      }
    case M_OS9_CMD_ACK:
      {
	IkLsOS9Ack ikm;
	ikm.StationId = lsMess.lsId;
	ikm.Status = ntohl (*((longWord *) lsMess.rawData));
	ikm.send (RcName);
	break;
      }
    case M_DOWNLOAD_ACK:
      {
	IkLsDownloadAck ikm;
	ikm.StationId = lsMess.lsId;
	word *sP = (word *) lsMess.rawData;
	ikm.CheckId = ntohs (*sP++);
	int missingNumber = ntohs (*sP++);
	for (int i = 0; i < missingNumber; i++)
	  ikm.MissingSlices.push_back (ntohs (*sP++));
	ikm.send (LOG);
	break;
      }
    }
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmBROADCASTServices.                                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBROADCASTServices::GetInput (IkMessage * ikm, char *mode, int type)
/* DESCRIPTION:
 *   Awakened by an IkMessage for a request to LSs excepted for IKLSLOGREQ mode
 *
 * PARAMS:
 *   IkMessage *
 *   mode = BROADCAST or LIST
 *   type = ikmessage type
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  // 
  unsigned int stat_size = ((IkPmMessage*)ikm)->addresses.size();
  // maximum CUTAT: 26
  #define CUTAT 26
  unsigned int start_at=0;
  if (type == IKT3)
    {
      while (start_at<stat_size) {
 //       cout << (short)((IkT3*)ikm)->id << ": One BSU needs splitting ("<< start_at << " " << stat_size << ")" << endl;
	unsigned int nb_stat=stat_size-start_at;
	if (nb_stat>CUTAT) nb_stat=CUTAT;
        PmMessReference tmpMess1;
        tmpMess1.slice = 0;
        tmpMess1.pointerData = 0;
        tmpMess1.length = 0;
        int s = strlen (mode);
        tmpMess1.mode = (char*) malloc (s+1);
        strncpy (tmpMess1.mode, mode, s+1);
        tmpMess1.rawData = NULL; // !! each message doesn't data !!
        
        //      for (unsigned int i=0; i<((IkPmMessage*)ikm)->addresses.size(); i++)
        for (unsigned int i=0; i<nb_stat; i++)
  	tmpMess1.ls.push_back (((IkPmMessage*)ikm)->addresses[i+start_at]);
  
        tmpMess1.priority = pEVTSVR; // RR +
        tmpMess1.type = M_T3_YES;
        //      tmpMess.length += LSRAWMESSAGEHEADERLENGTH + 10 + 2*((IkT3*)ikm)->addresses.size();
        tmpMess1.length += LSRAWMESSAGEHEADERLENGTH + 10 + 2*nb_stat;
        tmpMess1.rawData = (char*) malloc (tmpMess1.length-LSRAWMESSAGEHEADERLENGTH);
        longWord* lP; word* sP; unsigned char* cP;
        sP = (word*)tmpMess1.rawData;
        /* event identifier */
        *sP++ = htons((short)((IkT3*)ikm)->id);
        /* ref second */
        lP = (longWord*) sP; 
        *lP++ = htonl((longWord)((IkT3*)ikm)->refSecond); 
        /* ref usecond */
        *lP++ = htonl((longWord)((IkT3*)ikm)->refuSecond);
        /* offset and window for each ls */
        cP = (unsigned char*) lP;
        //      for (unsigned int i=0;i<((IkT3*)ikm)->addresses.size();i++)
        for (unsigned int i=0;i<nb_stat;i++)
  	{
  	  *cP++ = (unsigned char)((IkT3*)ikm)->offsets[i+start_at]; 
  	  *cP++ = (unsigned char)((IkT3*)ikm)->window[i+start_at];
  	}	
        
        list < PmMessReference >::iterator
  	first = _messToSend.begin (),
  	end = _messToSend.end (),
  	it = find_if (first, end, sup_priority (tmpMess1.priority));
        _messToSend.insert (it, tmpMess1);
	start_at+=nb_stat;
      }
      return;
    }

  // Insert PmMessage format in a list built from an Ik message
  PmMessReference tmp (ikm, mode, type);

  list < PmMessReference >::iterator
    first = _messToSend.begin (),
    end = _messToSend.end (),
    it = find_if (first, end, sup_priority (tmp.priority));
  _messToSend.insert (it, tmp);

}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBROADCASTServices::GetInput (const PmUPLOADMessage & mess)
/* DESCRIPTION:
 *   Awakened by an upload data incoming
 *
 * PARAMS:
 *   PmUPLOADMessage
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  // Insert PmMessage format in a list built from PmUPLOAD message
  PmMessReference tmp (mess);

  list < PmMessReference >::iterator
    first = _messToSend.begin (),
    end = _messToSend.end (),
    it = find_if (first, end, sup_priority (tmp.priority));
  _messToSend.insert (it, tmp);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBROADCASTServices::BuildOutput ()
/* DESCRIPTION:
 *   In the ProcessInterrupts phasis, we build message to be send to BSU
 *
 * PARAMS:
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  if (_messToSend.size () == 0)
    return;

  PmMessReference & mess = *(_messToSend.begin ());

  int totalLengthOfCsFrame = _CountLengthOfCsFrame (mess);
  if (totalLengthOfCsFrame == -10)	// can't happend in principle // RR - why not??
    {
      IkSevereSend ("Can't count CsFrame length for broadcast");
      PmLogPrint ((char *)IKERROR,(char *)"Can't count CsFrame length for broadcast");
      return;
    }

  CsFrame csFrame;
  LsRawMessage rawMess;

  // Contructing message in a CsFrame structure
  _FillCsFrameHeader (csFrame, totalLengthOfCsFrame, mess.mode, mess.ls);
  _FillLsRawMess (rawMess, mess);
  _FillLsRawMessInCsFrame (rawMess, csFrame, mess.mode);

  // +5 because of the long version with lsx, and the field length itself
  lengthOutputData_ = totalLengthOfCsFrame + 5;
  _ReallocOutputDataAndSetVersion ();
  _FillCsFrameInOutputData (csFrame);

  // Stores in a vector data to be sent to BSU
  _AddOutputInList ();

  // Check the end of sliced message received
  if (mess.completion == COMPLETION_ALL || mess.completion == COMPLETION_LAST) // RR - one func deleted
    {
      free (mess.mode);
      free (mess.rawData);
      _messToSend.erase (_messToSend.begin ());
    }
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBROADCASTServices::_FillCsFrameHeader (CsFrame & csFrame, int len,
					 char *mode, const vector < int >&addr)
/*-----------------------------------------------------------------------*/
{
  if (strcmp (mode, mLIST) == 0) // RR - using spec macros
    {
      int size = 0;
      csFrame.length = (unsigned char) (len);
      csFrame.address.list.mode = (unsigned char) CLFRAMELIST;
      csFrame.address.list.number = (unsigned char) addr.size ();
      for (unsigned int i = 0; i < addr.size (); i++)
	{
	  *((short *) (csFrame.data + size)) = htons ((short) addr[i]);
	  size += sizeof (short);
	}
    }
  else if (strcmp (mode, mBROADCAST) == 0) // RR - using spec macros
    {
      csFrame.length = (unsigned char) (len);
      csFrame.address.list.mode = (unsigned char) CLFRAMEBROADCAST;
      csFrame.address.list.number = (unsigned char) 0;
    }
  else if (strcmp (mode, mANTILIST) == 0) // RR - using spec macros
    {
			// to do
    }
  else if (strcmp (mode, mSLIST) == 0) // RR - using spec macros
    {
      csFrame.length = (unsigned char) (len);
      csFrame.address.single = htons ((short) addr[0]);
    }
  else
    {
      IkSevereSend ("Unknown transfer mode! see the spec please");
      PmLogPrint ((char *)IKERROR,(char *)"_FillCsFrameHeader : Unknown transfer mode! see the spec please");
    }
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBROADCASTServices::_FillLsRawMess (LsRawMessage & rawMess, PmMessReference & mess)
/*-----------------------------------------------------------------------*/
{
  unsigned int lenDestination = 0;
  char *cP;

  if (strcmp (mess.mode, mLIST) == 0) // RR - using spec macros
    lenDestination += mess.ls.size () * 2;

  if ((mess.length - mess.pointerData) > (CSFRAMEDATALENGTH - lenDestination))
    // message en tranches
    {
      rawMess.size = (unsigned char) (CSFRAMEDATALENGTH - lenDestination - 1);
      if (mess.pointerData == 0)	// première partie ?
	{
	  _messNumber++;
	  __num_mess__++;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	  rawMess.completion = (unsigned char) (COMPLETION_FIRST >> 6);
#else
	  rawMess.completion = (unsigned char) (COMPLETION_FIRST << 6);
#endif
	  mess.completion = (unsigned char) (COMPLETION_FIRST);
	}
      else			// suite
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	  rawMess.completion = (unsigned char) (COMPLETION_NEXT >> 6);
#else
	  rawMess.completion = (unsigned char) (COMPLETION_NEXT << 6);
#endif
	  mess.completion = (unsigned char) (COMPLETION_NEXT);
	}
      cP = mess.rawData + mess.pointerData;
      memcpy (rawMess.data, cP, rawMess.size - (LSRAWMESSAGEHEADERLENGTH - 1));
      mess.pointerData += (CSFRAMEDATALENGTH - LSRAWMESSAGEHEADERLENGTH - lenDestination);
    }
  else
    {
      rawMess.size = (unsigned char) (mess.length - mess.pointerData - 1);
      if (mess.pointerData != 0)	// dernière partie ?
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	  rawMess.completion = (unsigned char) (COMPLETION_LAST >> 6);
#else
	  rawMess.completion = (unsigned char) (COMPLETION_LAST << 6);
#endif
	  mess.completion = (unsigned char) (COMPLETION_LAST);
	}
      else			// un seul slice
	{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	  rawMess.completion = (unsigned char) (COMPLETION_ALL >> 6);
#else
	  rawMess.completion = (unsigned char) (COMPLETION_ALL << 6);
#endif
	  mess.completion = (unsigned char) (COMPLETION_ALL);
	}
      cP = mess.rawData + mess.pointerData;
      memcpy (rawMess.data, cP, rawMess.size - (LSRAWMESSAGEHEADERLENGTH - 1));
      mess.pointerData += (mess.length - mess.pointerData - LSRAWMESSAGEHEADERLENGTH - lenDestination);
    }
  rawMess.slice = (unsigned char) (mess.slice++);
  rawMess.mess = (unsigned char) (__num_mess__);
  //  rawMess.mess = (unsigned char) (_messNumber);
  rawMess.version = (unsigned char) (0);
  rawMess.type = (unsigned char) mess.type;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBROADCASTServices::_FillLsRawMessInCsFrame (LsRawMessage & rawMess,
					      CsFrame & cs, char *mode)
/*-----------------------------------------------------------------------*/
{
  short number;
  if (strcmp (mode, mSLIST) == 0) // RR - using spec macro
    number = 0;
  else
    number = cs.address.list.number;

  memcpy (cs.data + sizeof (short) * number, &rawMess, rawMess.size + 1);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmBROADCASTServices::_CountLengthOfCsFrame (const PmMessReference & mess)
/* DESCRIPTION:
 *   define MAXCSFRAMELENGTH 150 has changed in 149 ???
 *   but not in our include file !!!!!!!
 *   so I take MAXCSFRAMELENGTH 145, just to be sure
 *
 * PARAMS:
 *   PmMessReference
 *
 * RETURN VALUE:
 *   length of CS frame
 */
/*-----------------------------------------------------------------------*/
{
  int len = 0;

  if (strcmp (mess.mode, mLIST) == 0)
    len += mess.ls.size () * 2;

  len += (CSFRAMEHEADERLENGTH - 1) + mess.length - mess.pointerData;

  if ( ((len / MAXCSFRAMELENGTH) + ((len % MAXCSFRAMELENGTH) ? 1 : 0)) > 1 )
    return (MAXCSFRAMELENGTH - 1);

  return len;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBROADCASTServices::_ReallocOutputDataAndSetVersion ()
/*-----------------------------------------------------------------------*/
{
  OutputData_ = NULL;
  OutputData_ = (char *) malloc (lengthOutputData_);
  static int NbPackets = 1; // RR +
  if (!OutputData_)
    {
      IkSevereSend ("Can't allocate space to send broadcast message!");
      PmLogPrint ((char *)IKERROR,(char *)"Can't allocate space to send broadcast message!");
      return;
    }
  *((longWord *) OutputData_) = htonl (NbPackets++); // Field: packet number
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBROADCASTServices::_FillCsFrameInOutputData (CsFrame & cs)
/*-----------------------------------------------------------------------*/
{
  memcpy (OutputData_ + 4, &cs, cs.length + 1);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBROADCASTServices::_AddOutputInList ()
/* DESCRIPTION:
 *   Push in a list the data processed and ready to be sent in the next step
 *
 * PARAMS:
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  PmServicesOutput tmp (lengthOutputData_, OutputData_);
  listOfOutput_.push_back (tmp);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmBROADCASTServices::ResetOutput ()
/*-----------------------------------------------------------------------*/
{
  free (listOfOutput_[0].output);
  listOfOutput_.erase (listOfOutput_.begin ());
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmLTRASHServices.                                               */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmLTRASHServices * PmLTRASHServices::_instance = new PmLTRASHServices ();

PmLTRASHServices *
PmLTRASHServices::Instance ()
{
  return _instance;
}

PmLTRASHServices *
theLTRASHServices ()
{
  return PmLTRASHServices::Instance ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmLTRASHServices::GetInput (IkMessage * ikm)
/*-----------------------------------------------------------------------*/
{
  IkLsRecord *record = (IkLsRecord *) ikm;

  theStationDataBase ()->RequestedByTrashService (record->Station, record->StartStop);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmLTRASHServices::GetInput (LsFrameP frameP, int &lsId)
/*-----------------------------------------------------------------------*/
{
  snprintf (_trashFileName, __MAXFILENAME__, "%sstation%d", PMTRASHDIR, lsId);
  _trashFile = fopen (_trashFileName, "a");
  if (!_trashFile)
    {
      IkSevereSend ("Unable to open %s", _trashFileName);
      PmLogPrint ((char *)IKERROR,(char *)"Unable to open %s", _trashFileName);
      return;
    }

  int length = (int) frameP->length;
  fwrite (frameP, sizeof (char), length + sizeof (char), _trashFile);

  fclose (_trashFile);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmRADIOTRASHServices.                                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmRADIOTRASHServices::GetInput (char *data, int &len, int &bsId)
/*-----------------------------------------------------------------------*/
{
  snprintf (_trashFileName, __MAXFILENAME__, "%sradio_bs_id_%d", PMTRASHDIR, bsId);
  _trashFile = fopen (_trashFileName, "a");
  if (!_trashFile)
    {
      IkSevereSend ("Unable to open %s", _trashFileName);
      PmLogPrint ((char *)IKERROR,(char *)"Unable to open %s", _trashFileName);
      return;
    }

  fwrite (data, sizeof (char), len, _trashFile);

  fclose (_trashFile);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmT2Services.                                                   */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmT2Services * PmT2Services::_instance = new PmT2Services ();

PmT2Services *
PmT2Services::Instance ()
{
  return _instance;
}

PmT2Services *
theT2Services ()
{
  return PmT2Services::Instance ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmT2Services::PmT2Services () 
/*-----------------------------------------------------------------------*/
{
  _lastTime = _firstFiveSec = 0;
  TypeOfService = PmT2;
  for (int i = 0; i < 6; i++) _t2Second[i] = 0;
  _verboseCounter = 0;
}

/*-----------------------------------------------------------------------*/
void
PmT2Services::GetInput (const LsMessReference & lsMess)
/*-----------------------------------------------------------------------*/
{
  unsigned int second, index, index2;
  int lsId;

  /* first, discard if this packet is in future or past */
  lsId = lsMess.lsId;
  second = ntohl (*(unsigned int *) lsMess.rawData);
  index = second % 6;

  unsigned int tm = (unsigned int) time (NULL);
  /* 6 Jan 1980 0:0:0 = 315964800 seconds UNIC epoch */
  tm -= 315964800;
  //tm += 15;
  //tm += 16; // leap second until 1/03/2013 FC <fcontreras@auger.org.ar>
  tm += 17; // leap second 01/Jul/2015 <fcontreras@auger.org.ar>
  /* 13 seconds +- 1 per year ? == 60 
   * if we are above, this packet is not good */
  if (second > tm)
    {
      if (tm > _verboseCounter+30) // XB Sat May 15 17:54:00 UTC 2004: using a global for all stations to send 1 message/30sec
	{
	  IkWarningSend ("%d sent t2's from the future: %d > %d , discarding",
			 lsId, second, tm);
	  _verboseCounter = tm;
	}
      return;
    }
  /* in case of stop/start t2's */
  if (second < (tm - 5))
    {
      if (tm > _verboseCounter+30)
	{
	  IkWarningSend ("%d sent t2's from the past: %d < %d-5, discarding",
			 lsId, second, tm);
	  _verboseCounter = tm;
	}
      return;
    }

  if (second > _lastTime)
    {
      if (_firstFiveSec < 5)
	_firstFiveSec++;
      else
	{
	  /* send out of date buffers */
	  index2 = index + 1;
	  if (index2 == 6)
	    index2 = 0;
	  /* send buffers index2 to clients */
/* XB XB XB XB XB XB XB XB start dumping T2s XB XB XB XB XB XB */
          int indexxb=index2-3;
	  if (indexxb<0) indexxb+=6;
          if (1) theT2FastServices()->_BuildOutput(indexxb);
          //if (1) theT2FastServices()->_BuildOutput(indexxb);
/*          if (0) for (int indexxb=0;indexxb<6;indexxb++) {
            int indexxb2=indexxb-index2;
            if (indexxb2<0) indexxb2+=6;
            char fnamexb[128];
            snprintf(fnamexb,128,"/Raid/tmp/t2spy%d.dat",indexxb2);
            FILE * fxb=fopen(fnamexb,"a");
            for (unsigned int i = 0; i < _t2Buffer[indexxb].size (); i++) {
              fwrite(&(_t2Buffer[indexxb][i].id),sizeof(short),1,fxb);
              fwrite(&(_t2Buffer[indexxb][i].number),sizeof(short),1,fxb);
              fwrite(&(_t2Buffer[indexxb][i].second),sizeof(longWord),1,fxb);
              fwrite(_t2Buffer[indexxb][i].data,_t2Buffer[indexxb][i].number * sizeof (T2Ls),1,fxb);
            }
            fclose(fxb);
          }*/
/* XB XB XB XB XB XB XB XB end dumping T2s XB XB XB XB XB XB */
	  if (_CountClients)
	    _BuildOutput (index2);
	  _CleanMemory (index2);
	}
      _lastTime = second;
    }

  /* record this packet */
  _AddT2InListIndexedBySecond (lsMess, index);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmT2Services::_AddT2InListIndexedBySecond (const LsMessReference & lsMess, int index)
/*-----------------------------------------------------------------------*/
{
  unsigned int second, number, size, lsId, loop;
  T2LsBuffer t2Buffer = NULLT2LSBUFFER;

  lsId = lsMess.lsId;
  /* second is the first longWord */
  size = lsMess.length - sizeof (longWord);
  second = ntohl (*(unsigned int *) lsMess.rawData);

  if (_t2Second[index] != second)
    {
      for (unsigned int i = 0; i < _t2Buffer[index].size (); i++)
	free ((_t2Buffer[index][i]).data);
      _t2Buffer[index].clear ();
    }
  _t2Second[index] = second;

  for (loop = 0; loop < _t2Buffer[index].size (); loop++)
    if ((_t2Buffer[index][loop]).id == lsId)
      {
	t2Buffer = _t2Buffer[index][loop];
	break;
      }

  /* do not overwrite (more) recent data */
  if ((second == t2Buffer.second) && (t2Buffer.number > 0))
    {
      IkWarningSend (" station %d sent twice data for second %d, discarding",
		     lsId, second);
      return;
    }

  /* overwrite (more) recent data */
  if (t2Buffer.number > 0)
    {
      IkWarningSend ("discarding %d T2(s) from station %d, second %d to fill second %d",
	 t2Buffer.number, lsId, t2Buffer.second, second);
      free ((_t2Buffer[index][loop]).data);
      _t2Buffer[index].erase (_t2Buffer[index].begin () + loop);
      t2Buffer.number = 0;
      t2Buffer.availableSize = 0;
      t2Buffer.data = NULL;
    }

  /* fill data */
  t2Buffer.id = lsId;
  t2Buffer.second = second;
  number = size / sizeof (T2Ls);
  T2LsP t2_temp = NULL;
  if ((t2Buffer.number + number) > t2Buffer.availableSize)
    {
      t2_temp = (T2LsP) realloc (t2Buffer.data, sizeof (T2Ls) * (t2Buffer.availableSize + number));
      if (!t2_temp)
	{
	  IkSevereSend ("ALARM : couldn't allocate space for t2 data ! ");
	  PmLogPrint ((char *)IKERROR,(char *)"ALARM : couldn't allocate space for t2 data ! ");
	  return;
	}
      else
	{
	  t2Buffer.data = t2_temp;
	  t2Buffer.availableSize = number;
	}
    }
  memcpy (t2Buffer.data + t2Buffer.number, (T2LsP) (lsMess.rawData + sizeof (longWord)),
	  sizeof (T2Ls) * number);
  t2Buffer.number += number;
  _t2Buffer[index].push_back (t2Buffer);

}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmT2Services::_BuildOutput (int index)
/*-----------------------------------------------------------------------*/
{
  /* should also take into account client version !! */
  /* versoin (=2) , list<lsId,number,second,data> */
  int total, needed;
  longWord length;

  PmServicesOutput out;

  /* version */
//  if (!((longWord *) out.output = (longWord *) realloc (out.output, sizeof (longWord))))
  if (!( out.output = (char *) realloc (out.output, sizeof (longWord))))
    {
      PmLogPrint((char *)IKERROR,(char *)"Error realloc() %s",strerror(errno));
      IkFatalSend("Error realloc() %s\n",strerror(errno));
      exit (errno);
    }
  *((longWord *) out.output) = htonl (2);	/* version number */
  length = sizeof (longWord);
  /* second */
//  if (!((longWord *) out.output =  (longWord *) realloc (out.output, 2 * sizeof (longWord))))
  if (!(out.output =  (char *) realloc (out.output, 2 * sizeof (longWord))))
    {
      PmLogPrint((char *)IKERROR,(char *)"Error realloc() %s",strerror(errno));
      IkFatalSend("Error realloc() %s\n",strerror(errno));
      exit (errno);
    }
  *((longWord *) (out.output + sizeof (longWord))) = htonl (_t2Second[index]);	/* second */
  length += sizeof (longWord);

  /* build message. */
  for (unsigned int i = 0; i < _t2Buffer[index].size (); i++)
    {
      total = needed = (_t2Buffer[index][i]).number * sizeof (T2Ls);
      if (needed & 0x1) total++;		/* padding */
      if (needed & 0x10) total += 2;		/* padding */
      if (!(out.output = (char *) realloc (out.output, length + total +
					   2 * sizeof (short) +
					   sizeof (longWord))))
	{
	  IkSevereSend ("Forward T2: enable to allocate space for %d t2 %X",
			(_t2Buffer[index][i]).number,
			(_t2Buffer[index][i]).id);
	  PmLogPrint ((char *)IKERROR,(char *)"Forward T2: enable to allocate space for %d t2 %X",
		      (_t2Buffer[index][i]).number, (_t2Buffer[index][i]).id);
	  break;
	}
      *((short *) (out.output + length)) = htons ((_t2Buffer[index][i]).id);
      length += sizeof (short);
      *((short *) (out.output + length)) = htons ((_t2Buffer[index][i]).number);
      length += sizeof (short);
      *((longWord *) (out.output + length)) = htonl ((_t2Buffer[index][i]).second);
      length += sizeof (longWord);
      memcpy (out.output + length, (char *) (_t2Buffer[index][i]).data, needed);
      length += total;		/* padding */
    }
  lengthOutput = length;
  out.length = length;

  Outputs.push_back (out);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmT2Services::_CleanMemory (int index)
/*-----------------------------------------------------------------------*/
{
  for (unsigned int i = 0; i < _t2Buffer[index].size (); i++)
    {
      free ((_t2Buffer[index][i]).data);
    }
  _t2Buffer[index].clear ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmT2FastServices.                                                   */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmT2FastServices * PmT2FastServices::_instance = new PmT2FastServices ();

PmT2FastServices *
PmT2FastServices::Instance ()
{
  return _instance;
}

PmT2FastServices *
theT2FastServices ()
{
  return PmT2FastServices::Instance ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmT2FastServices::PmT2FastServices () 
/*-----------------------------------------------------------------------*/
{
  TypeOfService = PmT2Fast;
}

/*-----------------------------------------------------------------------*/
void
PmT2FastServices::_BuildOutput (int index)
/*-----------------------------------------------------------------------*/
{
  if (!_CountClients) return;
  if (!theT2Services()->_t2Second[index]) return;
  /* should also take into account client version !! */
  /* versoin (=2) , list<lsId,number,second,data> */
  int total, needed;
  longWord length;

  PmServicesOutput out;

  /* version */
//  if (!((longWord *) out.output = (longWord *) realloc (out.output, sizeof (longWord))))
  if (!(out.output = (char *) realloc (out.output, sizeof (longWord))))
    {
      PmLogPrint((char *)IKERROR,(char *)"Error realloc() %s",strerror(errno));
      IkFatalSend("Error realloc() %s\n",strerror(errno));
      exit (errno);
    }
  *((longWord *) out.output) = htonl (2);	/* version number */
  length = sizeof (longWord);
  /* second */
//  if (!((longWord *) out.output = (longWord *) realloc (out.output, 2 * sizeof (longWord))))
  if (!( out.output = (char *) realloc (out.output, 2 * sizeof (longWord))))
    {
      PmLogPrint((char *)IKERROR,(char *)"Error realloc() %s",strerror(errno));
      IkFatalSend("Error realloc() %s\n",strerror(errno));
      exit (errno);
    }
  *((longWord *) (out.output + sizeof (longWord))) = htonl (theT2Services()->_t2Second[index]);	/* second */
  length += sizeof (longWord);

  /* build message. */
  for (unsigned int i = 0; i < theT2Services()->_t2Buffer[index].size (); i++)
    {
      total = needed = (theT2Services()->_t2Buffer[index][i]).number * sizeof (T2Ls);
      if (needed & 0x1) total++;		/* padding */
      if (needed & 0x10) total += 2;		/* padding */
      if (!(out.output = (char *) realloc (out.output, length + total +
					   2 * sizeof (short) +
					   sizeof (longWord))))
	{
	  IkSevereSend ("Forward T2: enable to allocate space for %d t2 %X",
			(theT2Services()->_t2Buffer[index][i]).number,
			(theT2Services()->_t2Buffer[index][i]).id);
	  PmLogPrint ((char *)IKERROR,(char *)"Forward T2: enable to allocate space for %d t2 %X",
		      (theT2Services()->_t2Buffer[index][i]).number, (theT2Services()->_t2Buffer[index][i]).id);
	  break;
	}
      *((short *) (out.output + length)) = htons ((theT2Services()->_t2Buffer[index][i]).id);
      length += sizeof (short);
      *((short *) (out.output + length)) = htons ((theT2Services()->_t2Buffer[index][i]).number);
      length += sizeof (short);
      *((longWord *) (out.output + length)) = htonl ((theT2Services()->_t2Buffer[index][i]).second);
      length += sizeof (longWord);
      memcpy (out.output + length, (char *) (theT2Services()->_t2Buffer[index][i]).data, needed);
      length += total;		/* padding */
    }
  theT2FastServices()->lengthOutput = length;
  out.length = length;

  theT2FastServices()->Outputs.push_back (out);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmDOWNLOADServices                                              */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmDOWNLOADServices::GetInput (char *data)
/* DESCRIPTION:
 *   Processes data input and builds a raw data stream
 *
 * PARAMS:
 *   data = raw data from CDAS client
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  if (_firstMess == PmYES)
    _firstMess = PmNO;

  // Constructing PmDowndload message stucture from the input raw data
  PmDownloadMessage tmp (data);
  if (!(tmp.data))
    {
      IkSevereSend ("Impossible to allocate space for new PmDownloadMessage !");
      PmLogPrint ((char *)IKERROR,(char *)"GetInput: Impossible to allocate space for new PmDownloadMessage !");
      return;
    }

  // Managing the current PmDownload message to build a stream data to be sent
  _BuildOutput (tmp);
  free (tmp.data);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmDOWNLOADServices::_BuildOutput (PmDownloadMessage & mess)
/* DESCRIPTION:
 *   Builds a raw data stream and push in a vector storage
 *
 * PARAMS:
 *   PmDownloadMessage
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  char* mode = NULL;

  if ( !(mess.destNumber) ) {// Broadcast
    assert ( mode = new char[ strlen (mBROADCAST) + 1 ] ); // RR +
    strcpy (mode, mBROADCAST);
  } else {
    assert ( mode = new char[ strlen (mLIST) + 1 ] ); // RR +
    strcpy (mode, mLIST);
  }

	// Count the length of the CS frame
  int len_cs_frame = 0;
  if (strcmp (mode, mLIST) == 0) // List mode
    len_cs_frame += mess.addresses.size () * 2;
  len_cs_frame += (CSFRAMEHEADERLENGTH - 1) + LSRAWMESSAGEHEADERLENGTH + mess.dataLength;
	
  // RR - int len_cs_frame = _CountLengthOfCsFrame (mess, mess.addresses, mode);
  if (len_cs_frame == -10) // RR ? what do I suppose to understand
    {
      IkSevereSend ("Can't count CsFrame length for broadcast");
      PmLogPrint ((char *)IKERROR,(char *)"Can't count CsFrame length for broadcast");
      delete[]mode;
      return;
    }

  CsFrame csFrame;
  LsRawMessage rawMess;

  _FillCsFrameHeader (csFrame, len_cs_frame, mode, mess.addresses);

  int len_ls_raw_mess = (LSRAWMESSAGEHEADERLENGTH - 1) + mess.dataLength; //_CountLengthOfLsRawMess (mess);
  
  // Filling LsRaw Message header
  rawMess.size = (unsigned char) len_ls_raw_mess;
  rawMess.type = (unsigned char) M_DOWNLOAD;
  rawMess.completion = (unsigned char) (COMPLETION_ALL);
  rawMess.slice = (unsigned char) (_slice);
  rawMess.mess = (unsigned char) (__num_mess__);
  rawMess.version = (unsigned char) (0);

  // Filling LsRaw Message data block
  for (unsigned int i = 0; i < mess.dataLength; i++)
    rawMess.data[i] = mess.data[i];

  // Filling LsRaw Message in a CS frame structure
  _FillLsRawMessInCsFrame (rawMess, csFrame, mode);

  /* +5 because of the long version with lsx, and the field length itself */
  lengthOutputData_ = len_cs_frame + 5;
  _ReallocOutputDataAndSetVersion ();
  _FillCsFrameInOutputData (csFrame);
  _AddOutputInList ();

  delete[] mode;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmUPLOADServices.                                               */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmUPLOADServices * PmUPLOADServices::_instance = new PmUPLOADServices ();

PmUPLOADServices *
PmUPLOADServices::Instance ()
{
  return _instance;
}

PmUPLOADServices *
theUPLOADServices ()
{
  return PmUPLOADServices::Instance ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmUPLOADServices::WakeUp (IkMessage * ikm, int bsId, int sock)
/* DESCRIPTION:
 *   Initializing Upload process awakened by IkMessage
 *
 * PARAMS:
 *   IkMessage *
 *   bsId
 *   sock = the current opened socket
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  /* take into account new message of upload */
  if (_uploadParticipating[bsId].inTransfer.size ())
    {
      IkWarningSend ("Can't perform two UPLOAD at a time on BSU %d, aborting this one.", bsId);
      return;
    }
  _uploadParticipating[bsId].SetNewUpload (ikm, bsId, sock); // What kinda upload is it?
  time (&_uploadParticipating[bsId].currentTime);
  _uploadParticipating[bsId].lastTime = _uploadParticipating[bsId].currentTime;
  _BuildFilesName (bsId, ikm);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmUPLOADServices::GetInput (const LsMessReference & mess)
/* DESCRIPTION:
 *   Reads the input data and processes by writing on file
 *
 * PARAMS:
 *   LsMessReference
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  if (mess.bsId > __MAXBSUONLINE__)
    {
      IkWarningSend ("Received uploaded data form LS %d (%d) non requested!",
		     mess.lsId, mess.bsId);
      PmLogPrint ((char *)IKERROR,(char *)"Received uploaded data form LS %d (%d) non requested!",
		  mess.lsId, mess.bsId);
      return;
    }

  int ls = _uploadParticipating[mess.bsId].FindLSByLsId (mess.lsId);
  if (ls == -1)
    {
      IkWarningSend ("Received slice of file uploaded from LS %d non requested !", mess.lsId);
      PmLogPrint ((char *)IKERROR,(char *)"Received slice of file uploaded from LS %d non requested !\n", mess.lsId);
      return;
    }

  if (_CheckErrorCode (mess))
  {// Write the data received to a file...
    _upLoadFile = fopen (_uploadFileName[mess.lsId], "a");
    if (!_upLoadFile)
      {
        IkSevereSend ("Unable to open %s", _uploadFileName[mess.lsId]);
        PmLogPrint ((char *)IKERROR,(char *)"Unable to open %s", _uploadFileName[mess.lsId]);
        return;
      }

    int length = ntohl (*(longWord *) mess.rawData);
    fwrite (mess.rawData + 4, sizeof (char), length, _upLoadFile);
    fclose (_upLoadFile);
  }
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
EnumPmBoolean
PmUPLOADServices::State (int bsId)
/*-----------------------------------------------------------------------*/
{
  return (bsId > __MAXBSUONLINE__) ? PmNO : _uploadParticipating[bsId].GetStatus ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmUPLOADMessage *
PmUPLOADServices::CheckMessage (int bsId)
/* DESCRIPTION:
 *   Called during the ProcessInterrupts step by BSUConnection->Process
 *
 * PARAMS:
 *   bsId
 *
 * RETURN VALUE:
 *   PmUPLOADMessage *
 */
/*-----------------------------------------------------------------------*/
{
  if (bsId > __MAXBSUONLINE__)
    return NULL;

  time (&_uploadParticipating[bsId].currentTime);

  if ((_uploadParticipating[bsId].ReadyForNextPart ()) ||
      ((_uploadParticipating[bsId].currentTime - _uploadParticipating[bsId].lastTime) 
      > __MAXTIMETOGETUPLOAD__))
    {
      PmUPLOADMessage *output;
      if (!(output = new PmUPLOADMessage ()))
	{
	  IkSevereSend ("Cant't allocate space for new PmUPLOADMessage !!");
	  PmLogPrint ((char *)IKERROR,(char *)"Cant't allocate space for new PmUPLOADMessage !!");
	  return NULL;
	}
      if (_uploadParticipating[bsId].DiscardMissingStations () == -1)
	{
	  IkWarningSend ("Limit time expired for UPLOAD, terminating.");
	  PmLogPrint ((char *)IKINFO,(char *)"Limit time expired for UPLOAD, terminating.");
	  delete output;
	  return NULL;
	}
      *output = _uploadParticipating[bsId];
      _uploadParticipating[bsId].NextPart ();
      _uploadParticipating[bsId].lastTime = _uploadParticipating[bsId].currentTime;
      return output;
    }

  return NULL;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmUPLOADServices::_BuildFilesName (int bsId, IkMessage * ikm)
/*-----------------------------------------------------------------------*/
{
  for (unsigned int i = 0; i < _uploadParticipating[bsId].inTransfer.size (); i++)
    snprintf (_uploadFileName[_uploadParticipating[bsId].inTransfer[i]],__MAXFILENAME__,
	      "%s%s_%d", PMTRASHDIR,
	      (((IkLsLogReq *) ikm)->file_name).c_str (),
	      _uploadParticipating[bsId].inTransfer[i]);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmUPLOADServices::_CheckErrorCode (const LsMessReference & mess)
/*-----------------------------------------------------------------------*/
{
  int length = ntohl (*(longWord *) mess.rawData);

  if (length < 0 || length > __MAXLENGTHFORUPLOAD__)
    {
      IkWarningSend ("Receiving error code for UPLOAD from LS %d", mess.lsId);
      PmLogPrint ((char *)IKINFO,(char *)"Receiving error code for UPLOAD from LS %d", mess.lsId);
      _uploadParticipating[mess.bsId].RemoveLS (mess.lsId);
      return 0;
    }
  else if (length < __MAXLENGTHFORUPLOAD__)
    {
      IkInfoSend ("File %s written.", _uploadFileName[mess.lsId]);
      _uploadParticipating[mess.bsId].RemoveLS (mess.lsId);
    }
  else if (length == __MAXLENGTHFORUPLOAD__)
    {
      _uploadParticipating[mess.bsId].SetStatus (mess.lsId);
    }

  return 1;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmEVENTServices                                                 */
/*-----------------------------------------------------------------------*/
PmEVENTServices * PmEVENTServices::_instance = new PmEVENTServices ();

PmEVENTServices *
PmEVENTServices::Instance ()
{
  return _instance;
}

PmEVENTServices *
theEVENTServices ()
{
  return PmEVENTServices::Instance ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmCALIBServices                                                 */
/*-----------------------------------------------------------------------*/
PmCALIBServices * PmCALIBServices::_instance = new PmCALIBServices ();

PmCALIBServices *
PmCALIBServices::Instance ()
{
  return _instance;
}

PmCALIBServices *
theCALIBServices ()
{
  return PmCALIBServices::Instance ();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmMoSdRawServices                                               */
/*-----------------------------------------------------------------------*/
PmMoSdRawServices * PmMoSdRawServices::_instance = new PmMoSdRawServices ();

PmMoSdRawServices *
PmMoSdRawServices::Instance ()
{
  return _instance;
}

PmMoSdRawServices *
theMoSdRawServices ()
{
  return PmMoSdRawServices::Instance ();
}
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* class PmSpmtSdServices                                                */
/*-----------------------------------------------------------------------*/
PmSpmtSdServices * PmSpmtSdServices::_instance = new PmSpmtSdServices ();

PmSpmtSdServices *PmSpmtSdServices::Instance ()
{
  return _instance;
}

PmSpmtSdServices *
theSpmtSdServices ()
{
  return PmSpmtSdServices::Instance ();
}

/*-----------------------------------------------------------------------*/
