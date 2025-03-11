/******************************************************************************
 ******************************************************************************
 *
 * This file contains various comparison routines for finding the server
 * structure that matches the datum.
 * The following user callable routines are contained in this file :
 *
 *    GncCmpFd
 *
 *
 ******************************************************************************
 ******************************************************************************
 */

#include <string.h>

#include "GncInit.h"
#include "LkList.h"
//#include "UiMessage.h"
#include "UiLk.h"

extern LkList g_servers;

/******************************************************************************
 *
 * int GncCmpFd
 *
 *  This routine finds the server that corresponds to the specified socket
 *  descriptor.
 *
 *  Returns -
 *       SUCCESS : 0
 *       FAILURE : 1
 *
 ******************************************************************************
 */
int GncCmpFd(const void * const fd, const void * const server)
{
  int *l_fd = (int *)fd;
  CDASSERVER *l_server = (CDASSERVER *)server;

  if (*l_fd == l_server->fd) {
    /* we found a match */
    return(0);
  } else {
    return(1);
  }
}

/******************************************************************************
 *
 * int GncCmpName
 *
 *  This routine finds the server that corresponds to the specified name.
 *
 *  Returns -
 *       SUCCESS : 0
 *       FAILURE : 1
 *
 ******************************************************************************
 */
int GncCmpName(const void * const name, const void * const server)
{
  char *l_name = (char *)name;
  CDASSERVER *l_server = (CDASSERVER *)server;

  return(strcmp(l_name, l_server->name));
}

/******************************************************************************
 *
 * CDASSERVER *GncGetServerByName(server)
 *      server : name of CDAS server to connect to
 *
 *  This routine does the following - 
 *            - returns the first server whose name is the same as the passed
 *                  in name
 *
 *  Returns -
 *       SUCCESS : server structure
 *       FAILURE : NULL
 *
 ******************************************************************************
 */
CDASSERVER *GncGetServerByName(const char *serverName)
{
  /* find the server that corresponds with this name */
  LkEntry entryMatch = (LkEntry )UiLkFind(g_servers, serverName, GncCmpName);

  if (entryMatch != NULL) {
    return((CDASSERVER *)entryMatch);
  } else {
    return(NULL);
  }
}

/******************************************************************************
 *
 * CDASSERVER *GncGetServerByFd(server)
 *      server : name of CDAS server to connect to
 *
 *  This routine does the following - 
 *            - returns the server whose fd is the same as the passed
 *                  in fd
 *
 *  Returns -
 *       SUCCESS : server structure
 *       FAILURE : NULL
 *
 ******************************************************************************
 */
CDASSERVER *GncGetServerByFd(int fd)
{
  /* find the server that corresponds with this name */
  LkEntry entryMatch = (LkEntry )UiLkFind(g_servers, &fd, GncCmpFd);

  if (entryMatch != NULL) {
    return((CDASSERVER *)entryMatch);
  } else {
    return(NULL);
  }
}


