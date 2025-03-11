/*
 * UiArgs.h - Generic command-line parser. 
 *
 */

#ifndef _UIARGS_H
#define _UIARGS_H

#include "UiErr.h"

/*
 * Structure to specify how to handle command line options
 */
typedef struct {
    char *key;          /* The key string that flags the option in the
                         * argv array. */
    int type;           /* Indicates option type;  see below. */
    void *src;          /* Value to be used in setting dst;  usage
                         * depends on type. */
    void *dst;          /* Address of value to be modified;  usage
                         * depends on type. */
    char *help;         /* Documentation message describing this option. */
} UiArgsInfo;

/*
 * Public functions
 */
int UiParseArgs(int *, char **, UiArgsInfo *, int);

int UiArgIsPresent(int, char **, char *, UiArgsInfo *);

char *UiGetUsage(UiArgsInfo *argTable, int flags, char *cmd_name, int indent, 
		 char *usage_prefix, char *usage_suffix);

char *UiGetArgInfo(UiArgsInfo *argTable, int flags, char *cmd_name, 
		   int indent);

char *UiGetHelp(UiArgsInfo *argTable, int flags, char *cmd_name, int indent, 
		char *usage_prefix, char *usage_suffix, char *help_suffix);

/* defines for return value of UiParseArgs */

#define CDAS_ARGS_GIVEHELP    -1

/*
 * Legal values for the type field of a UiArgsInfo: see the user
 * documentation for details.
 */
 
#define UI_ARGS_CONSTANT                0x64
#define UI_ARGS_INT                     0x65
#define UI_ARGS_STRING                  0x66
#define UI_ARGS_DOUBLE                  0x68
#define UI_ARGS_FUNC                    0x69
#define UI_ARGS_HELP                    0x6A
#define UI_ARGS_END                     0x6B

/*
 * Flag bits for passing to UiParseArgv()
 */
 
#define UI_ARGS_NO_DEFAULTS             0x1
#define UI_ARGS_NO_LEFTOVERS            0x2
#define UI_ARGS_NO_ABBREV               0x4
#define UI_ARGS_DONT_SKIP_FIRST_ARG     0x8
#define UI_ARGS_IGNORE_FLAGS	        0x10
#define UI_PARSEARG                     0x20

/* help string and usage string max sizes */

#define UI_ARGS_USAGE_BUFSIZE    1025
#define UI_ARGS_ARGINFO_BUFSIZE  8193

#endif


