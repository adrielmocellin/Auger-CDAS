/*
 * UiArgs.c - Contains user-callable routine to parse command line
 *            options. Heavily inspired by the Tk command line parser.
 *
 * Definitions:
 * Switches              : command line arguments preceeded by a '-'. May or
 *                         may not take an argument. Example: 
 *                         -toggle           # No argument
 *                         -debugLevel 10    # Needs an extra argument
 * positional parameters : command line arguments where position matters. Are
 *                         NOT preceeded by a '-'. 
 *
 * Public Functions:
 *
 * UiParseArgs()  - Parse command line options
 * UiGetUsage()   - Get the usage string for a command
 * UiGetArgInfo() - Get the ArgInfo string for a command
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "UiArgs.h"
#include "UiErr.h"

/*
 * Default table of argument descriptors.  These are normally available
 * in every application.
 */

static UiArgsInfo defaultTable[] = {
    {"-help",	UI_ARGS_HELP,	NULL,	NULL,
	"Print summary of command-line options and abort"},
    {NULL,	UI_ARGS_END,	NULL,	NULL, (char *) NULL}
};

/*
 * Forward declarations for procedures defined in this file:
 */

static int   UiPass1(int *, char **, UiArgsInfo *, int);
static int   UiPass2(int *, char **, UiArgsInfo *, int);
static int   assign_defaults(UiArgsInfo *);
static int   UiNeedHelp(const int, char **, const UiArgsInfo *, const int);

/*
 * There is one global variable, g_Specified_args. This variable is a pointer
 * to a string of all options/parameters that appear on the command line.
 */
#define ARGSIZ  (BUFSIZ/2)
#define LINESIZE 256
#define NUMLINES 200
#define TOTALBYTES LINESIZE*NUMLINES
#define NUMARGS NUMLINES*2
static char g_Specified_args[ARGSIZ];
static char g_file_lines[TOTALBYTES];
static char *g_argv[NUMARGS];

/* return 1 if the passed character is a double/single quote character, else
   return 0. */
static int isquote(int val)
{
  if ((val == '\"') || (val == '\'')) {
    /* this is a quote */
    return(1);
  }
  return(0);
}

/* parse the buffer into an argc and argv */
static int parseBuf(char *buf, int *argc, char **argv)
{
  int len = strlen(buf), i = 0, j = 0, isq;
  int rtn = CDAS_SUCCESS;

  *argc = 0;   /* nothing yet */
  while ((i < len) && (j < NUMARGS)) {
    if (! isspace((int )buf[i])) {
      /* we found the beginning of a word */
      if ((isq = isquote((int )buf[i])) == 1) {    /* check if it's a quote */
	/* we do not want the string to include quotes, so skip them */
	++i;
      }
      argv[j++] = &buf[i];
      ++*argc;
      do {
	if (isspace((int )buf[i]) && (isq == 0)) {
	  /* we found the next space that is not contained within a pair of
	     quotes.  it's the end of the word as we know it. */
	  buf[i++] = '\0';
	  break;
	} else if (isquote((int )buf[i])) {
	  if (isq == 1) {
	    /* this was the end of a quoted section */
	    isq = 0;
	    /* this is also the end of the word, do not include the quotes */
	    buf[i] = '\0';
	  } else {
	    /* this is the beginning of a quoted section */
	    isq = 1;
	  }
	}
      } while (i++ <= len);
    } else {
      ++i;
    }
  }
  /* see if we finished because we filled the buffer or because we parsed
     everything */
  if (j >= NUMARGS) {
    /* we overran the buffer */
    rtn = CDAS_TOO_MANY_ARGS;
    UiErrPlace(); //UiErrAdd(rtn, CDAS_WARNING, NUMARGS);
  }
  return(rtn);
}

/*
 * ROUTINE:
 *    UiParseFile
 *
 *      Process a file with keyword/value pairs according to a table of
 *      expected command-line options.
 *
 * CALL:
 *   (int ) UiParseFile(char *fileName, UiArgsInfo *argTable, int flags)
 *            filename - Name of file containing keyword/value pairs
 *            argTable - Table of expected command-line options
 *            flags    - Flags to modify the behavior while parsing
 *                       command line options.
 *
 * DESCRIPTION:
 *   Processes a file with keyword/value pairs according to a table of 
 *   expected command line options.  The file is of the form -
 *             -opt1 val1
 *             -opt2 val2
 *             param1
 *             -opt3
 *             param4
 *  where the following rules apply -
 *       o only 1 option/value or parameter per line
 *       o all option names start with a '-'
 *       o option names may not contain whitespace
 *       o strings with embedded whitespace must be enclosed with parenthesis
 *           (e.g. "the larch")
 *
 * RETURNS:
 *    CDAS_SUCCESS        : if command-line options parsed successfully
 *    CDAS_ARGS_GIVEHELP  : if user gave option to request help for command
 *    CDAS_ARGS_BADSYNTAX : on error. Reason for error is put in error buffer
 */
int UiParseFile(char *fileName,	       /* Name of file containing arguments */
		UiArgsInfo *argTable,  /* Array of option descriptions */
		int flags	       /* Or'ed combination of various flag
				        * bits, such as UI_ARGS_NO_DEFAULTS. */
		)
{
  int ret, argc, total = 0, len;
  char *bufPtr, *rtn;
  FILE *fd;
  int l_flags = flags | UI_ARGS_DONT_SKIP_FIRST_ARG;

  g_file_lines[0] = '\0';      /* wipe out the past */
  bufPtr = g_file_lines;

  if ((fileName != NULL ) && (strcmp(fileName , "-help") != 0)) {
    /* open the file for reading */
    if ((fd = fopen(fileName, "r")) != (FILE *)NULL) {
      /* read in a line at a time and put it in g_file_lines. remove any
	 carriage returns */
      do {	
	if ((rtn = fgets(bufPtr, LINESIZE, fd)) != (char *)NULL) {
	  /* got another line, change the <cr> to be a NULL and point to the
	     next character */
	  len = strlen(bufPtr);
	  total += len;
	  bufPtr += len;        /* point beyond the string */
	}
      } while ((rtn != (char *)NULL) && (total+LINESIZE < TOTALBYTES));
      
      /* either we reached the end of the file or there was an error. */
      if (feof(fd) != 0) {
	/* the whole file has been read, turn the read in bytes into an argc
	   and argv. skip the first position in argv as parse args will 
	   assume it is the calling program */
	if ((ret = parseBuf(g_file_lines, &argc, g_argv)) == 
	    CDAS_SUCCESS) {
	  int i;
	  ret = UiParseArgs(&argc, g_argv, argTable, l_flags);
	}
      } else {
	/* is it an error on reading or that we filled up g_file_lines */
	if ((total+LINESIZE) >= TOTALBYTES) {
	  /* we filled up g_file_lines */
	  ret = CDAS_FILE_TOO_BIG_ERROR;
	  UiErrPlace(); //UiErrAdd(ret, CDAS_FATAL, fileName, TOTALBYTES);
	} else {
	  /* read error */
	  ret = CDAS_FILE_READ_ERROR;
	  UiErrPlace(); //UiErrAdd(ret, CDAS_FATAL, fileName);
	}
      }
      fclose(fd);
    } else {
      /* open error */
      ret = CDAS_FILE_OPEN_ERROR;
      UiErrPlace(); //UiErrAdd(ret, CDAS_FATAL, fileName);
    }
  } else {
    /* stimulate the output of help */
    ret = CDAS_INVALID;
  }
  return(ret);
}

