/******************************************************************************
 *
 *
 ******************************************************************************
 */

#ifndef __GncWatch__
#define __GncWatch__

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define BADFD -1

#define MINTIMEINCREMENT 5
#define MAXTIMEINCREMENT 300

int GncWatch(const char *serverName, int fd, fd_set rfds);
CDASSERVER *GncSetWatch(const char *serverName, int service, char *serverAddr, 
			int serverPort, int (*userInit)(CDASSERVER *srvr),
			int (*userProcess)(CDASSERVER *srvr));


#endif
