/************************************************************************
 *
 * FILE:
 *       UiErr.h
 * 
 * DESCRIPTION: 
 *       This file includes definition of error messages and associated
 *       variables.
 *
 ***********************************************************************/

#ifndef _UIERR_H_
#define _UIERR_H_

/* CAREFUL!! When adding errors, you must add to the enum (the mnemonic for the
   error) and add the format of the error to the array g_error_messages. The
   enum elements specify the array position of the format message in the array.
   ============================================================================
*/
typedef enum errors { CDAS_INVALID = -1,
		      CDAS_SUCCESS = 0,
		      CDAS_SOCKET_CLOSE_ERROR,
		      CDAS_ADDR_ERROR,
		      CDAS_NO_SOCKET,
		      CDAS_CONNECT_ERROR,
		      CDAS_SELECT_ERROR,
		      CDAS_INPUTS_ERROR,
		      CDAS_READ_ERROR,
		      CDAS_TRAILER_BAD,
		      CDAS_WRITE_ERROR,
		      CDAS_MSG_TYPE_ERROR,
		      CDAS_NO_SERVER,
		      CDAS_MALLOC_ERROR,
		      CDAS_NOT_FOUND,
		      CDAS_BUFFER_TOO_SMALL,
		      CDAS_ARGS_BADSYNTAX,
		      CDAS_FILE_OPEN_ERROR,
		      CDAS_FILE_READ_ERROR,
		      CDAS_FILE_TOO_BIG_ERROR,
		      CDAS_TOO_MANY_ARGS,
		      CDAS_LISTEN_ERROR,
		      CDAS_BIND_ERROR,
		      CDAS_NOT_SUPPORTED,
		      CDAS_MESS_ADD_ERROR,
		      CDAS_INV_CATEGORY,
		      CDAS_CONNECT_CLOSED,
		      CDAS_BIT_TOO_BIG,
		      CDAS_TOO_MUCH_CLIENTS,
		      CDAS_NO_SUCH_SERVICE,
		      CDAS_UNSUPPORTED_VERSION,
		      CDAS_SERVICE_FULL,
		      CDAS_INVALID_ID,
		      CDAS_TOO_MUCH,
		      CDAS_PRIVILEGED,
		      CDAS_RESERVED_COMMAND,
		      CDAS_CLIENT_REG_ERROR,
		      CDAS_INVALID_DATA_BIT_SET,
		      CDAS_INVALID_TYPE,
		      CDAS_NO_DATA,
		      CDAS_FULL_REGISTRATION,
		      CDAS_INVALID_IKMSG_TYPE,
		      CDAS_NO_IKMSG_HANDLER,
		      CDAS_INVALID_IKMSG_FORMAT,
					CDAS_BAD_DATA,
		      CDAS_NERR             /*  this one must always be last */
} cdasErrors;

#ifdef UIERRMAIN  /* to avoid warning at c++ compile time */
/* And now the error messages (the numbers on the left are an aid in debugging,
   that is all) */
static const char *g_error_messages[(int )CDAS_NERR] = {
/* 00 */  "%s: Success.",
/* 01 */  "%s: Unable to close port %d connected to %s.",
/* 02 */  "%s: Unable to get the address of host %s.",
/* 03 */  "%s: Unable to get a free socket descriptor.",
/* 04 */  "%s: Unable to connect on port 0x%04X (%d) to %s.",
/* 05 */  "%s: Select error.",
/* 06 */  "%s: Not all select inputs processed, %d left.",
/* 07 */  "%s: Error reading from server %s on port %d.",
/* 08 */  "%s: Trailer incorrect (0x%08X).",
/* 09 */  "%s: Error writing %d bytes to server %s on port %d.",
/* 10 */  "%s: Unexpected return message type, received - %d, expected - %d.",
/* 11 */  "%s: No server matching %s found.",
/* 12 */  "%s: Unable to malloc %d bytes.",
/* 13 */  "%s: Requested item (%s) was not found.",
/* 14 */  "%s: Buffer (%d bytes) not large enough (need %d bytes).",
/* 15 */  "%s: Argument Parsing Error: %s %s.",
/* 16 */  "%s: Error opening file %s.",
/* 17 */  "%s: Error reading file %s.",
/* 18 */  "%s: File, %s, is bigger than the maximum size - %d.",
/* 19 */  "%s: Too many arguments to process (max is %d).",
/* 20 */  "%s: Error while trying to listen on port %d.",
/* 21 */  "%s: Error while trying to bind on port %d.",
/* 22 */  "%s: The %s (%s) is not supported. %s",
/* 23 */  "%s: Error adding %s %d",
/* 24 */  "%s: Invalid data category - %d",
/* 25 */  "%s: The connection has been closed (fd: %d)",
/* 26 */  "%s: The specified bit (%d) must be less than %d.",
/* 27 */  "%s: Too many clients already registered ",
/* 28 */  "%s: Server does not offer service for type %d",
/* 29 */  "%s: Server does not support version %d",
/* 30 */  "%s: Service %d has already too many clients",
/* 31 */  "%s: Id %x is invalid for a %s ",
/* 32 */  "%s: There is too much %s on %s %d",
/* 33 */  "%s: Only one client of type %d at a time, please",
/* 34 */  "%s: You are not allowed to issue this command : %s",
/* 36 */  "%s: Could not register client for %s service.",
/* 37 */  "%s: Invalid data bit (%d), max is %d.",
/* 38 */  "%s: Invalid producer type (%d).",
/* 38 */  "%s: No Data in message. (Error Code is %d)",
/* 39 */  "%s: Too many categories (%d) for registration, maximum is %d",
/* 40 */  "%s: Unsupported Ik message from - %s, to - %s, of type %d",
/* 41 */  "%s: No message handler for Ik message from - %s, to - %s, of type %d",
/* 42 */  "%s: Invalid Ik message format from - %s, to - %s, of type %d"
};
#endif /* UIERRMAIN */

#define CDAS_FATAL           "ERROR"
#define CDAS_WARNING         "WARNING"
#define CDAS_INFORMATIONAL   "INFORMATIONAL"
#define CDAS_INFORMATIONAL_LEN 13

#define CDAS_UNKNOWN_TEXT    "(unknown)"

/*
 * Declarations of public variables.
 */
#ifndef UIERRMAIN
extern int CDAS_ERROR;
extern int g_line;
extern char *g_file;
#endif /* UIERRMAIN */


/*
 * Declaration of public functions.
 */
void UiErrClear(void);
//char *UiErrAdd (const int a_error_id, ...);
char *UiErrFormat (const int a_error_index, ...);
void UiErrOutput (void);
void UiErrBackup (void);
char *UiErrGet (void);
char *UiTxtFormat (const char *a_error_sev, const char *text);

#define UiErrPlace() g_line=__LINE__; g_file=(char *)__FILE__;

#define UiMallocPrint(func, bytes) printf("%s (line %d): %d bytes", func, __LINE__, bytes);




#endif /* _UiERR_H_ */