/*
 * ROUTINE:
 *    UiParseArgs
 *
 *      Process an argv array according to a table of expected
 *      command-line options.  See the manual page for more details.
 *
 * CALL:
 *   (int ) UiParseArgs(int *argcPtr, char **argv, 
 *                      UiArgsInfo *argTable, int flags)
 *            argcPtr  - Pointer to the argument count address
 *            argv     - Command line argument array
 *            argTable - Table of expected command-line options
 *            flags    - Flags to modify the behavior while parsing
 *                       command line options.
 *    Note: parameters marked by # will be changed as side effects.
 *
 * DESCRIPTION:
 *   Processes an argv array according to a table of expected command
 *   line options. 
 *
 * RETURNS:
 *    CDAS_SUCCESS        : if command-line options parsed successfully
 *    CDAS_ARGS_GIVEHELP  : if user gave option to request help for command
 *    CDAS_ARGS_BADSYNTAX : on error. Reason for error is put in error buffer
 */
int UiParseArgs(
    int *argcPtr,		/* Number of arguments in argv.  Modified
				 * to hold # args left in argv at end. */
    char **argv,		/* Array of arguments.  Modified to hold
				 * those that couldn't be processed here. */
    UiArgsInfo *argTable,	/* Array of option descriptions */
    int flags			/* Or'ed combination of various flag bits,
				 * such as UI_ARGS_NO_DEFAULTS. */
    )
{
   int retValue, length;
   register UiArgsInfo *ptr;
   int allowed_left;
   /*
    * Populate g_Specified_args, the buffer that holds all recognized command
    * line options and  parameters. We want to make sure that we do not
    * overflow it. So lets count all characters in the options/parameters
    * specified in the argument table. Their count should be <= ARGSIZ - 1 
    * (since g_Specified_args is dimensioned for ARGSIZ)
    */
   for (length = 0, ptr = argTable; ptr->type != UI_ARGS_END; ptr++)
        if (ptr->key)
            length += strlen(ptr->key) + 1;

   if (length >= ARGSIZ)  {
       printf("ERROR: Internal buffer overflow in UiParseArgs().  "
              "Please notify Ui maintainer!");
       /*
        * If this message is encountered, increase size of g_Specified_args
        * and recompile.
        */
       abort();
   }

   /*
    * If the user needs help for the command, bail out right away. We do not
    * want to modify the argument table, if the user simply needs help.
    */
   if (UiNeedHelp(*argcPtr, argv, argTable, flags))
       return CDAS_ARGS_GIVEHELP;

   g_Specified_args[0] = (char )0;

    /*
     * Pass 1 gets all the switches from argTable[]. Switches are defined
     * as tokens on the command line that are preceeded by a '-' and
     * may or may not use the next token as an argument. Examples:
     *
     * -toggle          # Option that does not have any argument
     * -name my.file    # Option that takes a string argument
     * -d 10            # Option that takes an integer argument
     */
    retValue = UiPass1(argcPtr, argv, argTable, flags);

    if (CDAS_SUCCESS == retValue)
       /*
        * Pass 2 extracts all the positional parameters from argTable[].
        * These parameters may be required, or optional.
        */
       retValue = UiPass2(argcPtr, argv, argTable, flags);
 
    if (CDAS_SUCCESS == retValue)  {
       if (flags &  UI_ARGS_DONT_SKIP_FIRST_ARG)
          allowed_left = 0;
       else 
          allowed_left = 1;
       if (flags & UI_ARGS_NO_LEFTOVERS)  {
           if (*argcPtr > allowed_left)  {         
               /*
                * If the user did not want any leftovers, but there's still
                * stuff left in argv[], flag that as an error.
                */
               retValue = CDAS_ARGS_BADSYNTAX;
	       UiErrPlace(); //UiErrAdd(retValue, CDAS_WARNING, "unprocessed command line parameters remain\n(check for extra parameters or invalid options)", "");
	   }
       }
   }

   return retValue;
}

/*
 * NAME
 *   UiNeedHelp - See if the user needs help for the command
 *
 * CALL:
 *   (static int) UiNeedHelp(const int argc, char **argv,
 *                           const UiArgsInfo *argTable, const int flags)
 *                a_argc   - Argument count
 *                argv     - Command line argument array
 *                argTable - Table of known arguments
 *                flags    - Flags that modify the processing of argTable[]
 *
 * DESCRIPTION:
 *   UiNeedHelp() goes thru argv and matches each argument against 
 *   argTable[] to see if the user specified a command line argument that
 *   matches a UI_ARGS_HELP key in argTable[]. Ordinarily, the search being
 *   done here could be accomplished in UiPass1(). But that function
 *   changes the state of the argument table by assigning values to
 *   the destination fields. The help needs to be created from a pristine
 *   table.
 *
 * RETURNS:
 *   1 : on success
 *   0 : otherwise
 */
