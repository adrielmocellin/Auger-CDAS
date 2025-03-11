/******************************************************************************
 *
 *
 ******************************************************************************
 */

#ifndef __GncInit__
#define __GncInit__

#include "LkListP.h"
#include "UiErr.h"
#define NULLSTRING ""
#define PORTINIT -1

#define OPEN 1
#define CLOSED 2

typedef struct cdasserver {
  LkEntry LkEntryItems;
  int fd;
  char *name;
  int state;
  int watch;
  int lastWatched;    /* records the last time we looked for this server */
  int nextWatched;    /* records how many seconds later we should look again */
  int service;
  char *host;
  int port;
  int (*userInit)(struct cdasserver *server);
  int (*userProcess)(struct cdasserver *server);
  int deleteFlag;
  int initFlag;
} CDASSERVER;

int GncInit(const char *serverName, int service, char *serverAddr, 
	    int serverPort, int (*userInit)(CDASSERVER *srvr),
	    int (*userProcess)(CDASSERVER *server), CDASSERVER **server);
char *GncGetMem(const char *string);
CDASSERVER *GncInitServer(CDASSERVER *server, int fd, const char *name, 
			  int service, char *serverAddr, int serverPort,
			  int (*userInit)(CDASSERVER *server),
			  int (*userProcess)(CDASSERVER *server));

#endif
