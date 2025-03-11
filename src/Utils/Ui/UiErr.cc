/************************************************************************
 *
 * FILE:
 *       UiErr.c
 * 
 * DESCRIPTION: 
 *       This module contains the error handling functions.
 *
 ***********************************************************************/

/* standard include files */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define CHECKPRINT(a) \
if ((a) > G_BUFSIZE) {fprintf(stderr,"Exiting on memory overwrite in errFormat\n"); exit(1);}

#define UIERRMAIN 1

/* cdas specific include files */
#include "UiErr.h"

/*
 * Definition of public variables.
 */

/* the following three cannot be declared static as they are declared extern
   in the UiErr.h file */
int CDAS_ERROR = CDAS_SUCCESS;    /* start out with no errors */
int g_line = 0;
char *g_file = NULL;


static int g_cdasErrno = 0;

/*
 * Declaration of private functions.
 */


/*
 * Definition of global variables.
 */
#define G_BUFSIZE 2000
#define G_ERROR_INIT -1
#define G_ERROR_BUF_MAX 100

static int g_buf_counter = G_ERROR_INIT;   /* pointer to current message */
static int g_buf_start = G_ERROR_INIT;     /* pointer to oldest message */

static char *g_error_buf[G_ERROR_BUF_MAX];

/*
 * Definition of public functions.
 */

/*-----------------------------------------------------------------------
 * addLineInfo
 *
 * add information on where the error occurred to the error message
 *
 * Input : error buffer
 * Output: none
 * Return: none
 */
static int addLineInfo (char *buf)
{
  int printed = 0;

  /* Check if we need to add error location information to output too */
  if (g_line) {
   printed =  sprintf(buf, "(line number %d in file %s)\n", g_line, g_file);
    g_line = 0;          /* reset so next time do not give false info */
  }
  return printed;
}

static char *errFormat(enum errors error_index, va_list args)
{
  static char buf[G_BUFSIZE]; 
  int printed = 0;

  /* Initialize */
  buf[0] = '\0';

  if ( (error_index < CDAS_NERR) && (error_index > CDAS_INVALID)) {
    /* format the error and put it in the error buf */
    printed += vsprintf(buf, g_error_messages[error_index], args);
    CHECKPRINT(printed);
    /* add in errno information if there is any */
    if (g_cdasErrno != 0) {
      printed += sprintf(&buf[printed], " (%s)\n", strerror(g_cdasErrno));
      CHECKPRINT(printed);
      g_cdasErrno = 0;
    }
    /* add in line information */
    printed += addLineInfo(&buf[printed]);
    CHECKPRINT(printed);

    if (strncmp(buf, CDAS_INFORMATIONAL, CDAS_INFORMATIONAL_LEN)) {
      CDAS_ERROR = error_index;
    }
  } else {
    /* This was an invalid error message request */
    printed += sprintf(buf, "ERROR: Invalid error message number %d.", 
		       error_index);
    CHECKPRINT(printed);
    printed += addLineInfo(&buf[printed]);
    CHECKPRINT(printed);
    CDAS_ERROR = CDAS_INVALID;
  }
  return(buf);
}

/*-----------------------------------------------------------------------
 * UiTxtFormat
 *
 * Format the passed in text to look like an official Ui error message.
 *
 * Input : severity, text
 * Output: none
 * Return: static buffer with formatted error message
 */
char *UiTxtFormat (const char *a_error_sev, const char *text)
{
  static char buf[G_BUFSIZE];
  int printed = 0;

  /* Initialize */
  buf[0] = '\0';

  /* format the error and put it in the error buf */
  printed += sprintf(buf, "%s: %s", a_error_sev, text);
  
  /* add in errno information if there is any */
  if (g_cdasErrno != 0) {
   printed += sprintf(&buf[printed], "(%s)", strerror(g_cdasErrno));
   CHECKPRINT(printed);
   g_cdasErrno = 0;
  }
  /* add in line information */
  printed += addLineInfo(&buf[printed]);
  CHECKPRINT(printed);
  
  return(buf);
}

/*-----------------------------------------------------------------------
 * UiErrFormat
 *
 * Return the requested message formatted with the input parameters. Do not
 *   store the message in the error buffer.
 *
 * Input : error message number, error severity and associated information
 * Output: none
 * Return: static buffer with formatted error message
 */
char *UiErrFormat (enum errors a_error_index, ...)
{
  va_list args;
  char *msgBufPtr;

  /* save record of error that occurred. will use later */
  g_cdasErrno = errno;
  va_start(args, a_error_index);
  msgBufPtr = errFormat(a_error_index, args);
  va_end(args);
  return(msgBufPtr);
}
/*-----------------------------------------------------------------------
 * UiErrAdd
 *
 * Add the requested message to the error buf.  If the error buf is full,
 * overwrite the oldest error message.
 *
 * Input : error message number, error severity and associated information
 * Output: none
 * Return: static buffer with error message
 */