static int UiNeedHelp(const int a_argc, char **argv, 
		      const UiArgsInfo *argTable, const int flags)
{
    register UiArgsInfo *infoPtr;
				/* Pointer to the current entry in the
				 * table of argument descriptions. */
    UiArgsInfo *matchPtr;	/* Descriptor that matches current argument. */
    char *curArg;		/* Current argument */
    register char c;		/* Second character of current arg (used for
				 * quick check for matching;  use 2nd char.
				 * because first char. will almost always
				 * be '-'). */
    int length;			/* Number of characters in current argument. */
    int srcIndex = 0;
    int argc;
    int i;

    argc = a_argc;
 
    /*
     * Do the real parsing...
     */
    while (argc > 0) {
	curArg = argv[srcIndex++];
	argc--;
	c = curArg[1];
	length = strlen(curArg);
	if (length == 0) {
	  /*
	   * Someone passed a NULL string.  Just move on.
	   */
	   continue;
	}

	/*
	 * Loop throught the argument descriptors searching for one with
	 * the matching key string.  If found, leave a pointer to it in
	 * matchPtr.
	 */

	matchPtr = NULL;
	for (i = 0; i < 2; i++) {
	    if (i == 0) {
		infoPtr = (UiArgsInfo *) argTable;
	    } else {
                if (!(flags & UI_ARGS_NO_DEFAULTS))
		    infoPtr = defaultTable;
                else
                    continue;
	    }
	    for (; infoPtr->type != UI_ARGS_END; infoPtr++) {
		 if (infoPtr->key == NULL) {
		     continue;
		 }

                 /*
                  * We're only interested in UI_ARGS_HELP
                  */
                 if (infoPtr->type != UI_ARGS_HELP)
                     continue;

		 if ((infoPtr->key[1] != c)
			 || (strncmp(infoPtr->key, curArg, length) != 0)) {
		     continue;
		 }
		 if (infoPtr->key[length] == 0) {
		     matchPtr = infoPtr;
		     goto gotMatch;
		 }
		 if (flags & UI_ARGS_NO_ABBREV) {
		     continue;
		 }
		 if (matchPtr != NULL)
		     return 0;

		 matchPtr = infoPtr;
	    }
        }

	if (matchPtr == NULL)
	    continue;

	/*
	 * Take the appropriate action based on the option type. Of course,
         * here we are only interested in the UI_ARGS_HELP option. If it's
         * found, return immediately.
	 */
gotMatch:
	infoPtr = matchPtr;
	switch (infoPtr->type) {
	    case UI_ARGS_CONSTANT:
	    case UI_ARGS_INT:
	    case UI_ARGS_STRING:
            case UI_ARGS_DOUBLE :
            case UI_ARGS_FUNC:
		break;
	    case UI_ARGS_HELP:
                return 1;
	    default:
                break;
	}

    }

    return 0;
}

/*
 * NAME
 *   UiPass1 - Do a first pass through argv[], weeding out all known 
 *             switches specified in argTable[].
 *
 * CALL: 
 *   (static int) UiPass1(int *argcPtr,
 *                        char **argv, UiArgsInfo *argTable, int flags)
 *                argcPtr  - Pointer to the address of argument count
 *                argv     - Command line argument array
 *                argTable - Table of known arguments
 *                flags    - Flags that modify the processing of argTable[]
 *
 * DESCRIPTION:
 *   UiPass1() performs a first pass through the command line arguments
 *   specified in argv[]. As it goes thru it's motions, it strips out all
 *   known arguments. Known arguments are specified in argTable[]. As an
 *   argument is recognized and processed, it is removed from argv[]. 
 *   *argcPtr will be decremented to account for this. Only switches
 *   are weeded out in this pass. Positional parameters are searched for 
 *   in the next pass.
 *
 *   Example - Consider the following command line argument:
 *
 *      myCommand reg1 20 -debugLevel 3 -useMalloc -delay 10 -o
 *
 *   Also consider that argTable[] knows about the following arguments
 *   <reg1>       - Positional parameter
 *   [<numCols>]  - Optional positional parameter
 *   -debugLevel  - Switch that takes an argument
 *   -useMalloc   - Switch that takes no argument
 *
 *   After a call to this function, argv[] will look like:
 *              argv[0] ---> myCommand
 *              argv[1] ---> reg1
 *              argv[2] ---> 20
 *              argv[3] ---> -delay
 *              argv[4] ---> 10
 *              argv[5] ---> -o
 *   argcPtr will have been modified to contain 6.
 *
 * RETURNS:
 *    CDAS_SUCCESS        : if command-line options parsed successfully
 *    CDAS_ARGS_GIVEHELP  : if user gave option to request help for command
 *    CDAS_ARGS_BADSYNTAX : on error. 
 */
