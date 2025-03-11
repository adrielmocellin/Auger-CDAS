#include "PmConst.h"
#include "PmLog.h"
#include "IkC.h"



/*-----------------------------------------------------------------------*/ 
void PmLogWriteOut (char* line, char *iktype)
/*-----------------------------------------------------------------------*/ 
{
  InternalLog(line, iktype);
}
/*-----------------------------------------------------------------------*/ 


/*-----------------------------------------------------------------------*/ 
/* Some comments between the impossible bugs...                          */
void PmLogPrint (char *iktype, char* format,...)
/*-----------------------------------------------------------------------*/ 
{
  char line[1500];
  va_list args;
 
  line[0] = 0;
  va_start (args, format);
  vsprintf (line, format, args);
  va_end (args);
  PmLogWriteOut (line, iktype);
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void PmLogPrintErrno ()
/*-----------------------------------------------------------------------*/ 
{
  PmLogPrint (IKERROR,"errno : ");

  switch (errno)
  {
  case EHOSTUNREACH :
    PmLogPrint (IKERROR,"EHOSTUNREACH  no route to host");
    break;
  case EBADF:
    PmLogPrint (IKERROR,"EBADF  fd is not a valid file descriptor or is not open for writing.");
     break;
  case EINVAL:
    PmLogPrint (IKERROR,"EINVAL  fd is attached to an object which is unsuitable for read or write.");
    break;
  case EFAULT:
    PmLogPrint (IKERROR,"EFAULT  buf is outside your accessible address space.");
    break;
  case EPIPE:
    PmLogPrint (IKERROR,"EPIPE  fd is connected to a pipe or socket whose reading");
    PmLogPrint (IKERROR,"end is closed. When this happens the writing pro­");
    PmLogPrint (IKERROR,"cess will receive a SIGPIPE signal; if it  catches,");
    PmLogPrint (IKERROR,"blocks or ignores this the error EPIPE is returned.");
    break;
  case EAGAIN:
    PmLogPrint (IKERROR,"EAGAIN  Non-blocking I/O has been selected using O_NONBLOCK");
    PmLogPrint (IKERROR,"and there was no room in the pipe or socket con­");
    PmLogPrint (IKERROR,"nected to fd to write the data immediately.");
    break;
  case EINTR:
    PmLogPrint (IKERROR,"EINTR  The call was interrupted by a signal before any data was written.");
    break;
  case ENOSPC :
    PmLogPrint (IKERROR,"ENOSPC  The device containing the file referred to by fd has no room for the data.");
    break;
  case EIO :
    PmLogPrint (IKERROR,"EIO  A low-level I/O error occurred while modifying the inode.");
    break;
  case EACCES :
    PmLogPrint (IKERROR,"EACCES  The user tried to connect to a broadcast address");
    PmLogPrint (IKERROR,"without having the socket broadcast flag  enabled.");
    break;
  case ENOTSOCK :
    PmLogPrint (IKERROR,"ENOTSOCK  The descriptor is not associated with a socket.");
    break;
  case EISCONN :
    PmLogPrint (IKERROR,"EISCONN  The socket is already connected.");
    break;
  case ECONNREFUSED :
    PmLogPrint (IKERROR,"ECONNREFUSED  Connection refused at server.");
    break;
  case ETIMEDOUT :
    PmLogPrint (IKERROR,"ETIMEDOUT  Timeout while attempting connection.");
    break;
  case ENETUNREACH :
    PmLogPrint (IKERROR,"ENETUNREACH  Network is unreachable.");
    break;
  case EADDRINUSE :
    PmLogPrint (IKERROR,"EADDRINUSE  Address is already in use.");
    break;
  case EINPROGRESS :
    PmLogPrint (IKERROR,"EINPROGRESS  The socket is non-blocking and the connection cannot");
    PmLogPrint (IKERROR,"not  be  completed immediately.  It is possible to");
    PmLogPrint (IKERROR,"select(2) or poll(2) for completion  by  selecting");
    PmLogPrint (IKERROR,"the  socket  for  writing.  After select indicates");
    PmLogPrint (IKERROR,"writability,  use  getsockopt(2)   to   read   the");
    PmLogPrint (IKERROR,"SO_ERROR  option  at level SOL_SOCKET to determine");
    PmLogPrint (IKERROR,"whether connect completed  successfully  (SO_ERROR");
    PmLogPrint (IKERROR,"is zero) or unsuccessfully (SO_ERROR is one of the");
    PmLogPrint (IKERROR,"usual error codes  listed  above,  explaining  the");
    PmLogPrint (IKERROR,"reason for the failure).");
    break;
  case EALREADY :
    PmLogPrint (IKERROR,"EALREADY  The socket is non-blocking and a previous connec­");
    PmLogPrint (IKERROR,"tion attempt has not yet been completed.");
    break;
  case EAFNOSUPPORT :
    PmLogPrint (IKERROR,"EAFNOSUPPORT  The passed address didn't have the correct address");
    PmLogPrint (IKERROR,"family in its sa_family field.");
    break;
  default :
    PmLogPrint (IKERROR,"errno = %i", errno);
    break;
  }

}
/*-----------------------------------------------------------------------*/ 

