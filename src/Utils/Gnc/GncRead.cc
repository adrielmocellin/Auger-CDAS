/******************************************************************************
 ******************************************************************************
 *
 * This file contains generic client read routines.  These routines are
 * called by clients who wish to receive data from a CDAS server.
 * The following user callable routines are contained in this file :
 *
 *    GncRead
 *    GncGetHeaderLastRead
 *
 ******************************************************************************
 ******************************************************************************
 */

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>

#include "GncError.h"
#include "GncInit.h"
#include "GncRead.h"
#include "IkC.h"

//#include "UiErr.h"
#include "GsMess.h"
//#include "UiMessage.h"

GsStdHeader g_header = {0, 0, 0, 0};
GsTrailer g_trailer = {0};
int g_bytesRead = 0;
void *g_data = NULL;

/******************************************************************************
 *
 * int exactRead(server, buffer, bytes)
 *      server : server structure
 *      buffer : buffer in which to put the data
 *      bytes  : number of bytes to read
 *
 *  This routine does the following - 
 *            - read the specified number of bytes
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other 
 *
 ******************************************************************************
 */
static int exactRead(CDASSERVER *server, void *buffer, int bytes, 
		     int *bytesRead)
{
  int n, rtn = CDAS_SUCCESS, bytesLeft = bytes;
  char *tmpB = (char *)buffer;

  for (*bytesRead = 0; bytesLeft > 0; bytesLeft -= n, tmpB += n, 
	 *bytesRead += n) {
    if ((n = read(server->fd, tmpB, bytesLeft)) == -1) {
      /* error during read */ 
      rtn = CDAS_READ_ERROR;
      IkFatalSend("exactRead failed on server %s (id %d)", server->name, server->fd);
      break;
    } else if (n == 0) {
      /* the connection has been closed */
      rtn = CDAS_CONNECT_CLOSED;
      break;
    }
  }
  return(rtn);
}

/******************************************************************************
 *
 * int GncReadHeader (server)
 *      server : server structure
 *
 *  This routine does the following - 
 *            - read the header of the message
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
static int GncReadHeader(CDASSERVER *server, int *bytesRead)
{
  int rtn = CDAS_SUCCESS;

  if ((rtn = exactRead(server, (void *)&g_header, GsSTDHEADERLTH, bytesRead)) ==
      CDAS_SUCCESS) {
    /* convert it to local byte order */
    g_header.length = ntohl(g_header.length);
    g_header.type = ntohl(g_header.type);
    g_header.protocolVersion = ntohl(g_header.protocolVersion);
    g_header.hLength = ntohl(g_header.hLength);
  }
  return(rtn);
}

/******************************************************************************
 *
 * int GncReadData (server, buffer, bufferSize)
 *      server     : server structure
 *      buffer     : buffer in which to put the data
 *      bufferSize : size of this buffer
 *
 *  This routine does the following - 
 *            - read the message data
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
static int GncReadData(CDASSERVER *server, void *buffer, int dataSize,
		       int *bytesRead)
{
  int rtn = CDAS_SUCCESS;

  if (dataSize > 0) {
    rtn = exactRead(server, buffer, dataSize, bytesRead);
  } else {
    *bytesRead = 0;
  }
  return(rtn);
}

/******************************************************************************
 *
 * int GncReadTrailer (server)
 *      server : server structure
 *
 *  This routine does the following - 
 *            - read the trailer of the message
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
static int GncReadTrailer(CDASSERVER *server, int *bytesRead)
{
  int rtn;

  if ((rtn = exactRead(server, (char *)&g_trailer, GsTRAILERLTH, bytesRead)) == 
      CDAS_SUCCESS) {
    /* convert it to local byte order */
    g_trailer.id = ntohl(g_trailer.id);
    if (g_trailer.id != GsMESSAGESENDMARKER) {
      /* we do not have the correct thing */
      rtn = CDAS_TRAILER_BAD;
      IkSevereSend("GncReadTrailer uncorrect trailer");
    }
  }
  return(rtn);
}