static int UiPass1(int *argcPtr, char **argv, 
		   UiArgsInfo *argTable, int flags)
{
    register UiArgsInfo *infoPtr;
				/* Pointer to the current entry in the
				 * table of argument descriptions. */
    UiArgsInfo *matchPtr;	/* Descriptor that matches current argument. */
    char *curArg;		/* Current argument */
    register char c;		/* Second character of current arg (used for
				 * quick check for matching;  use 2nd char.
				 * because first char. will almost always
				 * be '-'). */
    int srcIndex;		/* Location from which to read next argument
				 * from argv. */
    int dstIndex;		/* Index into argv to which next unused
				 * argument should be copied (never greater
				 * than srcIndex). */
    int argc;			/* # arguments in argv still to process. */
    int length;			/* Number of characters in current argument. */
    int i;
    char tmpKeyBuf[30], buf[20];

   if (flags & UI_ARGS_DONT_SKIP_FIRST_ARG)  {
       srcIndex = dstIndex = 0;
       argc = *argcPtr;
   } else  {
       srcIndex = dstIndex = 1;
       argc = *argcPtr - 1;
   }

   if (assign_defaults(argTable) == 0)
       return CDAS_ARGS_BADSYNTAX;

    /*
     * Do the real parsing...
     */
    while (argc > 0) {
	curArg = argv[srcIndex];
	srcIndex++;
	argc--;
	c = curArg[1];
	length = strlen(curArg);
	if (length == 0) {
	  /*
	   * Someone passed a NULL string.  Just move on.
	   */
	  argv[dstIndex] = curArg;
	  dstIndex++;
	  continue;
	}

	/*
	 * Loop throught the argument descriptors searching for one with
	 * the matching key string.  If found, leave a pointer to it in
	 * matchPtr.
	 */

	matchPtr = NULL;
	for (i = 0; i < 2; i++) {
	    if (i == 0) {
		infoPtr = argTable;
	    } else {
                if (!(flags & UI_ARGS_NO_DEFAULTS))
		    infoPtr = defaultTable;
                else
                    continue;
	    }
	    for (; infoPtr->type != UI_ARGS_END; infoPtr++) {
		 if (infoPtr->key == NULL) {
		     continue;
		 }
		 if ((infoPtr->key[1] != c)
			 || (strncmp(infoPtr->key, curArg, length) != 0)) {
		     continue;
		 }
		 if (infoPtr->key[length] == 0) {
		     matchPtr = infoPtr;
		     goto gotMatch;
		 }
		 if (flags & UI_ARGS_NO_ABBREV) {
		     continue;
		 }
		 if (matchPtr != NULL) {
		     UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "ambiguous option", curArg);
		     return CDAS_ARGS_BADSYNTAX;
		 }
		 matchPtr = infoPtr;
	    }
	}
	if (matchPtr == NULL) {

	    /*
	     * Unrecognized argument.  Just copy it down
	     */
	    argv[dstIndex] = curArg;
	    dstIndex++;
	    continue;
	}

	/*
	 * Take the appropriate action based on the option type
	 */

	gotMatch:
	infoPtr = matchPtr;
	switch (infoPtr->type) {
	    case UI_ARGS_CONSTANT:
		*((int *) infoPtr->dst) = (long) infoPtr->src;
                sprintf(tmpKeyBuf, "%s,", infoPtr->key);
                strcat(g_Specified_args, tmpKeyBuf);
		break;
	    case UI_ARGS_INT:
		if (argc == 0) {
		    goto missingArg;
		} else {
		    char *endPtr;

		    *((int *) infoPtr->dst) =
			    strtoul(argv[srcIndex], &endPtr, 0);
		    if ((endPtr == argv[srcIndex]) || (*endPtr != 0)) {
		        /* expected integer argument but got argv[srcIndex] */
		        UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "expected integer argument but got", argv[srcIndex]);
			return CDAS_ARGS_BADSYNTAX;
		    }
		    srcIndex++;
		    argc--;
		  }
                sprintf(tmpKeyBuf, "%s,", infoPtr->key);
                strcat(g_Specified_args, tmpKeyBuf);
		break;
	    case UI_ARGS_STRING:
		if (argc == 0) {
		    goto missingArg;
		} else {
		    *((char **)infoPtr->dst) = argv[srcIndex];
		    srcIndex++;
		    argc--;
		}
                sprintf(tmpKeyBuf, "%s,", infoPtr->key);
                strcat(g_Specified_args, tmpKeyBuf);
		break;
            case UI_ARGS_DOUBLE :
		if (argc == 0) {
		    goto missingArg;
		} else {
		    char *endPtr;

		    *((double *) infoPtr->dst) =
			    strtod(argv[srcIndex], &endPtr);
		    if ((endPtr == argv[srcIndex]) || (*endPtr != 0)) {
		        /* expected floating point but got  argv[srcIndex] */
		        UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "expected floating point argument but got", argv[srcIndex]);
			return CDAS_ARGS_BADSYNTAX;
		    }
		    srcIndex++;
		    argc--;
		}
                sprintf(tmpKeyBuf, "%s,", infoPtr->key);
                strcat(g_Specified_args, tmpKeyBuf);
		break;
	    case UI_ARGS_FUNC: {
		int (*handlerProc)(void*,void*,void*);

		handlerProc = (int (*)(void*,void*,void*))infoPtr->src;
		
		if ((*handlerProc)(infoPtr->dst, infoPtr->key,
			argv[srcIndex])) {
		    srcIndex += 1;
		    argc -= 1;
		}
                sprintf(tmpKeyBuf, "%s,", infoPtr->key);
                strcat(g_Specified_args, tmpKeyBuf);
		break;
	    }
	    default:
	        /* bad argument type */
	      sprintf(buf, "%d", infoPtr->type);
	      UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "bad argument type", buf);
		return CDAS_ARGS_BADSYNTAX;
	}
    }

    /*
     * Copy the remaining arguments down.
     */
    while (argc > 0) {
	argv[dstIndex] = argv[srcIndex];
	srcIndex++;
	dstIndex++;
	argc--;
    }
    argv[dstIndex] = (char *) NULL;
    *argcPtr = dstIndex;

    return CDAS_SUCCESS;

    missingArg:
    /* curArg option requires an additional argument */
    UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "option requires an additional argument -", curArg);
    return CDAS_ARGS_BADSYNTAX;
}

/*
 * NAME
 *   UiPass2 - Do a second pass through argv[], weeding out all known
 *             positional parameters specified in argTable[].
 *
 * CALL:
 *   (static int) UiPass2(int *argcPtr, char **argv, 
 *                        UiArgsInfo *argTable, int flags)
 *                argcPtr  - Pointer to the address of argument count
 *                argv     - Command line argument array
 *                argTable - Table of known arguments
 *                flags    - Flags that modify the processing of argTable[]
 *
 * DESCRIPTION:
 *   UiPass2() performs a second pass through the command line arguments
 *   specified in argv[]. Note that this argv[] has been (probably) modified
 *   by UiPass1(). Thus all known switches have been stripped out of argv.
 *   As UiPass2() goes thru it's motions, it strips out all positional
 *   parameters. Positional parameters are specified in argTable[]. As an
 *   parameter is recognized and processed, it is removed from argv[].
 *   *argcPtr will be decremented to account for this. 
 *
 *   Example - Working on the command line argument from the comments in
 *             UiPass1(), let's assume argv[] is now the following:
 *
 *           myCommand reg1 20 -delay 10 -o
 *
 *   i.e. all known switches have been removed from it.
 *
 *   Also consider that argTable[] knows about the following arguments
 *   <reg1>       - Positional parameter
 *   [<numCols>]  - Optional positional parameter
 *   -debugLevel  - Switch that takes an argument   # removed in UiPass1()
 *   -useMalloc   - Switch that takes no argument   # removed in UiPass1()
 *
 *   After a call to this function, argv[] will look like:
 *              argv[0] ---> myCommand
 *              argv[3] ---> -delay
 *              argv[4] ---> 10
 *              argv[5] ---> -o
 *   argcPtr will have been modified to contain 6.
 *
 * RETURNS:
 *   CDAS_SUCCESS         : on success
 *   CDAS_ARGS_BADSYNTAX  : otherwise.
 */
