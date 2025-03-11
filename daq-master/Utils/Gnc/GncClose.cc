/******************************************************************************
 ******************************************************************************
 *
 * This file contains generic client exit routines.  These routines are
 * called by clients who are currently connected to the CDAS system and wish to
 * close all connections to it.
 * The following user callable routines are contained in this file :
 *
 *    GncClose
 *
 *
 ******************************************************************************
 ******************************************************************************
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "GncError.h"
#include "GncClose.h"
#include "GncInit.h"
#include "IkC.h"
//#include "UiLk.h"
#include "LkListP.h"
//#include "UiMessage.h"

extern fd_set g_rfds;
//extern int uiErrno;

/******************************************************************************
 *
 * int GncDelServer (server)
 *
 *  This routine does the following - 
 *            - delete the server structure and un-hook it from the list of servers
 *
 *  Returns -
 *       SUCCESS : ---
 *       FAILURE : ---
 *
 ******************************************************************************
 */
void GncDelServer(CDASSERVER **server)
{
  CDASSERVER *l_server = *server;

  if (l_server->deleteFlag) {
    /* we malloced the host address value and name so free them */
    free(l_server->host);
    free(l_server->name);
    LkEntryDelete((LkEntry )l_server);
    *server = NULL;
  } else {
    l_server->state = CLOSED;
    LkEntryDisconnect((LkEntry )l_server);
  }
}
/******************************************************************************
 *
 * int GncClose (server)
 *
 *  This routine does the following - 
 *            - closes all open socket connections to the entered CDAS servers.
 *                if no servers are entered, close all open socket connections
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : CDAS_SOCKET_CLOSE_ERROR
 *
 ******************************************************************************
 */
int GncClose(CDASSERVER *server)
{
  int rtn;

  /* close the connection */
  if (server != NULL) {
    if (server->state == OPEN) {
      /* close the port */
      if (close(server->fd) != 0) {
	/* error in close */
	rtn = CDAS_SOCKET_CLOSE_ERROR;
	IkWarningSend("GncClose could not close socket on server %s id %d",
                   server->name, server->fd);
      } else {
	server->state = CLOSED;
	rtn = CDAS_SUCCESS;
      }
    }
    /* remove this element from the list of connections. */
    FD_CLR(server->fd, &g_rfds);
  } else {
    /* no server means nothing to close */
    rtn = CDAS_SUCCESS;
  }

  return(rtn);
}
