/******************************************************************************
 ******************************************************************************
 *
 * This file contains generic client initialization routines.  These routines
 * are called by clients who wish to receive data and messages from the CDAS 
 * system. 
 * The following user callable routines are contained in this file :
 *
 *    GncInit
 *
 *
 ******************************************************************************
 ******************************************************************************
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "GncError.h"
#include "GncInit.h"
#include "GncCmp.h"
#include "GncClose.h"
#include "GncWatch.h"
#include "IkC.h"
//#include "UiMessage.h"
//#include "UiLk.h"
#include "LkList.h"

#define INVALID_IP 0xFFFFFFFF
#define max(i0,i1) (i0 < i1 ? i1 : i0)

LkList g_servers = NULL;
fd_set g_rfds;
int g_rfds_set = 0;
int g_maxfd = 0;
char *g_serverAddr;
int g_serverPort;

/******************************************************************************
 *
 * int getServerAddr (sAddr)
 *      sAddr : IP address or host name of the server
 *
 *  This routine does the following - 
 *            - get a server address suitable for input to socket routines
 *
 *  Returns -
 *       SUCCESS : the server address
 *       FAILURE : 0xFFFFFFFF
 *
 ******************************************************************************
 */
static int getServerAddr(char *addrBuf, unsigned long *iAddr)
{
  int rtn = CDAS_SUCCESS;
  struct hostent *host;

  /* get the address of host, first try the dot format */
  if ((*iAddr = inet_addr(addrBuf)) == INVALID_IP) {
    /* now try hostname format */
    host = gethostbyname(addrBuf);
    if (host) {
      *iAddr = *( (unsigned long *) host->h_addr_list[0]);
    }
    if (*iAddr == ADDRESS_ERROR) {
      rtn = CDAS_ADDR_ERROR;
      IkSevereSend("getServerAddr Could not get address : %s", addrBuf);
    }
  }
  return(rtn);
}

/******************************************************************************
 *
 * int openServerComm (server)
 *      server : name of CDAS server to connect to
 *
 *  This routine does the following - 
 *            - establish a TCP/IP connection with the desired server
 *
 *  Returns -
 *       SUCCESS : port file descriptor
 *       FAILURE : CDAS_CONNECT_ERROR, CDAS_NO_SOCKET
 *
 ******************************************************************************
 */
static int openServerComm(char *serverAddr, int serverPort, int *portfd)
{
  int rtn = CDAS_SUCCESS;   /* return value */
  unsigned long sAddr;      /* server address for socket routines */
  struct sockaddr_in hsta;  /* server address in socket space */

  /* get the address of the server */
  if ((rtn = getServerAddr(serverAddr, &sAddr)) == CDAS_SUCCESS) {
    /* get a socket */
    if ((*portfd = socket(AF_INET,SOCK_STREAM,0)) != (-1)) {
      bzero((void *)&hsta, sizeof(struct sockaddr_in));
      hsta.sin_addr.s_addr = sAddr;
      hsta.sin_family = AF_INET;
      hsta.sin_port = htons(serverPort);
      /* connect to the server */
      if ((connect(*portfd, (struct sockaddr *)&hsta,
		   sizeof(struct sockaddr_in))) != (-1)) {
	/* we have a connection */
	g_maxfd = max(g_maxfd, *portfd);
      } else {
	/* could not establish connection */
	close(*portfd);
	rtn = CDAS_CONNECT_ERROR;
	IkSevereSend("openServerComm could not connect at address %s on port %d", serverAddr, serverPort); 
      }
    } else {
      /* could not get socket file descriptor */
      rtn = CDAS_NO_SOCKET;
      IkSevereSend("openServerComm could not get socket");
    }
  }
  return(rtn);
}

/******************************************************************************
 *
 * void GncInitServer (server, fd, name, service, address, port, userInit,
 *                     userProcess)
 *      server      : CDASSERVER structure to init
 *      fd          : open socket descriptor
 *      name        : name of server
 *      service     : requested service  (???? to be changed)
 *      address     : address of server
 *      port        : port of server
 *      userInit    : user init function
 *      userProcess : user process function
 *
 *  This routine does the following - 
 *            - initialize the passed in CDASserver structure
 *
 *  Returns -
 *
 ******************************************************************************
 */
