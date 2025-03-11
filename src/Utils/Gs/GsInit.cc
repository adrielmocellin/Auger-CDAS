/*--------------------------------------------------------------------------------*/
/*
  GsInit.c
*/
/*--------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <GsServer.h>
#include <Gsp.h>


extern int GsServicesInit(void);


/*--------------------------------------------------------------------------------*/
void GsInitTCPIP(int dataPort, int queueLen, int *portfd)
{
  struct sockaddr_in
    hsta;

  /* opening port for consumers */
  if((*portfd=socket(AF_INET,SOCK_STREAM,0))==(-1)) 
  {
    GsNotifyIk(TOLOG,IK_FATAL,
	       "socket() error : %s",strerror(errno));
    GsTerminate(EXIT_FAILURE);
  }

  hsta.sin_addr.s_addr=INADDR_ANY;
  hsta.sin_family=AF_INET;
  hsta.sin_port=htons(dataPort);

  if((bind(*portfd,(struct sockaddr *)&hsta,sizeof(struct sockaddr_in)))==(-1))
  {
    GsNotifyIk(TOLOG,IK_WARNING,
	     "bind() error for port 0x%04X (%d) : %s",
	     dataPort,dataPort,strerror(errno));
    GsNotifyIk(TOLOG,IK_FATAL,
	       "no data port available");
    GsTerminate(EXIT_FAILURE);
  }
	
  /* allow connections on dataPort */
  if((listen(*portfd,queueLen))==(-1)) 
  {
    GsNotifyIk(TOLOG,IK_FATAL,
	       "listen() error : %s",strerror(errno));
    GsTerminate(EXIT_FAILURE);
  }

}

/*--------------------------------------------------------------------------------*/