static int UiPass2(int *argcPtr, char **argv,
		   UiArgsInfo *argTable, int flags)
{
   register UiArgsInfo *infoPtr;
   int      reqPosParams,          /* Count of required positional params */
            optPosParams,          /* Count of optional positional params */
            i;
   short    flag;                  /* General purpose flag */
   char     *sp,                   /* Utility pointer used in various places */
            tmpBuf[30];            /* Temporary buffer area used many places */
   int start_pos;
   reqPosParams = 0;
   optPosParams = 0;
   if (flags & UI_ARGS_DONT_SKIP_FIRST_ARG)
      start_pos=0;
   else
      start_pos=1;
   flag = 0;
   /*
    * Do some housekeeping first: see how many positional parameters
    * are needed.
    */
   for (infoPtr = argTable; infoPtr->type != UI_ARGS_END; infoPtr++)  {
        if (infoPtr->type == UI_ARGS_HELP)
            continue;

        if (infoPtr->key == NULL)  {
            flag = 1;
            break;
	}

        if (*infoPtr->key == '-')
            continue;

        if (*infoPtr->key == '<')
            reqPosParams++;
        else if (*infoPtr->key == '[')
            optPosParams++;
        else  {
            flag = 1;
            break;
        }
   }

   if (flag)  {
       char *p;

       p = (char *)(infoPtr->key == NULL ? "NULL" : infoPtr->key);

       UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "programmer error, error in arg table, key must be non-null and begin with either '<', '[', or '-' only, not be -", p);
       return CDAS_ARGS_BADSYNTAX;
   }
 
   if (reqPosParams == 0 && optPosParams == 0) /* No sense sticking around */
       return CDAS_SUCCESS;               /* if pos. params. not needed */
 
   if (reqPosParams && *argcPtr <= start_pos)  {
       /* no posistional params and expected at least one */
       UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "expected at least one positional parameter", " ");
       return CDAS_ARGS_BADSYNTAX;
   }

   if (*argcPtr <= start_pos)   /* Boundary case: if by this pass only argv[0]*/
       return CDAS_SUCCESS;                   /* remains, return now */
   /* Go thru the formal parameters */
   for (infoPtr = argTable; infoPtr->type != UI_ARGS_END; infoPtr++)  {

        if (infoPtr->key == NULL)
            continue;
        if (*infoPtr->key == '-')   /* Skip all switches */
            continue;
        /*
         * Now set argv[] to the first non-switch argument. A switch
         * argument is anything preceeding a '-'. While searching for 
         * the first non-switch argument, check the first character
         * beyond the '-'. If that character is an alpha, we have
         * a bonafide switch. If that character is a digit or it is a '.' we 
         * will recognize it as a positional parameter (a negative number).
         * Note that when we start searching argv[], we start from
         * argv[1], effectively skipping argv[0]
         */
        flag = 0;
        i=start_pos;
        for (; argv[i] != NULL; i++)  {
             if ((*argv[i] == '-') && !(flags&UI_ARGS_IGNORE_FLAGS) )  {
                 if (isdigit(argv[i][1]) || argv[i][1] == '.')  {
                     /*
                      * argv[i] could be specified as -0.99 or -.99
                      */
                     flag = 1;
                     break; 
		 }
	         else 
                 if (flags&UI_PARSEARG) 
                     break;
             }
             else
                break;
        }
      
        /*
         * Boundary condition: see if argv[i] == NULL. If it is, error
         * out. argv[i] should not be NULL if more positional parameters
         * are expected.
         */
        
        if (*infoPtr->key == '[')  {
            if (argv[i] == NULL)
                break;
        }
        else if (*infoPtr->key == '<')  {
            if (argv[i] == NULL)  {
	      /* expected reqPosParams more required posisional params. */
	      sprintf(tmpBuf, "%d", reqPosParams);
	      UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "expected more required positional parameters -", tmpBuf);
	      return CDAS_ARGS_BADSYNTAX;
               
            }
            reqPosParams--;
	}

        /*
         * Now make the address passed in the table for this positional 
         * parameter point to argv[i]
         */
        switch (infoPtr->type)  {
                char keyBuf[50], *pSp;

            case UI_ARGS_INT :
                 *((int *) infoPtr->dst) = strtoul(argv[i], &sp, 0);
                  if (sp == argv[i] || *sp != 0)  {
		    /* expected integer posisitional params but got argv[i] */
		    UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "expected integer posisitional params but got", argv[i]);
		    return CDAS_ARGS_BADSYNTAX;
		  }
                  sprintf(keyBuf, "%s,", infoPtr->key);
                  strcat(g_Specified_args, keyBuf);
                  break;
            case UI_ARGS_DOUBLE :
                 *((double *) infoPtr->dst) = strtod(argv[i], &sp);
                 if (sp == argv[i] || *sp != 0)  {
		    /* expected floating point posisitional params but got 
		       argv[i] */
		    UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "expected floating point posisitional params but got", argv[i]);
                     return CDAS_ARGS_BADSYNTAX;
                 }
                 sprintf(keyBuf, "%s,", infoPtr->key);
                 strcat(g_Specified_args, keyBuf);
                 break;
            case UI_ARGS_STRING :
                 *((char **) infoPtr->dst) = argv[i];
                 sprintf(keyBuf, "%s,", infoPtr->key);
                 strcat(g_Specified_args, keyBuf);
                 break;
            case UI_ARGS_FUNC :
            case UI_ARGS_HELP :
            case UI_ARGS_CONSTANT :
                 /*
                  * None of these types can have positional parameters.
                  * Get rid of positional parameter delimiters '<...>' or
                  * '[...]' from infoPtr->key. Store the result in keyBuf[].
                  */
                 sprintf(keyBuf, "%s", &infoPtr->key[1]);
                 for (pSp = keyBuf; *pSp != (char )0; pSp++)
                      if (*pSp == '>' || *pSp == ']')
                          *pSp = (char )0;

		 /* wrong type for posisitional param */
		 UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "not a supported object for positional parameters -", infoPtr->key);
                    return CDAS_ARGS_BADSYNTAX;
                 break; /* NOTREACHED */
	}
      
        /* 
         * Now comes the fun part: shuffle the argv[] array to account
         * for the positional parameter just handled above. Example:
         * assume that the argv[] array contained:
         *       a.out -X 10 -name Region 20 -test
         * and that 20 is the positional parameter. Once the code above
         * has been executed and 20 has been recognized, argv[] should
         * be compacted so that it looks like:
         *       a.out -X 10 -name Region -test
         */
        for (; argv[i] != NULL; i++)
             argv[i] = argv[i+1];
        argv[i] = NULL;
         
   }

   for (i = 0; argv[i] != NULL; i++)   /* Set *argcPtr to contain the number */
        ;                              /* of elements in argv[] */
   *argcPtr = i;
 
   return CDAS_SUCCESS;
}

/*
 * NAME: assign_defaults()
 *
 * CALL:
 *   (int) assign_defaults(UiArgsInfo *argTable)
 *         argTable - Argument table
 *
 * DESCRIPTION:
 *   assign_defaults() goes through the argument table assigning default 
 *   values from the src field to the dst field. The src field contains a
 *   string representation of the default value. It may be NULL. If the src
 *   field is not NULL, the value in the field is converted to the appropriate 
 *   representation of dst and saved there (in dst).
 *
 *   If the user later specifies a different value for a command line 
 *   argument, the value of dst will be over-written with the new one.
 *
 * RETURNS: 
 *   1 : on success
 *   0 : on failure
 */