/******************************************************************************
 *
 * int GncRead (server, buffer, bufferSize)
 *      server     : server structure
 *      buffer     : buffer to read into
 *      bufferSize : size of buffer
 *
 *  This routine does the following - 
 *            - read information from the specified server into the buffer
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
int GncRead(CDASSERVER *server, void *buff, int buffSize, int *dbytesRead)
{
  int rtn = CDAS_SUCCESS;
  int bytesData;
  int bytesRead;

  if (server->state == OPEN) {
    if ((rtn = GncReadHeader(server, &bytesRead)) == CDAS_SUCCESS) {
      /* get number of bytes of data to read (total - (header + trailer)) */
      bytesData = g_header.length - (bytesRead + GsTRAILERLTH);
      if (bytesData <= buffSize) {
	/* set bytes in a different variable so we can return it to user */
	if ((rtn = GncReadData(server, buff, bytesData, dbytesRead)) ==
	    CDAS_SUCCESS) {
	  g_data = buff;
	  g_bytesRead = *dbytesRead;
	  rtn = GncReadTrailer(server, &bytesRead);
	}
      } else {
	/* buffer to read into is too small. */
	rtn = CDAS_BUFFER_TOO_SMALL;
	IkWarningSend("GncRead buffer too small, got %d bytes need %d", buffSize, bytesData);
      }
    }
  }
  return(rtn);
}

/******************************************************************************
 *
 * int GncGetLenLastRead (void)
 *
 *  This routine does the following - 
 *            - return the last number of data bytes read during the last read
 *
 *  Returns -
 *       the last read data size in bytes
 *
 ******************************************************************************
 */
int GncGetLenLastRead(void)
{
  return(g_bytesRead);
}

/******************************************************************************
 *
 * GsStdHeader GncGetHeaderLastRead (void)
 *
 *  This routine does the following - 
 *            - return the last read header
 *
 *  Returns -
 *       the last read header
 *
 ******************************************************************************
 */
GsStdHeader *GncGetHeaderLastRead(void)
{
  return(&g_header);
}

/******************************************************************************
 *
 * void *GncGetDataLastRead (void)
 *
 *  This routine does the following - 
 *            - return the last read data buffer
 *
 *  Returns -
 *       the last read data buffer
 *
 ******************************************************************************
 */
void *GncGetDataLastRead(void)
{
  return(g_data);
}

/******************************************************************************
 *
 * int GncReadAck (server)
 *      server     : server structure
 *
 *  This routine does the following - 
 *            - read the acknowledgment from the server
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
int GncReadAck(CDASSERVER *server)
{
  int rtn = CDAS_SUCCESS;
  int bytesData;
  int bytesRead;
  char buff[GsACKMESSAGELENGTH];
  char *buffPtr = &(buff[0]);
  int buffSize = GsACKMESSAGELENGTH;
  longWord version;

  if (server->state == OPEN) {
    if ((rtn = GncReadHeader(server, &bytesRead)) == CDAS_SUCCESS) {
      /* get number of bytes of data to read (total - (header + trailer)) */
      bytesData = g_header.length - (bytesRead + GsTRAILERLTH);
      if (bytesData <= buffSize) {
	/* set bytes in a different variable so we can return it to user */
	if ((rtn = GncReadData(server, buffPtr, bytesData, &bytesRead)) ==
	    CDAS_SUCCESS) {
	  rtn = GncReadTrailer(server, &bytesRead);
	  /* check for validity of returned information */
	  if (g_header.type == GsACK) {
	    version = (unsigned long )GsMessParam(buffPtr, GSLINT, 
					     (void **)&buffPtr);
	    rtn = (unsigned long )GsMessParam(buffPtr, GSLINT,  (void **)&buffPtr);
	  } else {
	    rtn = CDAS_MSG_TYPE_ERROR;
	    IkWarningSend("GncReadAck message type error, type is %d need %d", g_header.type, GsACK);
	  }
	}
      } else {
	/* buffer to read into is too small. */
	rtn = CDAS_BUFFER_TOO_SMALL;
	IkWarningSend("GncReadAck buffer too small, got %d bytes need %d", buffSize, bytesData);
      }
    }
  }
  return(rtn);
}

