#ifndef __PmLog__
#define __PmLog__

#include <sys/stat.h> 
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

void PmLogWriteOut (char*,char *);
void PmLogPrint (char *, char*,...);
void PmLogPrintErrno ();

// Error switches
#define ERROR_SWITCH_SHUTDOWN(_socket_) \
	{\
	  switch (errno)\
	    {\
	    case EBADF:\
	      IkWarningSend ("fd=%d is not a valid file descriptor.",_socket_);\
	      break;\
	    case ENOTSOCK:\
	      IkWarningSend ("fd=%d is a file, not a socket.",_socket_);\
	      break;\
	    case ENOTCONN:\
	      IkWarningSend ("The specified socket is not connected.");\
	      break;\
	    default:\
	      IkWarningSend ("shutdown(2) unknown error");\
	    }\
	}

#define ERROR_SWITCH_FSYNC(_socket_) \
	{\
	  switch (errno)\
	    {\
	    case EROFS:\
	    case EINVAL:\
	      IkWarningSend ("fd=%d is bound to a special file which does not support synchronization.",_socket_);\
	      break;\
	    case EBADF:\
	      IkWarningSend ("fd=%d is not a valid file descriptor open for writing.",_socket_);\
	      break;\
	    case EIO:\
	      IkWarningSend ("An error occurred during synchronization.");\
	      break;\
	    default:\
	      IkWarningSend ("fsync(2) unknown error");\
	    }\
	}


#endif