static int assign_defaults(UiArgsInfo *argTable)
{
   register UiArgsInfo *infoPtr;

   /*
    * Go thru the table assigning default values if specified. Default
    * values are specified in the src field. So, if the src field is not
    * NULL, convert the src field to the proper representation of the dst
    * field, and save that value in the dst field. 
    *
    * This way, if the user specifies switch value on the command line, the
    * default values can always be overwritten later. If the user did not
    * specify a command line value, it will be properly defaulted.
    */
   for (infoPtr = argTable; infoPtr->type != UI_ARGS_END; infoPtr++)  {
            char   *pSrc,
                   *pEnd;

            pSrc = (char *) infoPtr->src;

            switch (infoPtr->type)  {
               case UI_ARGS_STRING  :
                    if ((pSrc != NULL) && (infoPtr->dst != NULL))
                        *((char **)infoPtr->dst) = pSrc;
                    break;
               case UI_ARGS_CONSTANT :
               case UI_ARGS_FUNC     :
               case UI_ARGS_HELP     :
                    break;
               case UI_ARGS_INT      :
                    if ((pSrc != NULL) && (infoPtr->dst != NULL))
                    {
                        *((int *) infoPtr->dst) = strtoul(pSrc, &pEnd, 0);
                        if (pEnd == pSrc || *pEnd != 0)  {
			    /* parse error, cannot convert default value pSrc
			       to an integer */
			    UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "cannot convert default value to an integer -", pSrc);
                            return 0;
                        }
                    }
                    break;
   	       case UI_ARGS_DOUBLE   :
                    if ((pSrc != NULL) && (infoPtr->dst != NULL))
                    {
                        *((double *)infoPtr->dst) = strtod(pSrc, &pEnd);
                        if (pEnd == pSrc || *pEnd != 0)  {
			    /* parse error, cannot convert default value pSrc
			       to an double */
			    UiErrPlace(); //UiErrAdd(CDAS_ARGS_BADSYNTAX, CDAS_FATAL, "cannot convert default value to a double -", pSrc);
                            return 0;
                        }
                    }
                    break;
	    }
   }

   return 1;
}

/*
 * NAME: UiArgIsPresent
 *
 * CALL:
 *   (int) UiArgIsPresent(int argc, char **argv, 
 *                        char *arg, UiArgsInfo *unused)
 *         argc     - Current argument count (Unused)
 *         argv     - Current argument array (Unused)
 *         arg      - Argument to search for
 *         argTable - Argument table. (Unused)
 *
 * DESCRIPTION:
 *   UiArgIsPresent() ascertains if a given switch/parameter was present
 *   on the command line. As of this not all the arguments to this function
 *   are used. However, I have decided to make the signature of this
 *   function as above in the eventuality that they are needed at some later 
 *   date.
 *
 * RETURNS:
 *   1 : if arg was present on the command line
 *   0 : if it was not
 *  -1 : if arg was not specified in the argument table
 *
 * CAVEATS:
 *   This function does not save the state of the table being parsed. So, if
 *    multiple tables are being used in the same function, all information
 *    for a table should be extracted before parsing the next table. That is,
 *    the caller should do something like so:
 *
 *   UiArgsInfo Tbl_A, Tbl_B;
 *   ...
 *   UiParseArgs(..., Tbl_A, ...);
 *   var1 = UiArgIsPresent(...);
 *   var2 = UiArgIsPresent(...);
 *   ...
 *   UiParseArgs(..., Tbl_B, ...);
 *   var3 = UiArgIsPresent(...);
 *   var4 = UiArgIsPresent(...);
 *   ...
 *
 *   This function should be called AFTER the initial call to UiParseArgs().
 *
 *   Ostensibly, we can make the package infinitely more complex by introducing
 *   state information for a table. If so, then calls to UiParseArgs() and
 *   UiArgIsPresent() can be interspersed. But I don't think it's worth
 *   the time and effort to do so. Furthermore if we do so, we will have to
 *   provide APIs to release the state of a table. A user might forget to call
 *   the release API thus resulting in all these memory leaks. I think we 
 *   should subscribe to the KISS rule. The package is enough complex as it is.
 */
int UiArgIsPresent(int argc, char **argv, char *arg, UiArgsInfo *argTable)
{
   char *pSwitchArray,
         tmpBuf[50];
   int  retValue,
        i;

   if (strlen(g_Specified_args) == 0)
       return 0;

   retValue = i = 0;
   pSwitchArray = g_Specified_args;

   /*
    * g_Specified_args is an array  containing all the command line
    * switches/parameters passed to UiParseArgs(). It's contents will be 
    * of the form:
    *
    *    -switch1,-switch2,<param1>,-switch3,[param2],...,-switchn,
    *
    * Finding out if a certain command line switch/parameter was specified 
    * by the user now simply boils down to tokenizing this array and searching
    * each token for a match.
    */
   for (; *pSwitchArray != (char )0; pSwitchArray++)  {
        if (*pSwitchArray == ',')  {
            tmpBuf[i] = (char )0;
            if (strcmp(tmpBuf, arg) == 0)  {
                retValue = 1;
                break;
            } else  {
              i = 0;
            }
        } else  {
            tmpBuf[i++] = *pSwitchArray;
        }
   }  

   if (retValue == 0)  {
       /*
        * If arg was not specified on the command line, let's see if it was
        * present in the argument table. If it was not, we want to return
        * a -1 instead of a 0
        */
       register UiArgsInfo *ptr;

       for (ptr = argTable; ptr->type != UI_ARGS_END; ptr++)
            if (ptr->key)
                if (strcmp(ptr->key, arg) == 0)
                     break;

       if (ptr->type == UI_ARGS_END)
           retValue = -1;
   }

   return retValue;
}

/* **** private defines for constructing usage, arginfo, and help **** */

#define UI_ARGS_INDENT_BUFSIZE 101

/* macros for accumulating usage, arginfo, and help strings */

#define UI_ARGS_APPENDRESULT(cur_str, new_str, cur_len, max_len) {  \
    if ((char *) NULL != new_str && (char *) NULL != cur_str) {       \
        cur_len += strlen(new_str);                                   \
        if (cur_len > max_len)                                        \
            return (char *) NULL;                                     \
        else                                                          \
            strcat(cur_str, new_str);                                 \
	}                                                             \
    }
#define UI_ARGS_APPENDUSAGE(new_str) { \
    UI_ARGS_APPENDRESULT(usage_str, new_str, usage_length, \
                         (UI_ARGS_USAGE_BUFSIZE - 1))      \
    }
#define UI_ARGS_APPENDARGINFO(new_str) {                       \
    UI_ARGS_APPENDRESULT(arginfo_str, new_str, arginfo_length, \
                         (UI_ARGS_ARGINFO_BUFSIZE - 1))        \
    }

/*
 * NAME 
 *   UiGetHelp - Public interface to get help string
 *
 * CALL:
 *   char *UiGetHelp(UiArgsInfo *argTable, int flags, 
 *                   char *cmd_name, int indent, char *usage_prefix,
 *                   char *usage_suffix, char *help_suffix)
 *
 *          argTable      - Table of known arguments
 *          flags         - Flags to modify processing of argTable[]
 *          cmd_name      - name of command (for usage string)
 *          indent        - num chars to indent usage line
 *          usage_prefix  - prefix to put before command syntax
 *          usage_suffix  - suffix to put after command syntax (and before arginfo)
 *          help_suffix   - suffix to put after arginfo
 *
 * DESCRIPTION:
 *   Create help message (with usage and argument decriptions) from 
 *   argTable information
 *
 * RETURNS:
 *   ptr to help message or NULL on error
 *   NOTE: The string pointed to by the returned pointer should not be 
 *   modified.  So you should copy the string if you need to keep it around.
 */
