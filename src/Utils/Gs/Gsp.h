/*-----------------------------------------------------------------------------*/
/*
  Gsp       private include file : do not use outside Gs(Gs?) src files
*/
/*-----------------------------------------------------------------------------*/
#ifndef __GsP__
#define __GsP__



/*-----------------------------------------------------------------------------*/
/* Misc defines. */

#define DSERVERQL	32          /* Queue Length */
#define MAXDC		250         /* Maximum number of Data Clients */
#define MTU 1600                    /* Max length of packets on Ethernet */
#define MAXPORT         1
#define DEFPARFILENAME "./GsParam.dat"  /* default parameter file */
#define DATAPORT  10400                /* default dataPort */
#define GSTYPE   10400
#define DEFSERVERNAME "Gs"
/***********************************************/

/*-----------------------------------------------------------------------------*/
/* function prototypes. */

extern void GsLoop(int dataPort,int data);
extern void * GsClientManager(void * );
extern void * GsClientOutputManager(void * );
extern void * GsServer(void * );
extern void GsEndOfLoop(void);



#endif





