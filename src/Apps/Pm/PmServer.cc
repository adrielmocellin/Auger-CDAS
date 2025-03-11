
#include "PmServer.h"
#include "PmLog.h"

static int glob_sock_fd = -1;

/*-----------------------------------------------------------------------*/
static void
do_ignore (int code)
/*-----------------------------------------------------------------------*/
{
  code = code;
  IkWarningSend ("Ignoring SIGPIPE signal!");
  PmLogPrint ((char *)IKERROR,(char *)"Ignoring SIGPIPE signal!");
}

/*-----------------------------------------------------------------------*/
static void
do_cleanup (int code)
/*-----------------------------------------------------------------------*/
{
  if ( !thePmServer() ) exit ( code ); // RR + check the nullity

  vector < int >sockets = thePmServer ()->GetSocketsOpened ();
  for (unsigned int i = 0; i < sockets.size (); i++)
    {
      if (fsync (sockets[i]) == -1) ERROR_SWITCH_FSYNC(sockets[i])
      if (shutdown (sockets[i], 2) == -1) ERROR_SWITCH_SHUTDOWN(sockets[i])
      if ( close (sockets[i]) == -1 ) IkWarningSend ("close(2) an error occurs on socket=%d",sockets[i]);
    }

  // Free main socket...
  if ( fsync (glob_sock_fd) == -1 ) ERROR_SWITCH_FSYNC(glob_sock_fd)
  if ( shutdown (glob_sock_fd, 2) ) ERROR_SWITCH_SHUTDOWN(glob_sock_fd)
  if ( close (glob_sock_fd) == -1 ) IkWarningSend ("close(2) an error occurs on socket=%d",glob_sock_fd);

  PmLogPrint ((char *)IKFATAL,(char *)"Terminating with code %d", code);
  IkFatalSend ("Terminating with code %d", code);
  exit ( code );
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* class PmServer.                                                       */
/*-----------------------------------------------------------------------*/

/* Singleton */
PmServer * PmServer::_instance = new PmServer ();

PmServer *
PmServer::Instance ()
{
  return _instance;
}

PmServer *
thePmServer ()
{
  return PmServer::Instance ();
}

/*-----------------------------------------------------------------------*/
PmServer::PmServer ()
/*-----------------------------------------------------------------------*/
{
  /* Internal variables. */
  IkPingValue = 0;
  _num_data_client = 0;

  /* init connections */
  _pmDOWNLOADConnectionP = NULL;
  for (int i = 0; i < __MAXBSUONLINE__; i++)
    _pmBSUConnection[i] = NULL;
  for (int i = 0; i < __MAXCDAS__; i++)
    _pmCDASConnection[i] = NULL;

}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmServer::PmServer (const PmServer &)
/*-----------------------------------------------------------------------*/
{
  PmLogPrint ((char *)IKFATAL,(char *)"In the PmServer constructor copy");
  exit (1);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmServer::~PmServer ()
/*-----------------------------------------------------------------------*/
{
  PmLogPrint ((char *)IKFATAL,(char *)"In the PmServer destructor");
  exit (1);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServer::InitPort (int value)
/* DESCRIPTION:
 *   Open main socket, set socket params, bind and listen the port
 *   Trap all system signals
 *   Set main socket in the pool of select connections
 *
 * PARAMS:
 *   value = port number to be opened
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  _port = value;

  if ((_socket_fd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      IkFatalSend ("Error creating the socket: %s", strerror (errno));
      PmLogPrint ((char *)IKERROR,(char *)"InitPort: error creating the socket: %s", strerror (errno));
      PmLogPrintErrno ();
      exit (errno);
    }

  // Assigning main socket
  _max_fd = _socket_fd;
  glob_sock_fd = _socket_fd;
  PmLogPrint((char *)IKINFO,(char *)"   Main socket : %d\n", _socket_fd);

  int one = 1;
  if (setsockopt (_socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof (int)) < 0)
    {
      IkFatalSend ("Failed on setsockopt(): %s", strerror (errno));
      PmLogPrint ((char *)IKERROR,(char *)"InitPort: failed on setsockopt(): %s", strerror (errno));
      PmLogPrintErrno ();
      exit (errno);
    }

  memset (&_serv_addr, 0, sizeof (_serv_addr));
  _serv_addr.sin_addr.s_addr = INADDR_ANY;
  _serv_addr.sin_port = htons (_port);
  _serv_addr.sin_family = AF_INET;

  if (bind (_socket_fd, (struct sockaddr *) &_serv_addr, sizeof (_serv_addr)) == -1)
    {
      IkFatalSend ("Could not bind: %s", strerror (errno));
      PmLogPrint ((char *)IKERROR,(char *)"InitPort: could not bind: %s", strerror (errno));
      PmLogPrintErrno ();
      exit (errno);
    }

  if (listen (_socket_fd, __QUEUELEN__) == -1)
    {
      IkFatalSend ("Could not listen: %s", strerror (errno));
      PmLogPrint ((char *)IKERROR,(char *)"InitPort: could not listen: %s", strerror (errno));
      PmLogPrintErrno ();
      exit (errno);
    }

  FD_ZERO (&_Global_Fd);
  FD_SET (_socket_fd, &_Global_Fd);

  IkInfoSend ("Data port is 0x%04X (%d)", _port, _port);
  fprintf(stdout,"   Data port is 0x%04X (%d)\n", _port, _port);
		
  // Trap signals interruptions
  signal (SIGTERM, do_cleanup);
  signal (SIGQUIT, do_cleanup);
  signal (SIGABRT, do_cleanup);
  signal (SIGUSR1, do_cleanup);
  signal (SIGUSR2, do_cleanup);
  signal (SIGINT, do_cleanup);
  signal (SIGHUP, do_cleanup);
  signal (SIGPIPE, do_ignore);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServer::ListenPort ()
/* DESCRIPTION:
 *   Selects read event on the pool of opened connection
 *
 * PARAMS:
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday (&tv, &tz);

  struct timeval timeout;

  _Temp_Fd = _Global_Fd;
/*  if (tv.tv_usec < 650000)
    {
      timeout.tv_sec = 0;
      timeout.tv_usec = 650000 - tv.tv_usec;
    }
  else
    {
      timeout.tv_sec = 0;
      timeout.tv_usec = 1650000 - tv.tv_usec;
    }
*/
  timeout.tv_sec = 0;
  timeout.tv_usec = 1000;
  if ((_Number_Fd = select (_max_fd + 1, &_Temp_Fd, NULL, NULL, &timeout)) == -1)
    {
      IkFatalSend ("Error in select %s", strerror (errno));
      PmLogPrint ((char *)IKERROR,(char *)"ListenPort: error in select %s", strerror (errno));
      PmLogPrintErrno ();
      exit (errno);
    }
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServer::CheckNewConnection ()
/* DESCRIPTION:
 *   Checks if we are awaken by a new connection on the Pmport
 *   Accepts the new connection and read the raw data from the new socket
 *   Builds a Pm standard block structure for a specific client requested
 *   Creates a new object connection assigned to the new socket
 *   If everything goes well since 
 *     Sends an acknowledge to the new client notifying Pm's agreement
 *     Set the new socket in the pool of connection
 *   Else
 *     Sends an error acknowledge
 *
 * PARAMS:
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  unsigned int size_sock = sizeof (struct sockaddr_in);
  int new_client;
  cdasErrors error;
  struct sockaddr_in sock_in;
  PmStdConnection *pmStdConnectionP;

  // Listen the main socket of Pm for new read events
  if (FD_ISSET (_socket_fd, &_Temp_Fd))
    {
      // Clear main socket from the listening select pool...
      _Number_Fd--;
      FD_CLR (_socket_fd, &_Temp_Fd);

      // Accepting a client on a new socket
      if ((new_client = accept (_socket_fd, (struct sockaddr *) &sock_in, &size_sock)) == -1)
	{
	  IkFatalSend ("Error in accept(): %s", strerror (errno));
	  PmLogPrint ((char *)IKERROR,(char *)"CheckNewConnection: accept() error : %s", strerror (errno));
	  PmLogPrintErrno ();
	  exit (errno);
	}

      // Checking the Pm protocol used by the current connecting client
      if (!(pmStdConnectionP = _CheckProtocolOfNewConnection (new_client)))
	{
	  IkWarningSend ("Bad client tries to connect with bad protocol.");
	  PmLogPrint ((char *)IKERROR,(char *)"CheckNewConnection: Bad client tries to connect with bad protocol.");
	  close (new_client);
	  return;
	}

      // Allocating connections to the right object entity
      if ((error = _OpenConnection (*pmStdConnectionP, new_client, inet_ntoa (sock_in.sin_addr))) != CDAS_SUCCESS)
	{
	  // Something's wrong with the current connection! the BSu has been warned
	  _SendAck (new_client, error);
	  close (new_client);
	  delete pmStdConnectionP;
	  IkWarningSend ("Bad news for new client, open connection returns %d", error);
	  PmLogPrint ((char *)IKERROR,(char *)"CheckNewConnection: Bad news for new client, open connection returns %d", error);
	  return;
	}

      // Sending good acknowledge (GsACK) to the CDAS client
      if ((error = _SendAck (new_client, CDAS_SUCCESS)) != CDAS_SUCCESS)
	{
#if PMDEBUG
	  PmLogPrint((char *)IKDEBUG,(char *)"*  SendAck to new client(%d) failed!\n",new_client);
#endif
	  close (new_client);
	  _CloseConnection (new_client);
	  delete pmStdConnectionP;
	  IkWarningSend ("Error %d during sending acknowledge of connection !", error);
	  PmLogPrint ((char *)IKERROR,(char *)"CheckNewConnection: Error %d during sending acknowledge of connection !", error);
	  return;
	}
      // Updating the pool of client active connections
      else
	{
	  _max_fd = PMDEF_max_ (_max_fd, new_client);
	  _num_data_client++;
	  FD_SET (new_client, &_Global_Fd);
	  PmLogPrint ((char *)IKINFO,(char *)"++ Adding new client(%d) at %d -> maxfd=%d, number of clients=%d from %s",
		      pmStdConnectionP->pmStdHeader.type, new_client, _max_fd,
		      _num_data_client, inet_ntoa (sock_in.sin_addr));
	  IkInfoSend ("Accepting new client type %d at socket %d from %s, number of client(s)=%d",
	     pmStdConnectionP->pmStdHeader.type, new_client,
	     inet_ntoa (sock_in.sin_addr), _num_data_client);
	  delete pmStdConnectionP;
	}
    }
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServer::ListenRegisteredClients ()
/* DESCRIPTION:
 *   Scans the pool connections and checks for read event awakened
 *   Gets input data from each client alive
 *
 * PARAMS:
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  cdasErrors ret;

  for (int fd = 1; fd < (_max_fd + 1); fd++)
    {
      if (FD_ISSET (fd, &_Temp_Fd))
	{
	  // a read event on a socket has been awakened...
	  _Number_Fd--;
	  FD_CLR (fd, &_Temp_Fd);

	  // Fetch rawdata from different clients input
	  ret = _ManageClient (fd);

	  switch (ret)
	    {
	    case CDAS_CONNECT_CLOSED:
	      _RemoveClient (fd);
	      _CloseConnection (fd);
	      PmLogPrint((char *)IKINFO,(char *)"ListenRegisteredClients : closed connection with %d", fd);
	      break;

	    case CDAS_NO_SOCKET:
	      _RemoveClient (fd);
	      PmLogPrint((char *)IKINFO,(char *)"ListenRegisteredClients : closed connection with %d", fd);
	      break;

	    case CDAS_TRAILER_BAD:
	      PmLogPrint ((char *)IKINFO,(char *)"ListenRegisteredClients : BAD TRAILER");
	      break;

	    case CDAS_INPUTS_ERROR:
	      PmLogPrint ((char *)IKERROR,(char *)"ListenRegisteredClients : INPUTS ERROR");
	      break;

	    case CDAS_MALLOC_ERROR:
	      PmLogPrint ((char *)IKERROR,(char *)"ListenRegisteredClients : MALLOC ERROR");
	      break;

	    case CDAS_UNSUPPORTED_VERSION:
	    case CDAS_INVALID:
	      PmLogPrint ((char *)IKERROR,(char *)"ListenRegisteredClients : unknown error");
	    }
	}
    }

  if (_Number_Fd != 0)
    {
      IkFatalSend ("did not succeed to process all inputs, quit");
      PmLogPrint ((char *)IKERROR,(char *)"GetInputClients : did not succeed to process all inputs %d",_Number_Fd);
      exit (1);
    }
}

/*-----------------------------------------------------------------------*/

#define MINMICRO 150000
//#define MAXMICRO 800000  XB 2005-05-02, trying to get rid of black tanks...
#define MAXMICRO 800000
static int savecommsmicro=MINMICRO;
static int savecomms=0;
/*-----------------------------------------------------------------------*/
void
PmServer::ProcessInterrupts ()
/* DESCRIPTION:
 *   Check for IkMessage interrupts and get input raw data
 *   Stores and builds output before sending them to BSU.
 *   Sending 2 packets per second
 *
 * PARAMS:
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  // check Ik messages coming from CDAS internal clients
  IkMessageCheck ();
	
  struct timeval tv;
  struct timezone tz;

  gettimeofday (&tv, &tz);
  _currentTime = tv.tv_sec;

// Should work but doesn't... Give black tanks sometimes when 3 messages in 2 seconds XB 2007/01
  if (_currentTime>savecomms+1) savecommsmicro=MINMICRO;
  if ((tv.tv_usec > savecommsmicro) && (tv.tv_usec < MAXMICRO) && (_currentTime>savecomms))
#if 0
  // still 2 messages per second but one every 550 ms... didn't work...
  if (_currentTime>savecomms+1) savecommsmicro=MINMICRO;
  if (((tv.tv_usec > savecommsmicro+550000) && (tv.tv_usec < MAXMICRO) && (_currentTime==savecomms))||((_currentTime>savecomms) && (tv.tv_usec+450000 > savecommsmicro) && (tv.tv_usec > MINMICRO) && (tv.tv_usec < MAXMICRO)))
#endif
    { // Storing first message in the list
      OutputToBSU();
      usleep(10);
      // Removing 2nd packet (temporarily?) XB 2007/01
      //OutputToBSU();
      // to make sure more than 1 second passed
      gettimeofday (&tv, &tz);
      savecommsmicro=tv.tv_usec;
      savecomms=tv.tv_sec;
    }

  //gettimeofday (&tv, &tz);
      
  //if ((tv.tv_usec > 750000) && (tv.tv_usec < 850000)) 
    //{// Storing second message !! FIXME
      //OutputToBSU();
    //}
  
  _lastTime = _currentTime;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServer::OutputToBSU ()
/* DESCRIPTION:
 * PARAMS:
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
#if PMDEBUG_VERBOSE
  PmLogPrint((char *)IKDEBUG, (char *)"OutputToClients> Sending download output to BSU\n");
#endif  

  // Processing output data for download requests
  if (_pmDOWNLOADConnectionP)
    {
      if ( _pmDOWNLOADConnectionP->SendOutput () == CDAS_CONNECT_CLOSED )
	{

	  int sock = _FindSocketByConnection (PmDOWNLOADID);
	  _RemoveClient (sock);
	  _CloseConnection (sock);
	  PmLogPrint ((char *)IKINFO,(char *)"OutputToClients : closed connection with %d", sock);
	}
    }

  // Processing output data for BSU clients
  int numberOfBsu = (int) _listOfBsu.size();
  for (int i=0; i < numberOfBsu; i++)
    { 
      int bsuIndex = _listOfBsu[i];
      if ( bsuIndex != -1 && bsuIndex != -2 ) 
	{
	  _pmBSUConnection[bsuIndex]->Process ();
	  if (_pmBSUConnection[bsuIndex]->SendOutput () == CDAS_CONNECT_CLOSED)
	    {
	      int sock;
	      if ((sock = (int) _pmBSUConnection[bsuIndex]->GetSocket ()))
		{
		  _RemoveClient (sock);
		  _CloseConnection (sock);
		  PmLogPrint ((char *)IKINFO,(char *)"OutputToClients : closed connection with %d", sock);
		}
	      else
		PmLogPrint ((char *)IKERROR,(char *)"OutputToClients : can't disconnect bsu !!");
	    }
	}
      //else if ( bsuIndex == -1 ) usleep(100000); // BECAUSE OF uLSX´s...
      //else if ( bsuIndex == -1 ) usleep(10); // BECAUSE OF uLSX´s...
      else if ( bsuIndex == -2 ) break; // END
    }

}
/*-----------------------------------------------------------------------*/
void
PmServer::OutputToClients ()
/* DESCRIPTION:
 *   Reads the Output Data stored during the ProcessInterrupt process
 *   Sends these data to assigned clients
 *
 * PARAMS:
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
#if PMDEBUG_VERBOSE
  PmLogPrint((char *)IKDEBUG,(char *)"OutputToClients> Sending output to CDAS clients\n");
#endif
  // Processing output data for CDAS clients
  for (int i = 0; i < __MAXCDAS__; i++)
    {
      if (_pmCDASConnection[i])
	{
	  if (_pmCDASConnection[i]->SendOutput () == CDAS_CONNECT_CLOSED)
	    {
	      int sock;
	      if ((sock = (int) _pmCDASConnection[i]->GetSocket ()))
		{
		  _RemoveClient (sock);
		  _CloseConnection (sock);
		  PmLogPrint ((char *)IKINFO,(char *)"OutputToClients : closed connection with %d", sock);
		}
	      else
		PmLogPrint ((char *)IKERROR,(char *)"OutputToClients : can't disconnect cdas client !!");
	    }
	}
    }

#if PMDEBUG_VERBOSE
  PmLogPrint((char *)IKDEBUG,(char *)"OutputToClients> Resetting all services\n");
#endif
  // Resets all services output
  theT2Services ()->ResetOutput ();
  theEVENTServices ()->ResetOutput ();
  theCALIBServices ()->ResetOutput ();
  theMoSdRawServices ()->ResetOutput ();
  theT2FastServices ()->ResetOutput ();
  theSpmtSdServices()->ResetOutput();
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServer::TreatIkPmMessage (IkMessage * ikm)
/* DESCRIPTION:
 *   Ik messages coming from CDAS clients requests
 *   Treats message and their transfer mode
 *
 * PARAMS:
 *   IkMessage* = pointer to the handled IkMessage 
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  IkMessage *tmp = NULL;
  char *mode = NULL;
  int len = strlen ((((IkPmMessage *) ikm)->mode).c_str ());

  // Find the mode of request
  if (len)
    {
      mode = new char[len + 1];
      strcpy (mode, (((IkPmMessage *) ikm)->mode).c_str ());
    }

  if (!mode)
    {
      IkWarningSend ("Received Ik message of type %d in unknown mode.", ikm->get_type ());
      return;
    }

  // Processes Input read from the right mode
  if (strcmp (mode, mLIST) == 0 || strcmp (mode, mSLIST) == 0)
    {
      for (int i = 0; i < __MAXBSUONLINE__; i++)
	{
	  if (_pmBSUConnection[i])
	    {
	      longWord bsId = _pmBSUConnection[i]->GetBsId ();
	      int sock = _pmBSUConnection[i]->GetSocket ();
#if PMDEBUG
	      PmLogPrint((char *)IKDEBUG,(char *)"*  TreatIkMessage in mode list: bsId=%d socket=%d\n",(int)bsId,sock);
#endif
	      
              int BSUId_int = (int)bsId;
              if ((tmp = theStationDataBase ()-> ReBuildIkMessageWithListOfLsInThisBSu (ikm, BSUId_int, sock)))
              //if ((tmp = theStationDataBase ()-> ReBuildIkMessageWithListOfLsInThisBSu (ikm, (int) bsId, sock)))

		{
		  _pmBSUConnection[i]->GetInput (tmp, mode, ikm->get_type ());	// RR + not only mLIST but also mSLIST
		  delete tmp;
		  tmp = NULL;
		}
	    }
	}
    }
  else if (strcmp (mode, mANTILIST) == 0)
    {
      ;				// to do
    }
  else if (strcmp (mode, mBROADCAST) == 0)
    {
      for (int i = 0; i < __MAXBSUONLINE__; i++)
	{
	  if (_pmBSUConnection[i]) {
#if PMDEBUG
	    PmLogPrint((char *)IKDEBUG,(char *)"*  TreatIkMessage in mode broadcast: bsId=%d\n",(int)_pmBSUConnection[i]->GetBsId ());
#endif
	    _pmBSUConnection[i]->GetInput (ikm, mBROADCAST, ikm->get_type ());
	  }
	}
    }

  delete[]mode;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServer::IkPong ()
/*-----------------------------------------------------------------------*/
{
  if (IkPingValue)
    {
      IkSuPong pong;
      pong.PongValue = IkPingValue;
      pong.send (SU);
      IkPingValue = 0;
    }
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
vector < int >
PmServer::GetSocketsOpened ()
/*-----------------------------------------------------------------------*/
{
  vector < int >ret;

  for (unsigned int i = 0; i < _pmSocketLinkedWithService.size (); i++)
    ret.push_back (_pmSocketLinkedWithService[i].socket);
  return ret;
}

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
// LOCAL FUNCTIONS
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
PmStdConnection *
PmServer::_CheckProtocolOfNewConnection (int sock)
/*-----------------------------------------------------------------------*/
/* DESCRIPTION:
 *   Reading Pm header standard structure from the new socket allocated
 *   Filling the PmStdConnection object from the raw data sent by a CDAS client
 *   identified by there Id
 *   Checking packet Pm integrity
 *
 * PARAMS:
 *   sock = socket handled
 *
 * RETURN VALUE:
 *   PmStdConnection* = pointer to the object PmStdConnection
 */
/*-----------------------------------------------------------------------*/
{
  int nb = 0, len;
  int msgLen, dataLen, headerLen;
  longWord temp;
  PmStdConnection *pmStdConnectionP = NULL;
  PmStdHeader *pmStdHeaderP = NULL;

  // Building Pm standard header from raw data
  if (!(pmStdHeaderP = _ReadPmStdHeaderFromSocket (sock)))
    {
      PmLogPrint ((char *)IKERROR,(char *)"CheckProtocolOfNewConnection: pointer NULL to PmStdHeader");
      return NULL;
    }
  pmStdConnectionP = new PmStdConnection ();
  pmStdConnectionP->pmStdHeader = *pmStdHeaderP;
  delete pmStdHeaderP;

  msgLen = pmStdConnectionP->pmStdHeader.length;
  headerLen = pmStdConnectionP->pmStdHeader.hLength;
  dataLen = msgLen - headerLen;

  // Building the block part
  len = sizeof (longWord);
  nb = read (sock, (void *) &temp, len);
  pmStdConnectionP->serviceVersion = ntohl (temp);

  nb += read (sock, (void *) &temp, len);
  pmStdConnectionP->numberOfServices = ntohl (temp);
  PmLogPrint((char *)IKDEBUG,(char *)" Checking number of service on client connection sock=%d  %d (services)", sock, ntohl(temp));
  for (unsigned int i = 0; i < pmStdConnectionP->numberOfServices; i++)
    {
      nb += read (sock, (void *) &temp, len);
      pmStdConnectionP->service.push_back (ntohl (temp));
      nb += read (sock, (void *) &temp, len);
      pmStdConnectionP->numberOfParameterBytes = ntohl (temp);
      while (pmStdConnectionP->numberOfParameterBytes != 0)
	{
	  nb += read (sock, (void *) &temp, len);
	  if (pmStdConnectionP->pmStdHeader.type == PmBSID)
	    {			// Only in the case of PmBSID
	      pmStdConnectionP->id = ntohl (temp);
	    }
	  pmStdConnectionP->numberOfParameterBytes -= sizeof (longWord);
	}
    }
		
  // Building trailer
  nb += read (sock, (void *) &temp, len);
  if (nb != dataLen)
    {
      delete pmStdConnectionP;
      PmLogPrint ((char *)IKERROR,(char *)"CheckProtocolOfNewConnection: we are not reading the correct length!");
      return NULL;
    }

  pmStdConnectionP->trailer.id = ntohl (temp);
  if ((pmStdConnectionP->trailer.id) != PmMESSAGESENDMARKER)
    {
      delete pmStdConnectionP;
      PmLogPrint((char *)IKERROR,(char *)"CheckProtocolOfNewConnection: Bad PmTrailer");
      return NULL;
    }

  return pmStdConnectionP;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
PmStdHeader *
PmServer::_ReadPmStdHeaderFromSocket (int sock)
/* DESCRIPTION:
 *   Builds the Pm standard header from the raw data on current socket 
 *
 * PARAMS:
 *   sock = socket handled
 *
 * RETURN VALUE:
 *   PmStdHeader* = pointer to the object PmStdHeader
 */
/*-----------------------------------------------------------------------*/
{
  int nb = 0, len, dataLen;
  PmStdHeader *pmStdHeaderP;
  longWord temp;

  dataLen = sizeof (PmStdHeader);
  len = sizeof (longWord);

  pmStdHeaderP = new PmStdHeader ();

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
PmServer::_OpenConnection (PmStdConnection & pmStdConn, int sock, char *addrLsx)
/* DESCRIPTION:
 *   Creates a new specific object connection with it's Id
 *   For each specific clients, stores links with their assigned socket and
 *   only for the BSU adds a new item in the routing table
 *
 * PARAMS:
 *   sock = socket handled
 *   PmStdConnection = current object connection
 *
 * RETURN VALUE:
 *   cdasErrors = CDAS_UNSUPPORTED_VERSION
 *                CDAS_SERVICE_FULL
 *                CDAS_SUCCESS
 */
/*-----------------------------------------------------------------------*/
{
  /* Vérification de l'existence d'un protocole. */
  switch (pmStdConn.pmStdHeader.protocolVersion)
    {
    case 0:
    case 1:
      break;
    default:
      {
	IkWarningSend ("unknown protocol version : %d\n", pmStdConn.pmStdHeader.protocolVersion);
	PmLogPrint ((char *)IKERROR,(char *)"unknown protocol version : %d", pmStdConn.pmStdHeader.protocolVersion);
	return CDAS_UNSUPPORTED_VERSION;
      }
    }
  // Checking opened services and connected bsu clients
  switch (pmStdConn.pmStdHeader.type)
    {
    case PmBSID:
      {// BSU client requests
        // check if this is a new client
	unsigned int size = _pmSocketLinkedWithService.size();
	for (unsigned int k=0; k<size; k++) if (pmStdConn.id==_pmSocketLinkedWithService[k].bsId) {
          int sock = _pmSocketLinkedWithService[k].socket;
	  _CloseConnection(sock);
          _RemoveClient(sock);
	  if (FD_ISSET(sock, &_Temp_Fd)) { 
            FD_CLR (sock, &_Temp_Fd); 
	    _Number_Fd--;
  	  }
          IkWarningSend("BSU %d is trying to connect twice, deleting previous one",pmStdConn.id);
          break;
	}
	int i;
	for (i = 0; i < __MAXBSUONLINE__; i++)
	  {
	    if (!_pmBSUConnection[i])
	      {
		assert ( _pmBSUConnection[i] = new PmBSUConnection (pmStdConn, sock) ); // RR +
		PmSocketLinkedWithService tmp (PmBSID, sock, i, (int) pmStdConn.id, addrLsx);
		_pmSocketLinkedWithService.push_back (tmp);
		_OrderBsuByLsx();
		theStationDataBase ()->AddBSu ((int) pmStdConn.id, sock); // update BSU routing table
		break;
	      }
	  }
	if (i == __MAXBSUONLINE__)
	  return CDAS_SERVICE_FULL;
	break;
      }
    case PmT2:
    case PmT2Fast:
    case PmCTID:
    case PmEBID:
    case PmMOID:
    case PmCBID:
    case PmSpmt:
      {// CDAS tasks requests
	int i;
	for (i = 0; i < __MAXCDAS__; i++)
	  {
	    if (!_pmCDASConnection[i])
	      {
		assert ( _pmCDASConnection[i] = new PmCDASConnection (pmStdConn, sock) ); // RR +
		if (!_pmCDASConnection[i])
		  return CDAS_NO_SUCH_SERVICE;
		PmSocketLinkedWithService tmp (pmStdConn.pmStdHeader.type, sock, i, 0);
		_pmSocketLinkedWithService.push_back (tmp);
		break;
	      }
	  }
	if (i == __MAXCDAS__)
	  return CDAS_SERVICE_FULL;
	break;
      }
    case PmDOWNLOADID:
      {// Download client requests
	if (!_pmDOWNLOADConnectionP)
	  {
	    assert ( _pmDOWNLOADConnectionP = new PmDOWNLOADConnection (pmStdConn, sock) ); // RR +
	    PmSocketLinkedWithService tmp (PmDOWNLOADID, sock, 0, 0);
	    _pmSocketLinkedWithService.push_back (tmp);
	    break;
	  }
	return CDAS_SERVICE_FULL;
      }
    }

  return CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors
PmServer::_SendAck (int sock, int code)
/* DESCRIPTION:
 *   Sends acknowledge to clients with the correct protocol
 *
 * PARAMS:
 *   sock = socket handled
 *   code = 0 if success
 *
 * RETURN VALUE:
 *   cdasErrors = CDAS_WRITE_ERROR
 *                CDAS_SUCCESS
 */
/*-----------------------------------------------------------------------*/
{
  PmMessageAck Mess;
  int nb = 0, len;
  longWord temp;

  Mess.pmStdHeader.length = PmACKMESSLENGTH;
  Mess.pmStdHeader.protocolVersion = GsVERSION;	// RR - using macro
  Mess.pmStdHeader.type = GsACK;	// RR - using macro
  Mess.pmStdHeader.hLength = PmSTDHEADERLENGTH;
  Mess.version = 1;		// RR ? where is the macro for this!!
  Mess.errorCode = code;
  Mess.trailer.id = PmMESSAGESENDMARKER;

  len = sizeof (longWord);
  temp = htonl (Mess.pmStdHeader.length);
  nb = write (sock, (void *) &temp, len);
  temp = htonl (Mess.pmStdHeader.protocolVersion);
  nb += write (sock, (void *) &temp, len);
  temp = htonl (Mess.pmStdHeader.type);
  nb += write (sock, (void *) &temp, len);
  temp = htonl (Mess.pmStdHeader.hLength);
  nb += write (sock, (void *) &temp, len);
  temp = htonl (Mess.version);
  nb += write (sock, (void *) &temp, len);
  temp = htonl (Mess.errorCode);
  nb += write (sock, (void *) &temp, len);
  temp = htonl (Mess.trailer.id);
  nb += write (sock, (void *) &temp, len);
  if (nb != sizeof (PmMessageAck))
    {
      PmLogPrint ((char *)IKERROR,(char *)"SendAck: error only %i bytes written out of %i", nb, sizeof (PmMessageAck));
      return CDAS_WRITE_ERROR;
    }

  PmLogPrint((char *)IKINFO,(char *)"*  Acknowledge sent to the new client with %s", code ? "** failure **" : "++ success ++");
  return CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
cdasErrors
PmServer::_ManageClient (int socket)
/* DESCRIPTION:
 *   Read input data from a client and process data before building output to
 *   specific object client connection.
 *
 * PARAMS:
 *   sock = socket handled
 *
 * RETURN VALUE:
 *   cdasErrors = CDAS_SUCCESS
 *                CDAS_NO_SOCKET
 *                CDAS_CONNECT_CLOSED
 */
/*-----------------------------------------------------------------------*/
{
  int connection = _FindConnectionBySocket (socket);
  int bsu;

  switch (connection)
    {
    case PmBSID:
      {
	if ((bsu = _FindBSUBySocket (socket)) == -1)
	  {
	    IkWarningSend ("Unable to link socket with client!!");
	    PmLogPrint ((char *)IKERROR,(char *)"Unable to link socket with client!!");
	    return CDAS_NO_SOCKET;
	  }

	// Fetch data from BSu
#if PMDEBUG_VERBOSE
	PmLogPrint((char *)IKDEBUG,(char *)"BSUConnection object> GetInput() for BSU=%d",bsu);
#endif
	return _pmBSUConnection[bsu]->GetInput ();

      }
    case PmDOWNLOADID:
      {
	// Fetch data from a CDAS client
#if PMDEBUG_VERBOSE
	PmLogPrint((char *)IKDEBUG,(char *)"DOWNLOADConnection object> GetInput()");
#endif
	return _pmDOWNLOADConnectionP->GetInput ();

      }
    default:
      return CDAS_CONNECT_CLOSED;
    }

  return CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
void
PmServer::_RemoveClient (int sock)
/* DESCRIPTION:
 *   Removing connected client on current socket
 *   Updating the select connection pool
 *
 * PARAMS:
 *   sock = socket to be removed
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  if (shutdown (sock, 2) == -1) ERROR_SWITCH_SHUTDOWN(sock)
  close (sock);
  FD_CLR (sock, &_Global_Fd);

  if (sock == _max_fd)
    {
      for (int i = _max_fd; i > 0; i--)
	{
	  if (FD_ISSET (i, &_Global_Fd))
	    {
	      _max_fd = i;
	      break;
	    }
	} 
    }
  _num_data_client--;
  PmLogPrint ((char *)IKINFO,(char *)"Removing client at socket %d, number of client : %d", sock,
	      _num_data_client);
  IkInfoSend ("Removing client at socket %d, number of client : %d", sock,
	      _num_data_client);
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServer::_CloseConnection (int sock)
/* DESCRIPTION:
 *   Removing connected client on current socket
 *   Updating the select connection pool
 *
 * PARAMS:
 *   sock = socket to be removed
 *
 * RETURN VALUE:
 */
/*-----------------------------------------------------------------------*/
{
  int connection = _FindConnectionBySocket (sock);
  switch (connection)
    {
    case PmBSID:
      {
	int bsu = _FindBSUBySocket (sock);
        int bs_id = _FindBSIdBySocket (sock);
 
	if ( bsu == -1 || bs_id == -1 ) {
	  PmLogPrint ((char *)IKERROR,(char *)"CloseConnection> PmBSID object at socket=%d has bsu=%d - bsid=%d!",sock,bsu,bs_id);
		break;
	}
	_EraseSocketLinkedWithBySocket (sock);
        _OrderBsuByLsx();
	theStationDataBase ()->RemoveBSu (bs_id, sock);
	_pmBSUConnection[bsu]->RemoveClient ();
	delete _pmBSUConnection[bsu];
	_pmBSUConnection[bsu] = NULL;
	PmLogPrint ((char *)IKERROR,(char *)"CloseConnection> PmBSID object BSU[%d](socket=%d - id=%d) closed!",bsu,sock,bs_id);
#if PMDEBUG
	PmLogPrint((char *)IKDEBUG,(char *)"CloseConnection> PmBSID object BSU[%d](socket=%d - id=%d) closed!\n",bsu,sock,bs_id);
#endif
	break;
      }
    case PmT2:
    case PmT2Fast:
    case PmCTID:
    case PmEBID:
    case PmMOID:
    case PmCBID:
    case PmSpmt:
      {
	int cdas = _FindCDASBySocket (sock);

	if ( cdas == -1 ) {
	  PmLogPrint ((char *)IKERROR,(char *)"CloseConnection> CDASTasks object has no ID for socket=%d",sock);
		break;
	}
	_EraseSocketLinkedWithBySocket (sock);
	_pmCDASConnection[cdas]->RemoveClient ();
	delete _pmCDASConnection[cdas];
	_pmCDASConnection[cdas] = NULL;
	PmLogPrint ((char *)IKINFO,(char *)"CloseConnection> CDASTasks object (%d) closed at socket=%d!",cdas, sock);
#if PMDEBUG
	PmLogPrint((char *)IKDEBUG,(char *)"CloseConnection> CDASTasks object (%d) closed at socket=%d!\n",cdas, sock);
#endif
	break;
      }
    case PmDOWNLOADID:
      {
	_EraseSocketLinkedWithBySocket (sock);
	_pmDOWNLOADConnectionP->RemoveClient ();
	delete _pmDOWNLOADConnectionP;
	_pmDOWNLOADConnectionP = NULL;
	PmLogPrint ((char *)IKINFO,(char *)"CloseConnection> PmDOWNLOAD object closed at socket=%d!",sock);
#if PMDEBUG
	PmLogPrint((char *)IKDEBUG,(char *)"CloseConnection> PmDOWNLOAD object closed at socket=%d!\n",sock);
#endif
	break;
      }
    default:
      PmLogPrint ((char *)IKINFO,(char *)"fd %d has already been disconnected",sock);
      break;
    }
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmServer::_FindConnectionBySocket (int sock)
/*-----------------------------------------------------------------------*/
{
  for (unsigned int i = 0; i < _pmSocketLinkedWithService.size (); i++)
    if (sock == _pmSocketLinkedWithService[i].socket)
      return _pmSocketLinkedWithService[i].nameOfService;
  return -1;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmServer::_FindSocketByConnection (PmMessageType service)
/*-----------------------------------------------------------------------*/
{
  for (unsigned int i = 0; i < _pmSocketLinkedWithService.size (); i++)
    if (service == _pmSocketLinkedWithService[i].nameOfService)
      return _pmSocketLinkedWithService[i].socket;
  return -1;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmServer::_FindBSUBySocket (int sock)
/*-----------------------------------------------------------------------*/
{
  for (unsigned int i = 0; i < _pmSocketLinkedWithService.size (); i++)
    if (sock == _pmSocketLinkedWithService[i].socket)
      return _pmSocketLinkedWithService[i].bsu;
  return -1;
}

/*-----------------------------------------------------------------------*/
int
PmServer::_FindCDASBySocket (int sock)
/*-----------------------------------------------------------------------*/
{
  return _FindBSUBySocket( sock );
}
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
int
PmServer::_FindBSUByBsId (int bsId)
/*-----------------------------------------------------------------------*/
{
  for (int i = 0; i < __MAXBSUONLINE__; i++)
    {
      if (_pmBSUConnection[i])
	{
	  if ((longWord) bsId == _pmBSUConnection[i]->GetBsId ()) return i;
	}
    }
  return -1;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmServer::_FindBSIdBySocket (int sock)
/*-----------------------------------------------------------------------*/
{
  for (unsigned int i = 0; i < _pmSocketLinkedWithService.size (); i++)
    if (sock == _pmSocketLinkedWithService[i].socket)
      return _pmSocketLinkedWithService[i].bsId;
  return -1;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
int
PmServer::_FindSocketByBSId (longWord id)
/*-----------------------------------------------------------------------*/
{
  for (unsigned int i = 0; i < _pmSocketLinkedWithService.size (); i++)
    if (id == (longWord) _pmSocketLinkedWithService[i].bsId)
      return _pmSocketLinkedWithService[i].socket;
  return -1;
}

/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
int
PmServer::_EraseSocketLinkedWithBySocket (int sock)
/*-----------------------------------------------------------------------*/
{
  for (unsigned int i = 0; i < _pmSocketLinkedWithService.size (); i++)
    if (sock == _pmSocketLinkedWithService[i].socket) {
	_pmSocketLinkedWithService.erase (_pmSocketLinkedWithService.begin () + i);
	return 1;
    }
  return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
void
PmServer::_OrderBsuByLsx()
/*-----------------------------------------------------------------------*/
{
  _listOfBsu.clear();
  list<int> listOfLsxUnique; // list only to take profit of the unique function...
  list<int>::iterator it;
  int sizeOfSockets = (int) _pmSocketLinkedWithService.size ();
  for(int i=0; i<sizeOfSockets; i++) 
    {
      if (_pmSocketLinkedWithService[i].lsxAddr != -1) // because of cdas clients
	listOfLsxUnique.push_back(_pmSocketLinkedWithService[i].lsxAddr);
    }
  listOfLsxUnique.sort();
  listOfLsxUnique.unique();
  int numberOfLsx=(int)listOfLsxUnique.size();

  int matrixBsuLsx[40][40];
  for (int i=0; i<40; i++) for (int j=0; j<40; j++) matrixBsuLsx[i][j]=-99;
  int nBsuMaxPerLsx=0;
  it = listOfLsxUnique.begin();
  for(int i=0; i<numberOfLsx; i++,it++)
    {
      int tmp=0, k=0; 
      for(int j=0; j<sizeOfSockets; j++) if (_pmSocketLinkedWithService[j].lsxAddr == *(it)) 
	{
	  matrixBsuLsx[i][k++] = _pmSocketLinkedWithService[j].bsu;
	  tmp++;
	}
      if (tmp>nBsuMaxPerLsx) nBsuMaxPerLsx=tmp;
    }

  // 
  for (int k=0; k<nBsuMaxPerLsx; k++) 
    {
      for(int j=0; j<numberOfLsx; j++) 
	{
	  if (matrixBsuLsx[j][k]!=-99) _listOfBsu.push_back(matrixBsuLsx[j][k]);
	}
      _listOfBsu.push_back(-1);
    }
  
  _listOfBsu.push_back(-2);
}

/*-----------------------------------------------------------------------*/

