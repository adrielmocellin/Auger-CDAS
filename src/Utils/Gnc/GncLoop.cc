/******************************************************************************
 ******************************************************************************
 *
 * This file contains the main loop that will do the real work of the client.
 * The following user callable routines are contained in this file :
 *
 *    GncLoop
 *
 *
 ******************************************************************************
 ******************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "GncError.h"
#include "GncWrite.h"
#include "GncRead.h"
#include "GncInit.h"
#include "GncProcessMessage.h"
#include "GncWatch.h"
#include "GncCmp.h"
#include "IkC.h"
//#include "UiLk.h"
#include "LkListP.h"
//#include "UiMessage.h"

#define WATCHING 1
#define NOTWATCHING 0

extern fd_set g_rfds;          /* keep list of read file ids to watch for */
extern int g_maxfd;
extern LkList g_servers;

/******************************************************************************
 *
 * int GncLoop ()
 *
 *  This routine does the following - 
 *            - wait in a select loop for input on ont of our open socket fds.
 *            - process a message when we get one.
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
int GncLoop(void)
{
  int i, rtn = CDAS_SUCCESS;
  fd_set crfds;
  struct timeval tv;
  int nfd, limit, some_left, j;
  LkIterator iterator;
  LkEntry entry;

  /* wait to do something until we are sent something */
  for(;;) {
    /* Allow user to answer IkMessage if any */
    IkMessageCheck();
    /* make sure we have something to wait for */
    if (LkListCount(g_servers) == 0) {
      break;
    }
    crfds=g_rfds;     /* who should be watched for input */
    tv.tv_sec=6;      /* we need to reset this each time as it is changed */
    tv.tv_usec=700;   /* how long should we wait for */
      
    if ((nfd = select(g_maxfd+1, &crfds, NULL, NULL, &tv)) == -1) {
      /* select didn't work : that's bad ! */
      rtn = CDAS_SELECT_ERROR;
      IkSevereSend("GncLoop failed on select, sleeping for 1 second");
      sleep(1);
//      break;
    } else if (nfd > 0) {
      /* check for incoming messages */
      limit = g_maxfd + 1;
      for (i = 1; i < limit; i++) {
	if (FD_ISSET(i, &crfds)) {
	  nfd--;
	  FD_CLR(i, &crfds);
	  
	  /* handle the message */
	  if ((rtn = GncProcessMessage(i)) != CDAS_SUCCESS) {
	    /* there was an error, see if the connection is closed */
	    if (rtn == CDAS_CONNECT_CLOSED) {
	      /* no longer look for this fd */
	      FD_CLR(i, &g_rfds);
	      /* if we have nothing to wait for, return */
	      some_left = 0;
	      for (j = 1; j < limit; ++j) {
		if (FD_ISSET(j, &g_rfds)) {
		  some_left = 1;
		  break;
		}
	      }
	      iterator = LkIteratorNew(g_servers);
	      while ((entry = LkIteratorNext(iterator)) != NULL) {
		if (((CDASSERVER *)entry)->watch == TRUE) {
		  some_left = 1;
		  break;
		}
	      }
	      LkIteratorDelete(iterator);
	      if (some_left == 0) {
		return(rtn);
	      }
	    }
	  }
	} /* if FDISSET ( clients) */
      } /* loop on incoming messages */
      
      if(nfd!=0) { /* did we process all inputs ? */
	rtn = CDAS_INPUTS_ERROR;
	IkFatalSend("GncLoop failed to process some inputs");
	break;
      }
    }
    /* now see if we are watching any servers and try to reconnect if we
       are and the connection is closed */
    iterator = LkIteratorNew(g_servers);
    while ((entry = LkIteratorNext(iterator)) != NULL) {
      GncWatch(((CDASSERVER *)entry)->name, BADFD, g_rfds);
    }
    LkIteratorDelete(iterator);
  }
  return(rtn);
}
