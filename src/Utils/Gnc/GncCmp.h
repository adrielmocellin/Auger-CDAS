/******************************************************************************
 *
 *
 ******************************************************************************
 */

#ifndef __GNCCMP__
#define __GNCCMP__

int GncCmpFd(const void * const fd, const void * const server);
int GncCmpName(const void * const name, const void * const server);
CDASSERVER *GncGetServerByName(const char *serverName);
CDASSERVER *GncGetServerByFd(int fd);

#endif