CDASSERVER *GncInitServer(CDASSERVER *server, int fd, const char *name, 
			  int service, char *serverAddr, int serverPort,
			  int (*userInit)(CDASSERVER *server),
			  int (*userProcess)(CDASSERVER *server))
{
  LkEntry entry;
  char *tmpStr;

  if (! server) {
    entry = LkEntryNew(g_servers, sizeof(CDASSERVER));
    server = (CDASSERVER *)entry;
    server->initFlag = FALSE;
    server->name = NULL;
    server->host = NULL;
  }
  server->fd = fd;
  /* do this first because the user may pass in name as server->name, so copy
     it before deleting it */
  tmpStr = GncGetMem(name);
  if (server->name) {
    free(server->name);
  }
  server->name = tmpStr;
  server->userInit = userInit;
  server->userProcess = userProcess;
  server->service = service;
  /* do this first because the user may pass in serverAddr as server->host, so
     copy it before deleting it */
  tmpStr = GncGetMem(serverAddr);
  if (server->host) {
    free(server->host);
  }
  server->host = tmpStr;
  server->port = serverPort;

  /* only init the default values if we have not initialized this server
     before. */
  if (server->initFlag == FALSE) {
    server->state = OPEN;
    server->watch = FALSE;
    server->deleteFlag = 1;
    server->initFlag = TRUE;
    server->lastWatched = 0;
    server->nextWatched = MINTIMEINCREMENT;
  }
  return(server);
}

/******************************************************************************
 *
 * int GncInit (serverName, service, userInit, userProcess, server)
 *      serverName  : char * name of the CDAS server to which the user wants
 *                     to connect.
 *      service     : requested service
 *      serverAddr  : IP address or host where server is running
 *      serverPort  : port number connected to the server
 *      userInit    : user initialization routine that will be called for each
 *                     server with a successful connection
 *      userProcess : user processing routine that will be called for each
 *                     arrived message
 *      server      : returned server structure.  the initial value of this
 *                     must be NULL or pointing to a valid server structure.
 *
 *  This routine does the following - 
 *            - open socket connections to the specified server
 *            - register to receive messages from Ik if desired.
 *            - call a user intiailization routine
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *                 0  (no server passed)
 *       FAILURE : other
 *
 ******************************************************************************
 */
int GncInit(const char *serverName, int service, char *serverAddr, 
	    int serverPort, int (*userInit)(CDASSERVER *server),
	    int (*userProcess)(CDASSERVER *server),
	    CDASSERVER **server)
{
  int sPort, rtn = CDAS_SUCCESS;
  int drtn;
  CDASSERVER *l_server = NULL;
  
  /* get a server list if not done already */
  if (g_servers == NULL) {
    g_servers = LkListNew();
  }
  /* make sure we have been passed something */
  if (strcmp(serverName, NULLSTRING) != 0) {
    /* open a connection to the server passed to us. */
    if ((rtn = openServerComm(serverAddr, serverPort, &sPort)) == 
	CDAS_SUCCESS) {
      /* save this information for later */
      l_server = GncInitServer(*server, sPort, serverName, service, 
			       serverAddr, serverPort, userInit, userProcess);
      l_server->state = OPEN;
      /* call the user call out routine */
      if (userInit != NULL) {
	if ((rtn = userInit(l_server)) == CDAS_SUCCESS) {
	  if (!g_rfds_set) {
	    FD_ZERO(&g_rfds);       /* setup for reading from this port */
	    g_rfds_set = 1;
	  }
	  FD_SET(sPort, &g_rfds);
	} else {
	  /* error when calling the user initialization routine. 
	     back out */
	  drtn = GncClose(l_server);
	  GncDelServer(&l_server);
	  l_server = NULL;
	}
      }
    }
  } else {
    /* no server name was passed to us */
    rtn = CDAS_NO_SERVER;
    IkSevereSend("GncInit no server name given");
  }

  if (server != NULL) {
    /* return the server structure to the user */
    *server = l_server;
  }

  return(rtn);
}

/******************************************************************************
 *
 * int GncGetMem(host)
 *      host : host string to make a duplicate of
 *
 *  This routine does the following - 
 *            - make a duplicate of the passed in string
 *
 *  Returns -
 *       SUCCESS : string address
 *       FAILURE : NULL
 *
 ******************************************************************************
 */
char *GncGetMem(const char *string)
{
  int len = strlen(string);
  char *newString = (char *)malloc(len+1);  /* leave room for '\0' */

  if (newString != NULL) {
    memcpy(newString, string, len);
    newString[len] = '\0';
  } else {
    IkFatalSend("GncGetMem could not malloc %d bytes", len);
  }
  return(newString);
}
