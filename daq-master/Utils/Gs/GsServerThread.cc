/* ****************************************************************** */
/* GsServerThread.c :                                                 */
/*                                                                    */
/*     The Gs Ip Server manage the incoming IP connections.           */
/*                                                                    */
/*                                                                    */
/* ****************************************************************** */

/*! 
  The Gs Ip Server wait for connections on the server port. Once a connection
  is accepted, it "forks" a new manager (GsClientManager) for the incoming 
  client. As the "Parent" of all manager threads it will handle the Ik and 
  former "EndOfLoop" managers too.
*/


/*--------------------------------------------------------------------------------*/
/* GsACK */
/* TO BE CHANGED TO ALLOW VARIABLE SIZE STATUS (redirection) */
/* and to protect against bad clients (no manager created yet) */
static inline int GsSendAck(int fd, int errorCode)
{
  GsAcknowledgeMessage mess;
 
  mess.header.length = htonl(GsACKMESSAGELENGTH);
  mess.header.protocolVersion = htonl(GsVERSION);
  mess.header.type = htonl(GsACK);
  mess.header.hLength = htonl(GsSTDHEADERLTH);
  mess.version = htonl(1); /* to be changed ? */
  mess.errorCode = htonl(errorCode);
  mess.trailer.id = htonl(GsMESSAGESENDMARKER);
  return(exactWrite(fd,&(mess.header),closeFile));
}


/*--------------------------------------------------------------------------------*/


void * GsServer(void *param) // thread waiting for connections
{
  GsClientP client;

  int newClient; // TO BE CHANGED : bad name

  char * ir;

  unsigned int
    nfd,
    data,
    rSize;

  struct sockaddr_in
    hsta;

  fd_set
    crfds;
 
  struct timeval tv;



  /* TO BE DONE Signals handling are defined in GsLoop */
  /* threads should only block them */

  // get the parameters
  client = (GsClientP)param;
  /* get the port we are watching for */
  data= GsClientFd(client); /* no risk here, we are at the beginning */

  /* now watch for input */
  FOREVER
    {
    FD_SET (data, &crfds);
    /* TO BE CHECKED : don't sleep forever in order to exit properly */
    tv.tv_sec=2;
    tv.tv_usec=700;  /* how long should we wait for */
    
    nfd=select(data+1,&crfds,NULL,NULL,&tv); /* somebody here ? */
    switch(nfd) 
    {
    case -1 : /* select didn't work : that's bad ! */
      GsNotifyIk(TOLOG,IKFATAL,
		 "select() error : %s on fd %d ",strerror(errno), data);
      closeServer(EXIT_FAILURE,client);
      break;
    case 0 : /* nobody's comming : GOOD, I'm lazzy ? */
      break;
    default : /* somebody's connecting */
#ifdef VERBOSE
      GsNotifyIk(TOLOG,IK_INFO,
		 " new connection on %d fd",data);
#endif
      break;
    } /* switch nfd */
    
    if(FD_ISSET(data,&crfds)) // if not, how did we reach this point ???
    {
      GsNotifyIk(TOLOG,IK_INFO,
		 "new connection");
      nfd--;
      rSize=sizeof(struct sockaddr_in);
      if((newClient = accept(data,(struct sockaddr *)&hsta,&rSize))==(-1)) 
      {
	GsNotifyIk(TOLOG,IK_SEVERE,
		   "accept() error : %s",strerror(errno));
      }
      else  /* accept worked */
      {
	/* get block */
	if(readGsBlock(data,&ir,closeFile) == CDAS_SUCCESS)
	{
	  // setup "pseudo client"
	  /* signal data present to Gs */
	  /* don't pass data until previous message is ok */
	  GsClientInputLock(client);
	  GsClientSetFd(client,newClient);
	  GsClientSetInputData(client,(GsStdHeader*)ir);
	  FD_SET(data,&GsRfds);
	  GsClientInputUnlock(client);
	  pthread_cond_signal(&GsInputWaiting);
	} /* if exactRead Ok */
      } /* else (accept worked )*/

    } /* if FDISSET ( data ) */

    if(nfd!=0) /* did we process all inputs ? */
    {
      GsNotifyIk( TOLOG, IK_SEVERE,
		  "GsServer :select for %d saw something unexpected",data); 
      closeServer(EXIT_FAILURE,client); 
    }
  } /* FOREVER */
  return ((void*)NULL); // just to be consistent with declaration
}
