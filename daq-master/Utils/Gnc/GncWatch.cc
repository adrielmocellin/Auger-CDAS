#include <sys/types.h>
#include <time.h>
#include "GncInit.h"
#include "GncWatch.h"
#include "GncError.h"
#include "GncCmp.h"

#define TRYTOOPEN 1
#define DONTOPEN  0

static void setWatch(CDASSERVER *server) 
{
  server->watch = TRUE;
  server->state = CLOSED;
}

/******************************************************************************
 *
 * CDASSERVER *GncSetWatch (serverName)
 *
 *  This routine does the following - 
 *            - set up the client to periodically check if a connection can be
 *               made
 *
 *  Returns -
 *
 ******************************************************************************
 */
CDASSERVER *GncSetWatch(const char *serverName, int service, char *serverAddr, 
			int serverPort, int (*userInit)(CDASSERVER *srvr), 
			int (*userProcess)(CDASSERVER *srvr))
{
  CDASSERVER *server;

  /* see if a server structure exists, if not, make one */
  if ((server = GncGetServerByName(serverName)) == NULL) {
    server = GncInitServer(server, 0, serverName, service, serverAddr, 
			   serverPort, userInit, userProcess);
  }
  setWatch(server);
  return server;
}

/******************************************************************************
 *
 * int GncWatch (serverName, rfds)
 *
 *  This routine does the following - 
 *            - examine the specified connection 
 *            - check if the connection is still alive
 *            - if not, start the procedure for opening it again.  (i.e. - try
 *                to open it now, if no success, again in 2 secs then 4 secs
 *                etc. ????????????TBD)
 *
 *  Returns -
 *       1 : the connection needs to be reopened
 *       0 : the connection does not need to be reopened
 *
 ******************************************************************************
 */
int GncWatch(const char *serverName, int fd, fd_set rfds)
{
  CDASSERVER *server;
  int state = DONTOPEN, rtn = CLOSED;

  /* check to make sure that the connection is still alive.
     if the connection is closed, then the rfds structure will have had the
     file descriptor removed from it. */
  if (fd != BADFD) {
    server = GncGetServerByFd(fd);
  } else {
    server = GncGetServerByName(serverName);
  }
  if (server) {
    if ((! server->fd) || (! FD_ISSET(server->fd, &rfds))) {
      /* the connection was closed, now we have these cases - 
                  the connection was never open - do nothing
                  the connection used to be open - it should be reopened 
                  the connection is still marked open - should be reopened */
      if (server->state == OPEN) {
	/* this is the first time noticing the connection has been closed */
	setWatch(server);
	state = TRYTOOPEN;
      } else {
	/* the connection is marked closed.  if connection is being watched we 
	   should try again to reopen it */
	if (server->watch == TRUE) {
	  state = TRYTOOPEN;
	}
      }
    }
    if (state == TRYTOOPEN) {
      time_t currentTime;
      /* is it time to watch the connection? we don't want to do it too 
	 often */
      currentTime = time(NULL);
      if ((server->lastWatched + server->nextWatched) <= currentTime) {
	/* ok it is time */
	server->lastWatched = currentTime;
	server->nextWatched += server->nextWatched;
	if (server->nextWatched > MAXTIMEINCREMENT) {
	  server->nextWatched = MAXTIMEINCREMENT;
	}
	/* we should try to open the connection */
	if (GncInit(serverName, server->service, server->host, server->port,
		    server->userInit, server->userProcess, &server) == 
	    CDAS_SUCCESS) {
	  /* set the watch for the next time */
	  server->watch = TRUE;
	  rtn = OPEN;
	}
      }
    }
  }
  return(rtn);
}
