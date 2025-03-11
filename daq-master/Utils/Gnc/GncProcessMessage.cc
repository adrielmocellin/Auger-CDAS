/******************************************************************************
 ******************************************************************************
 *
 * This file will call a routine to handle the arrived message.
 * The following user callable routines are contained in this file :
 *
 *    GncProcessMessage
 *
 *
 ******************************************************************************
 ******************************************************************************
 */
#include <stdlib.h>

#include "GncError.h"
#include "GncInit.h"
#include "GncRead.h"
#include "GncWrite.h"
#include "GncCmp.h"
#include "GncClose.h"
#include "IkC.h"

//#include "UiLk.h"
#include "LkListP.h"
//#include "UiMessage.h"

#define DATALTH 10

#define GDATABUFFSIZE 5000
void *g_dataBuff = NULL;
int g_dataBuffCurrentSize = 0;

extern LkList g_servers;

/******************************************************************************
 *
 * int GncProcessMessage (int fd)
 *
 *  This routine does the following - 
 *            - read in the message 
 *            - pass it to a user routine to handle it
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
int GncProcessMessage(int fd)
{
  int rtn = CDAS_SUCCESS, drtn;
  int bytesRead;
  CDASSERVER *server = GncGetServerByFd(fd);

  if (server != NULL) {
    /* malloc space to hold the message */
    if (g_dataBuffCurrentSize == 0) {
      if ((g_dataBuff = malloc(GDATABUFFSIZE)) == NULL) {
	rtn = CDAS_MALLOC_ERROR;
	IkFatalSend("GncProcessMessage could not malloc %d bytes",GDATABUFFSIZE);
      }
      g_dataBuffCurrentSize = GDATABUFFSIZE;
    }
    /* read in the message */
    if ((rtn = GncRead(server, g_dataBuff, g_dataBuffCurrentSize, 
		       &bytesRead))
	== CDAS_SUCCESS) {
      /* call a user routine to handle this message */
      if (server->userProcess != NULL) {
	rtn = (server->userProcess)(server);
	if (rtn == CDAS_CONNECT_CLOSED) {
	  drtn = GncClose(server);
	}
      }
    } else if (rtn == CDAS_CONNECT_CLOSED) {
      GncClose(server);
    }
  } else {
    /* no server matched the file descriptor number */
    rtn = CDAS_NO_SERVER;
    IkSevereSend("GncProcessMessage server 0x%04X does not match any of the file descriptor numbers",fd);
  }
  return(rtn);
}
