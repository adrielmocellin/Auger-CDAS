
#include "PmDefs.h"
#include "PmLog.h"
#include "PmLSX.h"

/*-----------------------------------------------------------------------*/ 
/* class LsMessReference                                                 */
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
LsMessReference::LsMessReference ()
/*-----------------------------------------------------------------------*/ 
{
  lsId = type = version = length = slice = messNumber = 0;
  bsId = -1;
  rawData = NULL;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
LsMessReference::LsMessReference (int& ls_id, int bs_id, const LsRawMessage& mess)
/*-----------------------------------------------------------------------*/ 
{
  lsId = ls_id;
  bsId = bs_id;
  slice = (unsigned short) (mess.slice);
  messNumber = (unsigned short) (mess.mess);
#if __BYTE_ORDER == __LITTLE_ENDIAN
  completion = ((mess.completion) << 6);
#else
  completion = ((mess.completion) >> 6);
#endif
  version = (unsigned int)(mess.version);
  length = (unsigned int) (mess.size - (LSRAWMESSAGEHEADERLENGTH-1));
  type = (unsigned int) mess.type;
  rawData = NULL;
  if ( length > MAX_DATA_LENGTH ) // RR
    return;
  if ( !(rawData = (char*) malloc (length)) )
    cerr << "impossible de faire le malloc..." << endl;
  memcpy (rawData, mess.data, length);
}
/*-----------------------------------------------------------------------*/ 

/* WARNING */
/* These two following canonical functions are used by the standard library of vector,
   which does a lot of copy during the insert() or push_back() operations, so we need 
   here to ONLY copy pointers, and especially we MUST NOT copy data. 
*/

/*-----------------------------------------------------------------------*/ 
LsMessReference::LsMessReference (const LsMessReference& mess)
/*-----------------------------------------------------------------------*/ 
{
  lsId = mess.lsId;
  bsId = mess.bsId;
  type = mess.type;
  version = mess.version;
  length = mess.length;
  slice = mess.slice;
  messNumber = mess.messNumber;
  completion = mess.completion;
  /* don't malloc here ! */
  rawData = mess.rawData;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
LsMessReference& LsMessReference::operator = (const LsMessReference& mess)
/*-----------------------------------------------------------------------*/ 
{
  lsId = mess.lsId;
  bsId = mess.bsId;
  type = mess.type;
  version = mess.version;
  length = mess.length;
  slice = mess.slice;
  messNumber = mess.messNumber;
  completion = mess.completion;
  /* don't malloc here ! */
  rawData = mess.rawData;
  return *this;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* class PmMessReference                                                 */
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmMessReference::PmMessReference (IkMessage* ikm, char* m, int ikType)
/* DESCRIPTION:
 *   PmMessage built from IkMessage structure received from CDAS clients
 *
 * PARAMS:
 *   IkMessage*
 *   m = mode
 *   ikType
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/ 
{
  slice = 0;
  pointerData = 0;
  length = 0;
  int s = strlen (m);
  mode = (char*) malloc (s+1);
  strncpy (mode, m, s+1);
  rawData = NULL; // !! each message doesn't data !!

  for (unsigned int i=0; i<((IkPmMessage*)ikm)->addresses.size(); i++)
    ls.push_back (((IkPmMessage*)ikm)->addresses[i]);

  priority = pCONTROL; // RR + default priority

  switch (ikType)
    {
    case IKT3:
      {
	priority = pEVTSVR; // RR +
	type = M_T3_YES;
	length += LSRAWMESSAGEHEADERLENGTH + 10 + 2*((IkT3*)ikm)->addresses.size();
	rawData = (char*) malloc (length-LSRAWMESSAGEHEADERLENGTH);
	longWord* lP; word* sP; unsigned char* cP;
	sP = (word*)rawData;
	/* event identifier */
	*sP++ = htons((short)((IkT3*)ikm)->id);
	/* ref second */
	lP = (longWord*) sP; 
	*lP++ = htonl((longWord)((IkT3*)ikm)->refSecond); 
	/* ref usecond */
	*lP++ = htonl((longWord)((IkT3*)ikm)->refuSecond);
	/* offset and window for each ls */
	cP = (unsigned char*) lP;
	for (unsigned int i=0;i<((IkT3*)ikm)->addresses.size();i++)
	  {
	    *cP++ = (unsigned char)((IkT3*)ikm)->offsets[i]; 
	    *cP++ = (unsigned char)((IkT3*)ikm)->window[i];
	  }	
	break;
      }
    case IKLSOS9:
      {
	type = M_OS9_CMD;
	length += LSRAWMESSAGEHEADERLENGTH + 1 + strlen ((((IkLsOS9*)ikm)->command).c_str());
	rawData = (char*) malloc (length-LSRAWMESSAGEHEADERLENGTH);
	IkLsOS9* os9 = (IkLsOS9*)ikm;
	char* cP = (char*)rawData;
	int size = strlen ((os9->command).c_str());
	*cP++ = (char) (size);
	memcpy (cP, (os9->command).c_str(), size);
	break;
      }
    case IKLSCHANGESTATE:
      type = M_RUN_ENABLE;
      length = LSRAWMESSAGEHEADERLENGTH + 1;
      rawData = (char*) malloc (length-LSRAWMESSAGEHEADERLENGTH);
      *((unsigned char*)rawData) = (unsigned char) ((IkLsChangeState*)ikm)->RunEnable;
      break;
    case IKLSCONFIG:
      type = M_CONFIG_SET;
      // to do
      break;
    case IKLSDOWNLOADCHECK:
      {
	type = M_DOWNLOAD_CHECK;
	length += LSRAWMESSAGEHEADERLENGTH + 3*sizeof (word);
	rawData = (char*) malloc (length-LSRAWMESSAGEHEADERLENGTH);
	IkLsDownloadCheck* check = (IkLsDownloadCheck*)ikm;
	word* sP = (word*) (rawData);
	*sP++ = htons ((short)(check->DownloadId));
	*sP++ = htons ((short)(check->CheckId));
	*sP = htons ((short)(check->SliceNumbers));
	break;
      }
    case IKLSGPSSET:
      priority = pGPS; // RR +
      type = M_GPS;
      // to be defined
      break;
    case IKLSMONREQ:
      priority = pMONIT; // RR +
      type = M_MONIT_REQ;
      length += LSRAWMESSAGEHEADERLENGTH + 4;
      rawData = (char*) malloc (length-LSRAWMESSAGEHEADERLENGTH);
      *((longWord*)rawData) = htonl ((longWord)(((IkLsMonReq*)ikm)->Identifier));
      break;
//     case IKLSPARAM:
//       type = M_SET_PARAM;
//      break;
    case IKLSLOADCONF:
      type = M_FLASH_TO_CONFIG;
      length += LSRAWMESSAGEHEADERLENGTH;
      break;
    case IKLSSAVECONF:
      type = M_CONFIG_TO_FLASH;
      length += LSRAWMESSAGEHEADERLENGTH;
      break;
    case IKLSCONT:
      type = M_RUN_CONTINUE_REQ;
      length += LSRAWMESSAGEHEADERLENGTH;
      break;
    case IKLSREBOOT:
      type = M_REBOOT;
      length += LSRAWMESSAGEHEADERLENGTH;
      break;
    case IKLSSTART:
      type = M_RUN_START_REQ;
      length += LSRAWMESSAGEHEADERLENGTH;
      break;
    case IKLSSTOP:
      type = M_RUN_STOP_REQ;
      length += LSRAWMESSAGEHEADERLENGTH;
      break;
    case IKLSWAKEUP:
      type = M_WAKEUP;
      length += LSRAWMESSAGEHEADERLENGTH;
      break;
    case IKLSCALREQ:
      priority = pMONIT; // RR +
      type = M_CALIB_REQ;
      length += LSRAWMESSAGEHEADERLENGTH;
      break;
    case IKLSPAUSE:
      type = M_RUN_PAUSE_REQ;
      length += LSRAWMESSAGEHEADERLENGTH;
      break;
    default:
      priority = pLAST; // RR +
      IkWarningSend ("Got message to send in unknown stream.");
    }
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmMessReference::PmMessReference (const PmUPLOADMessage& mess)
/* DESCRIPTION:
 *   PmMessage built from PmUPLOADMessage
 *
 * PARAMS:
 *   PmUPLOADMessage
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/ 
{
  slice = 0;
  pointerData = 0;
  length = 0;
  int s = strlen (mess.mode);
  mode = (char*) malloc (s+1);
  strncpy (mode, mess.mode, s+1);

  for (unsigned int i=0; i<mess.ls.size(); i++)
    ls.push_back (mess.ls[i]);

  type = M_LOG_REQ;
  priority = pCONTROL;
  int n = strlen (mess.filename);
  char* toto = new char[n+1];
  strncpy (toto, mess.filename, n);
  toto [n] = '\0';
  length = LSRAWMESSAGEHEADERLENGTH + 2*sizeof(longWord) + n + 1;
  rawData = (char*) malloc (length-LSRAWMESSAGEHEADERLENGTH);
  longWord* lP = (longWord*) rawData;
  *lP++ = htonl (mess.initPos);
  *lP++ = htonl (mess.length);
  byte* cP = (byte*) lP;
  memcpy (cP, toto, n+1);
  delete[] toto;  

}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmMessReference::PmMessReference (const PmMessReference& mess)
/*-----------------------------------------------------------------------*/ 
{
  completion = mess.completion;
  priority = mess.priority;
  slice = mess.slice;
  messNumber = mess.messNumber;
  pointerData = mess.pointerData;
  type = mess.type;
  mode = mess.mode;
  length = mess.length;
  for (unsigned int i=0; i<mess.ls.size(); i++)
    ls.push_back (mess.ls[i]);
  /* don't malloc here ! */
  rawData = mess.rawData;  
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmMessReference& PmMessReference::operator = (const PmMessReference& mess)
/*-----------------------------------------------------------------------*/ 
{
  completion = mess.completion;
  priority = mess.priority;
  slice = mess.slice;
  messNumber = mess.messNumber;
  pointerData = mess.pointerData;
  type = mess.type;
  mode = mess.mode;
  length = mess.length;
  for (unsigned int i=0; i<mess.ls.size(); i++)
    ls.push_back (mess.ls[i]);
  /* don't malloc here ! */
  rawData = mess.rawData;  
  return *this;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* class PmUPLOADMessage                                                 */
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmUPLOADMessage::PmUPLOADMessage (const PmUPLOADMessage& mess)
/*-----------------------------------------------------------------------*/ 
{
  for (unsigned int i=0; i<mess.ls.size (); i++)
    ls.push_back (mess.ls[i]);
  strcpy (filename, mess.filename);
  strcpy (mode, mess.mode);
  initPos = mess.initPos;
  length = mess.length;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmUPLOADMessage& PmUPLOADMessage::operator = (const PmUPLOADMessage& mess)
/*-----------------------------------------------------------------------*/ 
{
  for (unsigned int i=0; i<mess.ls.size (); i++)
    ls.push_back (mess.ls[i]);
  strcpy (filename, mess.filename);
  strcpy (mode, mess.mode);
  initPos = mess.initPos;
  length = mess.length;
  return *this;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/
EnumPmBoolean PmUPLOADMessage::GetStatus ()
/*-----------------------------------------------------------------------*/
{
  return _status;
}
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/ 
void PmUPLOADMessage::SetNewUpload (IkMessage *ikm, int bsId, int sock)
/*-----------------------------------------------------------------------*/ 
{
  _status = PmYES;
  for (unsigned int i=0; i<((IkPmMessage*)ikm)->addresses.size(); i++)
    {
      inTransfer.push_back (((IkPmMessage*)ikm)->addresses[i]);
      status.push_back (1);
      ls.push_back (((IkPmMessage*)ikm)->addresses[i]);
    }

  strncpy (filename, (((IkLsLogReq*)ikm)->file_name).c_str(), 50);
  initPos = ((IkLsLogReq*)ikm)->position;
  length = 1024;
  if ( !(ls.size()) )
    {
      strncpy (mode, mBROADCAST, 10);
      vector<int> tmp = theStationDataBase ()-> GetStationsInBSU (bsId, sock);
      for (unsigned int i=0; i<tmp.size(); i++)
	{
	  inTransfer.push_back (tmp[i]);
	  status.push_back (1);
	}
    }
  else
    strncpy (mode, mLIST, 10);
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
int PmUPLOADMessage::FindLSByLsId (int id)
/*-----------------------------------------------------------------------*/ 
{
  for (unsigned int i=0; i<inTransfer.size (); i++)
    if (inTransfer[i]==id)
      return i;

  return -1;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void PmUPLOADMessage::RemoveLS (int id)
/*-----------------------------------------------------------------------*/ 
{
  for (unsigned int i=0; i<ls.size (); i++)
    {
      if (ls[i] == id) ls.erase (ls.begin()+i);
    }
  for (unsigned int i=0; i<inTransfer.size (); i++)
    {
      if (inTransfer[i] == id)
	{
	  inTransfer.erase (inTransfer.begin()+i); 
	  status.erase (status.begin()+i);
	}
    }
  /* test to know if current upload is ended */
  if ( !(inTransfer.size ()) )
    _status = PmNO;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void PmUPLOADMessage::SetStatus (int id)
/*-----------------------------------------------------------------------*/ 
{
  for (unsigned int i=0; i<inTransfer.size (); i++)
    {
      if (inTransfer[i] == id) status [i] = 1;
    }  
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
int PmUPLOADMessage::DiscardMissingStations ()
/*-----------------------------------------------------------------------*/ 
{
  for (unsigned int i=0; i<status.size (); i++)
    {
      if (status [i] == 0) RemoveLS (inTransfer [i]);
    }
  if ( !(status.size ()) )
    {
      _status = PmNO;
      return -1;
    }
  
  return 0;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void PmUPLOADMessage::NextPart ()
/*-----------------------------------------------------------------------*/ 
{
  /* reset status of each LS */
  for (unsigned int i=0; i<status.size (); i++)
    status [i] = 0;

  initPos += 1024;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
int PmUPLOADMessage::ReadyForNextPart ()
/*-----------------------------------------------------------------------*/ 
{
  /* check status of each LS */
  for (unsigned int i=0; i<status.size (); i++)
    {
      if (status[i] == 0) return 0;
    }
  return 1;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* class PmDownloadMessage                                               */
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmDownloadMessage::PmDownloadMessage (char* input)
{
	data = NULL;
	longWord * lP = (longWord*) input;
	destNumber = ntohl (*lP++);
	
	for (unsigned int i=0; i<destNumber; i++)
		addresses.push_back (ntohl(*lP++));
	
	dataLength = ntohl (*lP++);
	if ( !(data = (char*) malloc (dataLength)) ) return;
	memcpy (data, lP, dataLength);
}
/*-----------------------------------------------------------------------*/ 


/*-----------------------------------------------------------------------*/ 
/* class BaseSubscriver                                                  */
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
BaseSubscriber *BaseSubscriber::_instance = new BaseSubscriber ();
/*-----------------------------------------------------------------------*/ 
BaseSubscriber *BaseSubscriber::Access ()
/*-----------------------------------------------------------------------*/ 
{
  return _instance;
}
/*-----------------------------------------------------------------------*/ 
BaseSubscriber *theStationDataBase ()
/*-----------------------------------------------------------------------*/ 
{
  return BaseSubscriber::Access ();
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void BaseSubscriber::AddBSu (int bs_id, int sock)
/*-----------------------------------------------------------------------*/ 
{
  BaseSubscriberUnity tmp (bs_id, sock);
  list_BSu.push_back (tmp);
  PmLogPrint ((char *)IKINFO,(char *)"AddBsu : bs id = %d, socket = %d", bs_id, sock);
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void BaseSubscriber::RemoveBSu (int bs_id, int sock)
/*-----------------------------------------------------------------------*/ 
{
  unsigned int i;
  for (i=0;i<list_BSu.size();i++)
    {
      if ( list_BSu[i].bsId == bs_id && list_BSu[i].socket == sock )
	{
	  for (unsigned int j=0; j<list_BSu[i].list_ls.size (); j++)
	    {
	      local_station [list_BSu[i].list_ls[j]].SetBsId (-1);
	      local_station [list_BSu[i].list_ls[j]].SetSocket (-1);
	      //PmLogPrint(IKINFO,"   -- Removing station %d of run with BSU Id = %d ", list_BSu[i].list_ls[j],bs_id);
	    }
	  list_BSu.erase (list_BSu.begin()+i);	  
	  break;  // RR - don't need to find another BSU
	}
    }
  if ( i == list_BSu.size() )
    PmLogPrint ((char *)IKINFO,(char *)"RemoveBSu failed ** id = %d, socket = %d", bs_id, sock);
  else
    PmLogPrint ((char *)IKINFO,(char *)"RemoveBSu id = %d, socket = %d", bs_id, sock);
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
int BaseSubscriber::FindBSuFromId (int bs_id, int sock)
/*-----------------------------------------------------------------------*/ 
{
  for (unsigned int i=0;i<list_BSu.size();i++)
    {
      if ( list_BSu[i].bsId == bs_id && list_BSu[i].socket == sock )
	return i;
    }
  return -1;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
cdasErrors BaseSubscriber::AddLsInBSU (int ls_id,int bs_id, int sock)
/*-----------------------------------------------------------------------*/ 
{
  int bsu = FindBSuFromId (bs_id, sock);
  if ( bsu == -1 )
    return CDAS_INVALID;
  list_BSu[bsu].list_ls.push_back (ls_id);
  return CDAS_SUCCESS;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void BaseSubscriber::RemoveLsInBSU (int ls_id,int bs_id, int sock)
/*-----------------------------------------------------------------------*/ 
{
  int bsu = FindBSuFromId (bs_id, sock);
  if ( bsu == -1 ) return;
  
  for (unsigned int j=0;j<list_BSu[bsu].list_ls.size();j++)
    {
      if ( list_BSu[bsu].list_ls[j] == ls_id ) {
				list_BSu[bsu].list_ls.erase (list_BSu[bsu].list_ls.begin ()+j);
				break; // RR - don't need to find another LS!
			}
    }
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
int BaseSubscriber::LinkLsWithBSu (int ls_id, int bs_id, int sock)
/*-----------------------------------------------------------------------*/ 
{
  if ( ls_id > __MAXLSID__ )
    return -1;
  
  int bsu = local_station [ls_id].GetBsId ();
  int check_sock = local_station [ls_id].GetSocket ();
  if ( bsu != bs_id || check_sock != sock )
    {
      int bs_id_in_list = FindBSuFromId (bs_id, sock);
       //      PmLogPrint (IKINFO,"++ Linking new LS[%d] with the BSU[%d] - socket=%d)", ls_id, bs_id, sock);
      if ( bs_id_in_list == -1 ) return -1; // this BSu (bs_id) isn't connected !!
      fprintf(stdout,"   ++ New station in run, CPU id = %d attached with BSU id = %d\n", ls_id, bs_id);
      // reconnect ls with other BSu
      local_station [ls_id].SetBsId (bs_id);
      local_station [ls_id].SetSocket (sock);
      RemoveLsInBSU (ls_id, bsu, check_sock);
      AddLsInBSU (ls_id, bs_id, sock);
    }
  
  return 0;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
vector<int> BaseSubscriber::GetListOfSockets ()
/*-----------------------------------------------------------------------*/ 
{
  vector<int> listOfSockets;
  
  for (unsigned int i=0; i<list_BSu.size();i++)
    listOfSockets.push_back (list_BSu[i].socket);
  
  return listOfSockets;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
int BaseSubscriber::GetSocketOfBsByLsId (int id)
/*-----------------------------------------------------------------------*/ 
{
  if ( id > __MAXLSID__ )
    return -1;
  
  return local_station [id].GetSocket ();
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
vector<int> BaseSubscriber::GetStationsInBSU (int bs_id, int sock)
/*-----------------------------------------------------------------------*/ 
{
  int bsu = FindBSuFromId (bs_id, sock);
  if ( bsu == -1 )
    {
      IkSevereSend ("Problem with the upload, don't find the BSU(%d)...",bs_id);
      PmLogPrint ((char *)IKERROR,(char *)"Problem with the upload, don't find the BSU(%d)...",bs_id);
      vector<int> toto; return toto;
    }
    
  return list_BSu[bsu].list_ls;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
IkMessage* BaseSubscriber::ReBuildIkMessageWithListOfLsInThisBSu (IkMessage* ikm, int& bs_id, int sock)
/*-----------------------------------------------------------------------*/ 
{
#define HEADBLOCK_INSTR(IkGeneric) \
	for (unsigned int i=0;i<list_BSu[bsu].list_ls.size();i++)\
	  {\
	    for (unsigned int j=0;j<((IkGeneric*)ikm)->addresses.size();j++)\
	      {\
		if ( list_BSu[bsu].list_ls[i] == (((IkGeneric*)ikm)->addresses[j] ) )

  int bsu = FindBSuFromId (bs_id, sock);
  if ( bsu == -1 )
    return NULL;
  
#if PMDEBUG
  PmLogPrint((char *)IKDEBUG,(char *)"*  Construction d'un message ik : bsu = %d, bsId = %d, sock = %d\n", bsu, bs_id, sock);
#endif

  IkMessage* ret = NULL;

  switch ( ikm->get_type() )
    {
    case IKT3:
      {
	/* For test purposes */
	if ( strcmp (((IkT3*)ikm)->algo.c_str(), "IGNORE") == 0 ) // RR ?
	  return NULL;
	//	    ret = new IkT3 ();
	ret=(IkT3*)new IkT3 ();
		  HEADBLOCK_INSTR(IkT3)
		  {
		    ((IkT3*)ret) -> addresses.push_back (((IkT3*)ikm)->addresses[j]);
		    ((IkT3*)ret) -> offsets.push_back (((IkT3*)ikm)->offsets[j]);
		    ((IkT3*)ret) -> window.push_back (((IkT3*)ikm)->window[j]);
		  }
	      }
	  }
	
	if ( ((IkT3*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	//      ret -> mode = ikm.mode;
	((IkT3*)ret) -> id = ((IkT3*)ikm)->id;
	((IkT3*)ret) -> refSecond = ((IkT3*)ikm)->refSecond;
	((IkT3*)ret) -> refuSecond = ((IkT3*)ikm)->refuSecond;
	
	return ret;
      }
    case IKLSWAKEUP:
      {
	ret = (IkLsWakeUp*)new IkLsWakeUp ();
		  HEADBLOCK_INSTR(IkLsWakeUp)
		  ((IkLsWakeUp*)ret) -> addresses.push_back (((IkLsWakeUp*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsWakeUp*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	return ret;
      }
    case IKLSREBOOT:
      {
	ret = (IkLsReboot*)new IkLsReboot ();
		  HEADBLOCK_INSTR(IkLsReboot)
		  ((IkLsReboot*)ret) -> addresses.push_back (((IkLsReboot*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsReboot*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	return ret;
      }
    case IKLSLOGREQ:
      {
	ret = (IkLsLogReq*)new IkLsLogReq ();
		  HEADBLOCK_INSTR(IkLsLogReq)
		  ((IkLsLogReq*)ret) -> addresses.push_back (((IkLsLogReq*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsLogReq*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	((IkLsLogReq*)ret) -> position = ((IkLsLogReq*)ikm)->position;
	((IkLsLogReq*)ret) -> file_name = ((IkLsLogReq*)ikm)->file_name;
	
	return ret;
      }
    case IKLSDOWNLOADCHECK:
      {
	ret = (IkLsDownloadCheck*)new IkLsDownloadCheck ();
		  HEADBLOCK_INSTR(IkLsDownloadCheck)
		  ((IkLsDownloadCheck*)ret) -> addresses.push_back (((IkLsDownloadCheck*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsDownloadCheck*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	((IkLsDownloadCheck*)ret) -> DownloadId = ((IkLsDownloadCheck*)ikm)->DownloadId;
	((IkLsDownloadCheck*)ret) -> CheckId = ((IkLsDownloadCheck*)ikm)->CheckId;
	((IkLsDownloadCheck*)ret) -> SliceNumbers = ((IkLsDownloadCheck*)ikm)->SliceNumbers;
	
	return ret;
      }
    case IKLSSTART:
      {
	ret = (IkLsStart*)new IkLsStart ();
		  HEADBLOCK_INSTR(IkLsStart)
		  ((IkLsStart*)ret) -> addresses.push_back (((IkLsStart*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsStart*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	return ret;	    
      }
    case IKLSSTOP:
      {	    
	ret = (IkLsStop*)new IkLsStop ();
		  HEADBLOCK_INSTR(IkLsStop)
		  ((IkLsStop*)ret) -> addresses.push_back (((IkLsStop*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsStop*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	return ret;	    
      }
    case IKLSPAUSE:
      {	    
	ret = (IkLsPause*)new IkLsPause ();
		  HEADBLOCK_INSTR(IkLsPause)
		  ((IkLsPause*)ret) -> addresses.push_back (((IkLsPause*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsPause*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	return ret;	    
      }
    case IKLSCONT:
      {	    
	ret = (IkLsCont*)new IkLsCont ();
		  HEADBLOCK_INSTR(IkLsCont)
		  ((IkLsCont*)ret) -> addresses.push_back (((IkLsCont*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsCont*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	return ret;	    
      }
    case IKLSLOADCONF:
      {	    
	ret = (IkLsLoadConf*)new IkLsLoadConf ();
		  HEADBLOCK_INSTR(IkLsLoadConf)
		  ((IkLsLoadConf*)ret) -> addresses.push_back (((IkLsLoadConf*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsLoadConf*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	return ret;	    
      }
    case IKLSSAVECONF:
      {	    
	ret = (IkLsSaveConf*)new IkLsSaveConf ();
		  HEADBLOCK_INSTR(IkLsSaveConf)
		  ((IkLsSaveConf*)ret) -> addresses.push_back (((IkLsSaveConf*)ikm)->addresses[j]);
		  }
	  }
	
	if ( ((IkLsSaveConf*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	return ret;	    
      }
    case IKLSCHANGESTATE:
      {	    
	ret = (IkLsChangeState*)new IkLsChangeState ();
		  HEADBLOCK_INSTR(IkLsChangeState)
		  ((IkLsChangeState*)ret) -> addresses.push_back (((IkLsChangeState*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsChangeState*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }

	((IkLsChangeState*)ret) -> RunEnable = ((IkLsChangeState*)ikm)->RunEnable;
		      
	return ret;	    
      }
    case IKLSPARAM:
      {	    
	ret = (IkLsParam*)new IkLsParam ();
		  HEADBLOCK_INSTR(IkLsParam)
		  {
		    ((IkLsParam*)ret) -> addresses.push_back (((IkLsParam*)ikm)->addresses[j]);
		    ((IkLsParam*)ret) -> Offset.push_back (((IkLsParam*)ikm)->Offset[j]);
		    ((IkLsParam*)ret) -> Value.push_back (((IkLsParam*)ikm)->Value[j]);
		  }
	      }
	  }
	
	if ( ((IkLsParam*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	((IkLsParam*)ret) -> ConfigVersion = ((IkLsParam*)ikm)->ConfigVersion;
	((IkLsParam*)ret) -> ConfigFormat = ((IkLsParam*)ikm)->ConfigFormat;
	
	return ret;	    
      }
    case IKLSMONREQ:
      {	    
	ret = (IkLsMonReq*)new IkLsMonReq ();
		  HEADBLOCK_INSTR(IkLsMonReq)
		  ((IkLsMonReq*)ret) -> addresses.push_back (((IkLsMonReq*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsMonReq*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	((IkLsMonReq*)ret) -> Identifier = ((IkLsMonReq*)ikm)->Identifier;
	
	return ret;	    
      }
    case IKLSCALREQ:
      {	    
	ret = (IkLsCalReq*)new IkLsCalReq ();
		  HEADBLOCK_INSTR(IkLsCalReq)
		  ((IkLsCalReq*)ret) -> addresses.push_back (((IkLsCalReq*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsCalReq*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	((IkLsCalReq*)ret) -> Identifier = ((IkLsCalReq*)ikm)->Identifier;
	
	return ret;	    
      }
    case IKLSGPSSET:
      {	    
	ret = (IkLsGpsSet*)new IkLsGpsSet ();
		  HEADBLOCK_INSTR(IkLsGpsSet)
		  {
		    ((IkLsGpsSet*)ret) -> addresses.push_back (((IkLsGpsSet*)ikm)->addresses[j]);
		    // to be defined !!!!!!!!
		  }
	      }
	  }
	
	if ( ((IkLsGpsSet*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	// to be defined !!!!!!!!
	
	return ret;	    
      }
    case IKLSCONFIG:
      {	    
	ret = (IkLsConfig*)new IkLsConfig ();
		  HEADBLOCK_INSTR(IkLsConfig)
		  {
		    ((IkLsConfig*)ret) -> addresses.push_back (((IkLsConfig*)ikm)->addresses[j]);
		    ((IkLsConfig*)ret) -> Config.push_back (((IkLsConfig*)ikm)->Config[j]);
		  }
	      }
	  }
	
	if ( ((IkLsConfig*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	((IkLsConfig*)ret) -> ConfigVersion = ((IkLsConfig*)ikm)->ConfigVersion;
	((IkLsConfig*)ret) -> ConfigFormat = ((IkLsConfig*)ikm)->ConfigFormat;
	((IkLsConfig*)ret) -> ConfigPart = ((IkLsConfig*)ikm)->ConfigPart;
	
	return ret;	    
      }
    case IKLSOS9:
      {
	ret = (IkLsOS9*)new IkLsOS9 ();
		  HEADBLOCK_INSTR(IkLsOS9)
		  ((IkLsOS9*)ret) -> addresses.push_back (((IkLsOS9*)ikm)->addresses[j]);
	      }
	  }
	
	if ( ((IkLsOS9*)ret) -> addresses.size() == 0 )
	  {
	    delete ret;
	    return NULL;
	  }
	
	((IkLsOS9*)ret) -> command = ((IkLsOS9*)ikm) -> command;
	
	return ret;
      }
    }
  
  return ret;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
int BaseSubscriber::IsRequestedByTrashService (int lsId)
/*-----------------------------------------------------------------------*/ 
{
  return (lsId > __MAXLSID__) ? -1 : local_station [lsId].IsRequestedByTrashService ();
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void BaseSubscriber::RequestedByTrashService (int lsId, int state)
/*-----------------------------------------------------------------------*/ 
{
  if (lsId <= __MAXLSID__)
     local_station [lsId].RequestedByTrashService (state);
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* class _PmLsMessage                                                    */
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
_PmLsMessage::_PmLsMessage (const LsMessReference& mess)
/*-----------------------------------------------------------------------*/ 
{
  length = mess.length;
  type = mess.type;
  version = mess.version;
  data = mess.rawData;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
_PmLsMessage& _PmLsMessage::operator = (const _PmLsMessage& mess)
/*-----------------------------------------------------------------------*/ 
{
  length = htonl (mess.length+12);
  type = htonl (mess.type);
  version = htonl (mess.version);
  data = mess.data;
  return *this;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* class _PmLsRawMessage                                                 */
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
_PmLsRawMessage::_PmLsRawMessage (const LsMessReference& mess_ref)
/*-----------------------------------------------------------------------*/ 
{
  lsId = htonl (mess_ref.lsId);
  _PmLsMessage tmp (mess_ref);
  /* see the surcharge of this affectation operator */
  mess = tmp;
//   mess.length = htonl (mess_ref.length+12);
//   mess.type = htonl (mess_ref.type);
//   mess.version = htonl (mess_ref.version);
//   mess.data = mess_ref.rawData;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* class PmServicesOutput                                                */
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmServicesOutput::PmServicesOutput ()
/*-----------------------------------------------------------------------*/ 
{
  output = NULL;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmServicesOutput::PmServicesOutput (int size, char* data)
/*-----------------------------------------------------------------------*/ 
{
  length = size;
  output = data;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmServicesOutput::PmServicesOutput (const PmServicesOutput& mess)
/*-----------------------------------------------------------------------*/ 
{
  length = mess.length;
  output = mess.output;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
PmServicesOutput& PmServicesOutput::operator = (const PmServicesOutput& mess)
/*-----------------------------------------------------------------------*/ 
{
  length = mess.length;
  output = mess.output;
  return *this;
}
/*-----------------------------------------------------------------------*/ 