char *UiGetHelp(UiArgsInfo *argTable, int flags,
		char *cmd_name, int indent, char *usage_prefix,
		char *usage_suffix, char *help_suffix)
{
   int len;
   char *usage_str;
   char *arginfo_str;
   static char *help_str = (char *)NULL;

   usage_str = UiGetUsage(argTable, flags, cmd_name, indent, 
			  usage_prefix, usage_suffix);

   arginfo_str = UiGetArgInfo(argTable, flags, cmd_name, indent);

   /* format the whole string, first get rid of the old one */
   if (help_str != (char *)NULL) {
     free(help_str);
   }
   len = strlen(usage_str) + strlen(arginfo_str) + strlen(help_suffix);
   if ((help_str = (char *)malloc(len)) != (char *)NULL) {
     strcat(help_str, usage_str);
     strcat(help_str, arginfo_str);
     /* user might pass NULL here, that's ok (not an error) */
     if (NULL != help_suffix)
       strcat(help_str, help_suffix);
   }

    /* return ptr to help string */
    return help_str;
}

/*
 * ROUTINE:
 *    UiGetUsage
 *
 *      private routine to create a brief usage statement from argTable info.
 *
 * CALL:
 *   char *UiGetUsage(UiArgsInfo *argTable, int flags, char *cmd_name, 
 *                    int indent, char *usage_prefix, char *usage_suffix);
 *
 *          argTable      - Table of known command-line options
 *          flags         - Flags that control the parsing the of argTable
 *          cmd_name      - name of command giving usage for
 *          indent        - num chars to indent usage line
 *          usage_prefix  - prefix to put before command syntax
 *          usage_suffix  - suffix to put after command syntax
 *
 * DESCRIPTION:
 *   Generates a brief usage string describing command-line options.
 *
 * RETURNS:
 *   ptr to usage message or NULL on error
 *   NOTE: The string pointed to by the returned pointer should not be 
 *   modified. So you should copy the string if you need to keep it around.
 */
char *UiGetUsage(UiArgsInfo *argTable, int flags, char *cmd_name, 
		 int indent, char *usage_prefix, char *usage_suffix)
{
    register UiArgsInfo *infoPtr;  /* ptr to entry in argTable */
    char indent_str[UI_ARGS_INDENT_BUFSIZE];
                                     /* string form of "indent" spaces */
    int i;                           /* index variable */

    char empty_string[] = "";
    char one_space[]    = " ";

    char *after_prefix_spacing;
    char *after_cmd_spacing;

    int usage_length;                
    static char *usage_str = (char *) NULL;

    /* only do malloc if haven't done it before or last try failed.
     * If this malloc fails, return
     */
    if ( (char *) NULL == usage_str) {
        if ((usage_str = (char *) malloc((size_t) UI_ARGS_USAGE_BUFSIZE)) ==
	    (char *) NULL)
	  return(usage_str);
    }
    usage_str[0] = (char )0;
    usage_length = 0;

    /* put in default values for parameters if requested */

    if (NULL == usage_prefix)
        usage_prefix = empty_string;
    if (NULL == usage_suffix)
        usage_suffix = empty_string;
    if (NULL == cmd_name)
        cmd_name = empty_string;
    if (indent < 0)
        indent = 0;
    if (indent > (UI_ARGS_INDENT_BUFSIZE - 1))
        indent = UI_ARGS_INDENT_BUFSIZE - 1;

    if (0 == strlen(usage_prefix))
        after_prefix_spacing = empty_string;
    else
        after_prefix_spacing = one_space;

    if (0 == strlen(cmd_name))
        after_cmd_spacing = empty_string;
    else
        after_cmd_spacing = one_space;

    /* put in indent, usage prefix, and command name */
    sprintf(indent_str,"%*s", indent, "");

    UI_ARGS_APPENDUSAGE(indent_str)
    UI_ARGS_APPENDUSAGE(usage_prefix)
    UI_ARGS_APPENDUSAGE(after_prefix_spacing) 
    UI_ARGS_APPENDUSAGE(cmd_name)
    UI_ARGS_APPENDUSAGE(after_cmd_spacing)

    /* put in keys for arguments in argTable */
    for (i = 0; i < 2; i++) {
	for (infoPtr = i ? defaultTable : argTable;
		infoPtr->type != UI_ARGS_END; infoPtr++) {
	    if (infoPtr->key == NULL) {
		continue;
	    }
	    UI_ARGS_APPENDUSAGE(infoPtr->key)
	    UI_ARGS_APPENDUSAGE(" ")
	}
	if ((flags & UI_ARGS_NO_DEFAULTS) || (i > 0)) {
	    break;
	}
    }

    /* put in usage suffix */
    UI_ARGS_APPENDUSAGE(usage_suffix)

    /* return ptr to usage string */
    return usage_str;

}

/*
 * ROUTINE:
 *    UiGetArgInfo
 *
 *      public routine to create string with argument descriptions/help
 *      from argTable info.
 *
 * CALL:
 *   char *UiGetArgInfo(UiArgsInfo *argTable, 
 *                       int flags, char *cmd_name, int indent);
 *
 *          argTable      - Table of known command-line options
 *          flags         - Flags that control the parsing the of argTable
 *          cmd_name      - name of command giving usage for
 *          indent        - num chars to indent ArgInfo text
 *
 * DESCRIPTION:
 *   Create Argument description / help message from argTable information. 
 *   This ArgInfo text will contain argument key names, a help string for 
 *   each argument key (if the programmer has supplied a help string), and 
 *   default values for arguments (when available). Also help text will be 
 *   printed for argTable entries with type = UI_ARGS_HELP and key=NULL.
 *
 * RETURNS:
 *   ptr to arginfo message or NULL on error 
 *   NOTE: The string pointed to by the returned pointer should not be 
 *   modified.  So you should copy the string if you need to keep it around.
 */
