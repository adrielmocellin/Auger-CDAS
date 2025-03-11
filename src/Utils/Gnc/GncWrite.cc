/******************************************************************************
 ******************************************************************************
 *
 * This file contains generic client write routines.  These routines are
 * called by clients who wish to send data to a CDAS server.
 * The following user callable routines are contained in this file :
 *
 *    GncWrite
 *
 *
 ******************************************************************************
 ******************************************************************************
 */

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>

#include "GncError.h"
#include "GncRead.h"
#include "GncWrite.h"
#include "IkC.h"
//#include "UiMessage.h"

/******************************************************************************
 *
 * int exactWrite (server, buffer)
 *      server   : server structure
 *      buff     : buffer of data to write
 *      buffSize : number of bytes to write
 *
 *  This routine does the following - 
 *            - write the specified bytes in the buffer
 *
 *  Returns -
 *       SUCCESS : number of bytes written
 *       FAILURE : ERROR_RTN
 *
 ******************************************************************************
 */
static int exactWrite(CDASSERVER *server, void *buff, int buffSize,
		      int *bytesWritten)
{
  int n, rtn = CDAS_SUCCESS, bytesLeft = buffSize;
  char *tmpB = (char *)buff;

  for (*bytesWritten = 0; bytesLeft > 0; bytesLeft -= n, tmpB += n, 
	 *bytesWritten += n) {
    if ((n = write(server->fd, tmpB, bytesLeft)) == -1) {
      /* error during write */
      rtn = CDAS_WRITE_ERROR;
      IkSevereSend("exactWrite wrote only %d bytes to %s on port %d, %d bytes left",
                    *bytesWritten, server->name, server->fd, bytesLeft); 
      break;
    } else if (n == 0) {
      /* unexpected end of file (TODO: not sure this is possible.  check */
      rtn = CDAS_WRITE_ERROR;
      IkSevereSend("exactWrite unexpected EOF on %s on port %d, %d bytes left",
                    server->name, server->fd, bytesLeft); 
      break;
    }
  }
  return(rtn);
}
/******************************************************************************
 *
 * int GncWriteHeader (, type)
 *      server : server structure
 *      type   : type of message
 *
 *  This routine does the following - 
 *            - write the header to the server
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
static int GncWriteHeader(CDASSERVER *server, longWord type, int dataSize,
			  int *bytesWritten)
{
  GsStdHeader header;

  header.length = htonl(GsSTDHEADERLTH + GsTRAILERLTH + dataSize);
  header.type = htonl(type);
  header.protocolVersion = htonl(1);
  header.hLength = htonl(GsSTDHEADERLTH);
  return(exactWrite(server, (void *)&header, GsSTDHEADERLTH, bytesWritten));
}

/******************************************************************************
 *
 * int GncWriteData (server, buffer, buffSize, bytesWritten)
 *      server   : server structure
 *      buffer   : buffer of data to write
 *      buffSize : number of bytes to write
 *      bytesWritten : number of bytes written
 *
 *  This routine does the following - 
 *            - write the information in the buffer to the server
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
static int GncWriteData(CDASSERVER *server, void *buff, int buffSize, 
			int *bytesWritten)
{
  int rtn = CDAS_SUCCESS;

  if (buffSize != 0) {
    rtn = exactWrite(server, buff, buffSize, bytesWritten);
  } else {
    *bytesWritten = 0;
  }
  return(rtn);
}

/******************************************************************************
 *
 * int GncWriteTrailer (server)
 *      server : server structure
 *
 *  This routine does the following - 
 *            - write the trailer to the socket
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
static int GncWriteTrailer(CDASSERVER *server, int *bytesWritten)
{
  GsTrailer trailer;

  trailer.id = htonl(GsMESSAGESENDMARKER);
  return(exactWrite(server, (void *)&trailer, GsTRAILERLTH, bytesWritten));
}

/******************************************************************************
 *
 * int GncWrite (server, buffer)
 *      server     : name of server to write to
 *      buffer     : buffer of data to write
 *
 *  This routine does the following - 
 *            - write the information in the buffer to the server
 *
 *  Returns -
 *       SUCCESS : CDAS_SUCCESS
 *       FAILURE : other
 *
 ******************************************************************************
 */
int GncWrite(CDASSERVER *server, void *buff, int buffSize, longWord type,
	     int *dbytesWritten)
{
  int rtn = CDAS_SUCCESS;
  int bytesWritten;

  *dbytesWritten = 0;
  if (server->state == OPEN) {
    if ((rtn = GncWriteHeader(server, type, buffSize, &bytesWritten)) == 
	CDAS_SUCCESS) {
      /* we want to return the number of data bytes written */
      if ((rtn = GncWriteData(server, buff, buffSize, dbytesWritten)) == 
	  CDAS_SUCCESS) {
	rtn = GncWriteTrailer(server, &bytesWritten);
      }
    }
  }
  return(rtn);
}