#if 0
char *UiErrAdd (enum errors a_error_index, ...)
{
  va_list args;
  char *tmpBufPtr, *msgBufPtr;
  static char buf[G_BUFSIZE];

  /* save record of error that occurred. will use later */
  g_cdasErrno = errno;
  va_start(args, a_error_index);
  msgBufPtr = errFormat(a_error_index, args);
  va_end(args);

  /* Malloc space for the message so we can save it and copy it in. */
  tmpBufPtr = (char *)malloc(strlen(msgBufPtr) + 1);  /* leave room for \0 too */
  (void) strcpy(tmpBufPtr, msgBufPtr);

  /* Add the error message to the error buf.  If we are at the bottom of
     the buf, go back to the top and overwrite the oldest message */
  if (++g_buf_counter == G_ERROR_BUF_MAX) {
    /* we have reached the end of the buf, go to the start */
    g_buf_counter = 0;
  }

  if (g_buf_counter == g_buf_start) {
    /* the buf is full, we must delete the oldest message to make room */
    free(g_error_buf[g_buf_start++]);
    if (g_buf_start == G_ERROR_BUF_MAX) {
      /* we have reached the end of the buf, go to the start */
      g_buf_start = 0;
    }
  }

  /* check if this is our first message*/
  if (g_buf_start == G_ERROR_INIT) {
    /* yes, move to start of buffer */
    g_buf_start = 0;
  }

  g_error_buf[g_buf_counter] = tmpBufPtr;

  return(msgBufPtr);
}
#endif
/*-----------------------------------------------------------------------
 * UiErrBackup
 *
 * Backup over the last error added to the buffer
 *
 * Input : none
 * Output: none
 * Return: none
 */
void UiErrBackup (void)
{
  /* only do something if there are messages in the buffer */
  if (g_buf_start != G_ERROR_INIT) {
    /* free the current message */
    free(g_error_buf[g_buf_counter]);
    g_error_buf[g_buf_counter] = NULL;

    /* adjust the current message counter */
    --g_buf_counter;
    if (g_buf_counter < 0) {
      /* if the oldest msg is the first one, then that was the only one */
      if (g_buf_start == 0) {
	g_buf_start = G_ERROR_INIT;
	g_buf_counter = G_ERROR_INIT;
      } else {
	/* reset to the last entry in the buffer */
	g_buf_counter = G_ERROR_BUF_MAX - 1;
      }
    }
  }

  CDAS_ERROR = CDAS_SUCCESS;
}
/*-----------------------------------------------------------------------
 * UiErrClear
 *
 * Clear out the error buf.  All messages currently in the buf are lost.
 *
 * Input : none
 * Output: none
 * Return: none
 */
void UiErrClear (void)
{
  int i;

  /* only do something if there are messages in the buffer */
  if (g_buf_start != G_ERROR_INIT) {
    /* free all of the error message bufs */
    for (i = g_buf_start; i != g_buf_counter; ++i) {
      if (i < G_ERROR_BUF_MAX) {
	free(g_error_buf[i]);
      } else {
	i = G_ERROR_INIT;
      }
    }

    /* catch the last one we missed */
    free(g_error_buf[g_buf_counter]);

    /* Reset */
    g_buf_counter = G_ERROR_INIT;

    /* Reset */
    g_buf_start = G_ERROR_INIT;
  }

  CDAS_ERROR = CDAS_SUCCESS;
}

/*-----------------------------------------------------------------------
 * UiErrOutput
 *
 * Output the error buf to stderr if it is not empty.
 *
 * Input : none
 * Output: none
 * Return: none
 */
void UiErrOutput (void)
{
  int i;

  /* only do something if there are messages in the buffer */
  if (g_buf_start != G_ERROR_INIT) {
    for (i = g_buf_start; i != g_buf_counter; ++i) {
      if (i < G_ERROR_BUF_MAX) {
	if (g_error_buf[i]) {
	  (void) fputs(g_error_buf[i], stderr);
	}
      } else {
	i = G_ERROR_INIT;
      }
    }

    /* catch the last one we missed */
    if (g_error_buf[g_buf_counter]) {
      (void) fputs(g_error_buf[g_buf_counter], stderr);
    }
  }
}

/*-----------------------------------------------------------------------
 * UiErrGet
 *
 * Return all errors currently in the error buffer to the user in a single
 * string.  messages are added to the string newest first.
 *
 * Input : none
 * Output: none
 * Return: a string containing the formatted error messages.
 */
char *UiErrGet (void)
{
  int i, len = 0;
  int msglens[G_ERROR_BUF_MAX];
  char *buf, *bufPtr;

  /* only do something if there are messages in the buffer */
  if (g_buf_start != G_ERROR_INIT) {
    /* first count how much space we will need */
    for (i = g_buf_start; i != g_buf_counter; ++i) {
      if (i < G_ERROR_BUF_MAX) {
	if (g_error_buf[i]) {
	  msglens[i] = strlen(g_error_buf[i]);
	  len += msglens[i];
	}
      } else {
	i = G_ERROR_INIT;
      }
    }
    /* catch the last one we missed */
    if (g_error_buf[g_buf_counter]) {
      msglens[i] = strlen(g_error_buf[g_buf_counter]);
      len += msglens[g_buf_counter];
    }

    /* malloc a buffer to hold all of this */
    if ((buf = (char *)malloc(len)) != NULL) {
      memset(buf, '\0', len);
      bufPtr = buf;
      for (i = g_buf_start; i != g_buf_counter; ++i) {
	if (i < G_ERROR_BUF_MAX) {
	  if (g_error_buf[i]) {
	    memcpy(bufPtr, g_error_buf[i], msglens[i]);
	    bufPtr += msglens[i];
	  }
	} else {
	  i = G_ERROR_INIT;
	}
      }
      /* catch the last one we missed */
      if (g_error_buf[g_buf_counter]) {
	memcpy(bufPtr, g_error_buf[g_buf_counter], msglens[g_buf_counter]);
      }
    } else {
      UiErrPlace(); //UiErrAdd(CDAS_MALLOC_ERROR, CDAS_WARNING, len);
    }
  }
  return(buf);
}