char *UiGetArgInfo(UiArgsInfo *argTable, int flags, 
		   char *cmd_name, int indent)
{
#define NUM_SPACES 20

    register UiArgsInfo *infoPtr;
    int width, i, numSpaces;
    char spaces[] = "                    ";
    char tmp[UI_ARGS_ARGINFO_BUFSIZE];

    char indent_str[UI_ARGS_INDENT_BUFSIZE]; 
                                   /* string form of "indent" spaces */
    char indent_arg_str[UI_ARGS_INDENT_BUFSIZE*2];
                                   /* string form of # spaces to precede
                                    * printing of new line of arg help str */
    char indent_default_str[UI_ARGS_INDENT_BUFSIZE*2];  
                                   /* string form of # spaces to precede
                                    * printing of arg default value */

    char empty_string[] = "";

    int arginfo_length; 
    static char *arginfo_str = (char *) NULL;

    /* only do malloc if haven't done it before or last try failed.
     * If this malloc fails, return
     */
    if ( (char *) NULL == arginfo_str) {
        if ((arginfo_str = (char *) malloc((size_t) UI_ARGS_ARGINFO_BUFSIZE))
	    == (char *) NULL)
	  return(arginfo_str);
    }
     
    arginfo_str[0] = (char )0;
    arginfo_length = 0;
      
    /* put in default values for parameters if requested */

    if (NULL == cmd_name)
        cmd_name = empty_string;
    if (indent < 0)
        indent = 0;
    if (indent > (UI_ARGS_INDENT_BUFSIZE - 1))
        indent = UI_ARGS_INDENT_BUFSIZE - 1;

    /* put newline as first char of arginfo string */
    UI_ARGS_APPENDARGINFO("\n")

    /*
     * First, compute the width of the widest option key, so that we
     * can make everything line up.
     */
    width = 4;
    for (i = 0; i < 2; i++) {
	for (infoPtr = i ? defaultTable : argTable;
		infoPtr->type != UI_ARGS_END; infoPtr++) {
	    int length;
	    if (infoPtr->key == NULL) {
		continue;
	    }
	    length = strlen(infoPtr->key);
	    if (length > width) {
		width = length;
	    }
	}
    }

    /* make strings for indent that arg elements use and indent that default
     * values for arg elements will use. Put Default values at a 4 space 
     * indent further than the starting point of help string for a particular 
     * arg. The regular help string of an arg starts after 
     * indent + width_of_longest_key + ": " separator
     */
    sprintf(indent_str,"%*s", indent, "");
    sprintf(indent_arg_str, "%s%*s", indent_str, width+(int)strlen(": "), "");
    sprintf(indent_default_str, "%s%*s", indent_arg_str, 4, "");

    for (i = 0; ; i++) {
	for (infoPtr = i ? defaultTable : argTable;
		infoPtr->type != UI_ARGS_END; infoPtr++) {

            char buf[UI_ARGS_ARGINFO_BUFSIZE];
            char *start, *end;   /* start and end of segments of help string */
            int len;             /* length of segment of help string */

	    if ((infoPtr->type == UI_ARGS_HELP) && (infoPtr->key == NULL)) {
                start = infoPtr->help;
                if (NULL != infoPtr->help)
                    len = strlen(infoPtr->help);

                while ((char *)NULL != start && (char )0 != *start) {
                    end = strchr(start,'\n');
                    if ((char *) NULL == end)
                        end = infoPtr->help + len;
                    else
                        while ('\n' == *end) end++;

                    if (end - start >  UI_ARGS_ARGINFO_BUFSIZE - 1)
                        return((char *) NULL);

                    strncpy(buf, start, end - start);
                    buf[end-start] = (char )0;
		    UI_ARGS_APPENDARGINFO(indent_str)
                    UI_ARGS_APPENDARGINFO(buf)
                    start = end;
                }
                /* always finish specifig arg help string with newline */
		UI_ARGS_APPENDARGINFO("\n") 
		continue;
	    }

	    UI_ARGS_APPENDARGINFO(indent_str)
            if (NULL != infoPtr->key) {
                UI_ARGS_APPENDARGINFO(infoPtr->key)
	    }
	    UI_ARGS_APPENDARGINFO(":")
              
	    numSpaces = width + 1;
            if ((char *) NULL != infoPtr->key)
                numSpaces -= strlen(infoPtr->key);
	    while (numSpaces > 0) {
		if (numSpaces >= NUM_SPACES) {
		    UI_ARGS_APPENDARGINFO(spaces)
		} else {
		    UI_ARGS_APPENDARGINFO(spaces+NUM_SPACES-numSpaces)
		}
		numSpaces -= NUM_SPACES;
	    }

            start = infoPtr->help;
            if (NULL != infoPtr->help)
                len = strlen(infoPtr->help);

            while ((char *)NULL != start && (char )0 != *start) {
                end = strchr(start,'\n');
                if ((char *) NULL == end)
                    end = infoPtr->help + len;
                else
                    while ('\n' == *end) end++;

                if (end - start >  UI_ARGS_ARGINFO_BUFSIZE - 1)
                    return((char *) NULL);

                strncpy(buf, start, end - start);
                buf[end-start] = (char )0;
                /* already have the proper indent if at start of help_str */
                if (start != infoPtr->help)
		    {
		      UI_ARGS_APPENDARGINFO(indent_arg_str)
		    }
		UI_ARGS_APPENDARGINFO(buf)
                start = end;
            }
            /* always finish specific arg help string with newline */

	    UI_ARGS_APPENDARGINFO("\n")

            /* don't print default values for required parameters
             * required parameters have "<" as the first char
             */
            if (NULL != infoPtr->key && '<' == (infoPtr->key)[0])
                continue;

        if (infoPtr->src != NULL)
	{
            switch (infoPtr->type) {
                case UI_ARGS_INT: {
		    UI_ARGS_APPENDARGINFO(indent_default_str)
		    UI_ARGS_APPENDARGINFO("Default value: ")
		    UI_ARGS_APPENDARGINFO((const char *)infoPtr->src)
		    UI_ARGS_APPENDARGINFO("\n")
                    break;
                }
                case UI_ARGS_DOUBLE :  {
		    UI_ARGS_APPENDARGINFO(indent_default_str)
		    UI_ARGS_APPENDARGINFO("Default value: ")
		    UI_ARGS_APPENDARGINFO((const char *)infoPtr->src)
                    UI_ARGS_APPENDARGINFO("\n")
                    break;
                }
                case UI_ARGS_STRING: {
		    UI_ARGS_APPENDARGINFO(indent_default_str)
		    UI_ARGS_APPENDARGINFO("Default value: \"")
		    UI_ARGS_APPENDARGINFO((const char *)infoPtr->src)
		    UI_ARGS_APPENDARGINFO("\"")
		    UI_ARGS_APPENDARGINFO("\n")
                    break;
                    }
                default: {
                    break;
                }
            }
        }
    }

	if ((flags & UI_ARGS_NO_DEFAULTS) || (i > 0)) {
	    break;
	}
    }

    /* return ptr to arginfo string */
    return arginfo_str;
}
