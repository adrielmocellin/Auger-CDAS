#include <stdio.h>

#include "UiArgs.h"
#include "UiErr.h"

UiArgsInfo g_argsTable[] = {
  {NULL, UI_ARGS_HELP, NULL, NULL, "This is a sample args table!\n\n"},
  {"-i", UI_ARGS_INT, NULL, NULL, "This is an integer."},
  {"-c", UI_ARGS_CONSTANT, (void *)1, NULL, "This is a constant."},
  {"-s", UI_ARGS_STRING, (void *)"default", NULL, "This is a string."},
  {"-d", UI_ARGS_DOUBLE, NULL, NULL, "This is a double."},
  {"<p1>", UI_ARGS_STRING, NULL, NULL, "This is a required parameter."},
  {"[p2]", UI_ARGS_STRING, NULL, NULL, "This is an optional parameter."},
  {(char *)NULL, UI_ARGS_END, NULL, NULL, (char *)NULL}
};

int main(int argc, char *argv[])
{
  int ret;
  int c = 0, i = 1234;
  char *s;
  double d = 3.14159;
  char *p1 = "before1", *p2 = "before2";

  /* fill in the dst field in the table, these are addresses of the locations
     where UiParseFile will put the passed information */
  g_argsTable[1].dst = &i;
  g_argsTable[2].dst = &c;
  g_argsTable[3].dst = &s;
  g_argsTable[4].dst = &d;
  g_argsTable[5].dst = &p1;
  g_argsTable[6].dst = &p2;

  /* This routine takes as input, the name of a file.  this file is then read
     in and the contents are parsed and the appropriate variables filled in.
     for example, a file may look like this -

               -s "/d/f/g"
	       -d 98.7
	       param1
	       -c

     there are a few rules the file contents must follow -

       o only 1 option/value or parameter per line
       o all option names start with a '-'
       o option names may not contain whitespace
       o strings with embedded whitespace must be enclosed with parenthesis
                    (e.g. "the larch")
  */
  if ((ret = UiParseFile(argv[1], g_argsTable, UI_ARGS_NO_LEFTOVERS)) ==
      CDAS_SUCCESS) {
    printf("i is %d\n", i);
    printf("c is %d\n", c);
    printf("s is %s\n", s);
    printf("d is %g\n", d);
    printf("p1 is %s\n", p1);
    printf("p2 is %s\n", p2);


    if (UiArgIsPresent(argc, argv, g_argsTable[1].key, g_argsTable)) {
      printf("i was present\n");
    }
    if (UiArgIsPresent(argc, argv, g_argsTable[2].key, g_argsTable)) {
      printf("c was present\n");
    }
    if (UiArgIsPresent(argc, argv, g_argsTable[3].key, g_argsTable)) {
      printf("s was present\n");
    }
    if (UiArgIsPresent(argc, argv, g_argsTable[4].key, g_argsTable)) {
      printf("d was present\n");
    }
  } else {
    UiErrPlace(); UiErrOutput();
    printf("\n");
    printf(UiGetHelp(g_argsTable, UI_ARGS_NO_LEFTOVERS, "UiArgTest", 5, 
		     "USAGE: ", "", "\n"));
  }  
  return(0);
}
