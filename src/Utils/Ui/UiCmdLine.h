/***********************************************************************/
/* UiCmdLine.h                                                         */
/*   Handles Global variables and routines declarations  for           */
/*   for "standard" command line parsing.                              */
/*                                                                     */
/***********************************************************************/

#ifndef __UICMDLINE__
#define __UICMDLINE__

extern char *TaskName;
extern char *TaskIkSuffix;
extern char *FdAddr;
extern int FdPort;
extern char *MoAddr;
extern int MoPort;
extern char *PmAddr;
extern int PmPort;

int UiParseCommandLine(int argc, char **argv);


#endif







