#ifndef lint
static const char RegYYsccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20120115

#define YYEMPTY        (-1)
#define RegYYclearin      (RegYYchar = YYEMPTY)
#define RegYYerrok        (RegYYerrflag = 0)
#define YYRECOVERING() (RegYYerrflag != 0)

#define YYPREFIX "RegYY"

#define YYPURE 0

#include <stdio.h>

#include <Reg.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

int RegYYparse ();
void RegYYerror (const char* text);
int RegYYlex ();

void RegLexStartup (char* text);
RegExp RegExpParse (char* text);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

static RegSet CurrentSet = 0;
static RegExp CurrentExp = 0;

#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union {
  char character;
  RegValueRange valueRange;
  RegCharRange charRange;
  RegExp exp;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() RegYYparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() RegYYparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() RegYYparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() RegYYlex(void *YYLEX_PARAM)
# define YYLEX RegYYlex(YYLEX_PARAM)
#else
# define YYLEX_DECL() RegYYlex(void)
# define YYLEX RegYYlex()
#endif

/* Parameters sent to RegYYerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() RegYYerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) RegYYerror(msg)
#endif

extern int YYPARSE_DECL();

#define VALUERANGE 257
#define STAR 258
#define QUESTION 259
#define PLUS 260
#define OPENPAR 261
#define CLOSEPAR 262
#define OPENNOSET 263
#define OPENSET 264
#define CHARACTER 265
#define CLOSESET 266
#define CHARRANGE 267
#define VERTICAL 268
#define NEWLINE 269
#define DOT 270
#define CARRET 271
#define DOLLAR 272
#define PRIMARY 273
#define YYERRCODE 256
static const short RegYYlhs[] = {                           -1,
    0,    0,    1,    1,    2,    2,    3,    3,    3,    3,
    4,    4,    6,    6,    5,    5,    7,    7,    7,    7,
    7,    8,    8,    8,    8,    8,    9,    9,   10,   12,
   11,   11,   13,   13,   14,   14,   14,
};
static const short RegYYlen[] = {                            2,
    1,    3,    0,    1,    1,    3,    3,    1,    1,    2,
    0,    1,    0,    1,    1,    2,    1,    2,    2,    2,
    2,    1,    1,    1,    3,    1,    3,    3,    1,    1,
    0,    1,    1,    2,    1,    1,    1,
};
static const short RegYYdefred[] = {                         0,
    0,    9,    0,    1,    4,    0,    0,   10,    0,    0,
   26,    0,   30,   29,   22,   24,    0,   15,    0,   23,
    0,    0,    2,    6,    0,   14,    7,   16,   21,   18,
   19,   20,   37,   35,   36,    0,    0,   33,    0,   25,
   27,   34,   28,
};
static const short RegYYdgoto[] = {                          3,
    4,    5,    6,    7,   17,   27,   18,   19,   20,   21,
   36,   22,   37,   38,
};
static const short RegYYsindex[] = {                      -220,
 -272,    0, -265,    0,    0, -261, -231,    0, -220, -220,
    0, -220,    0,    0,    0,    0, -253,    0, -212,    0,
 -229, -229,    0,    0, -249,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -257, -229,    0, -248,    0,
    0,    0,    0,
};
static const short RegYYrindex[] = {                        26,
   16,    0,    0,    0,    0,   14,    0,    0,   26, -221,
    0, -241,    0,    0,    0,    0,    6,    0,    1,    0,
 -238, -238,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -235,    0,    0,    0,
    0,    0,    0,
};
static const short RegYYgindex[] = {                         0,
   -7,    0,   27,    0,    0,    0,   24,    0,    0,    0,
   28,    0,    0,   17,
};
#define YYTABLESIZE 296
static const short RegYYtable[] = {                          8,
   17,   23,   11,    9,   25,   13,   10,   12,   41,   13,
   14,   15,   40,    5,   11,    8,   16,   43,   26,   11,
    3,   11,   11,   11,   11,    3,   33,   31,   11,   12,
   32,   13,   14,   15,   11,   34,   24,   35,   16,   11,
   28,   11,   11,   11,   29,   30,   31,   32,   11,   39,
    1,    2,    0,   42,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   17,    0,    0,    0,
    0,   17,   17,   17,   17,   17,    0,   13,   17,   17,
   17,   12,   17,   13,   13,    5,   12,    8,   12,   12,
   12,   11,    5,    8,    8,   12,   11,    0,   11,   11,
   11,    0,    0,    0,    3,   11,
};
static const short RegYYcheck[] = {                        272,
    0,    9,  256,  269,   12,    0,  268,  261,  266,  263,
  264,  265,  262,    0,  256,    0,  270,  266,  272,  261,
  262,  263,  264,  265,  256,    0,  256,  266,  270,  261,
  266,  263,  264,  265,  256,  265,   10,  267,  270,  261,
   17,  263,  264,  265,  257,  258,  259,  260,  270,   22,
  271,  272,   -1,   37,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,   -1,   -1,
   -1,  261,  262,  263,  264,  265,   -1,  262,  268,  269,
  270,  256,  272,  268,  269,  262,  261,  262,  263,  264,
  265,  256,  269,  268,  269,  270,  261,   -1,  263,  264,
  265,   -1,   -1,   -1,  269,  270,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 273
#if YYDEBUG
static const char *RegYYname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"VALUERANGE","STAR","QUESTION",
"PLUS","OPENPAR","CLOSEPAR","OPENNOSET","OPENSET","CHARACTER","CLOSESET",
"CHARRANGE","VERTICAL","NEWLINE","DOT","CARRET","DOLLAR","PRIMARY",
};
static const char *RegYYrule[] = {
"$accept : exprs",
"exprs : expr",
"exprs : exprs NEWLINE expr",
"expr :",
"expr : nonemptyexpr",
"nonemptyexpr : pattern",
"nonemptyexpr : pattern VERTICAL pattern",
"pattern : optcarret pat optdollar",
"pattern : CARRET",
"pattern : DOLLAR",
"pattern : CARRET DOLLAR",
"optcarret :",
"optcarret : CARRET",
"optdollar :",
"optdollar : DOLLAR",
"pat : unary",
"pat : pat unary",
"unary : primary",
"unary : primary STAR",
"unary : primary QUESTION",
"unary : primary PLUS",
"unary : primary VALUERANGE",
"primary : CHARACTER",
"primary : set",
"primary : DOT",
"primary : OPENPAR expr CLOSEPAR",
"primary : error",
"set : openset optchars CLOSESET",
"set : opennoset optchars CLOSESET",
"openset : OPENSET",
"opennoset : OPENNOSET",
"optchars :",
"optchars : chars",
"chars : char",
"chars : chars char",
"char : CHARACTER",
"char : CHARRANGE",
"char : error",

};
#endif

int      RegYYdebug;
int      RegYYnerrs;

int      RegYYerrflag;
int      RegYYchar;
YYSTYPE  RegYYval;
YYSTYPE  RegYYlval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

typedef struct {
    unsigned stacksize;
    short    *s_base;
    short    *s_mark;
    short    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA RegYYstack;


void RegYYerror (const char* text)
{
  fprintf (stderr, "Reg> %s\n", text);
}

int RegYYwrap ()
{
  return (1);
}

RegExp RegExpParse (char* text)
{
  CurrentExp = 0;

  RegLexStartup (text);
  RegYYparse ();

  return (CurrentExp);
}


#if YYDEBUG
#include <stdio.h>		/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int RegYYgrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = data->s_mark - data->s_base;
    newss = (short *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void RegYYfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define RegYYfreestack(data) /* nothing */
#endif

#define YYABORT  goto RegYYabort
#define YYREJECT goto RegYYabort
#define YYACCEPT goto RegYYaccept
#define YYERROR  goto RegYYerrlab

int
YYPARSE_DECL()
{
    int RegYYm, RegYYn, RegYYstate;
#if YYDEBUG
    const char *RegYYs;

    if ((RegYYs = getenv("YYDEBUG")) != 0)
    {
        RegYYn = *RegYYs;
        if (RegYYn >= '0' && RegYYn <= '9')
            RegYYdebug = RegYYn - '0';
    }
#endif

    RegYYnerrs = 0;
    RegYYerrflag = 0;
    RegYYchar = YYEMPTY;
    RegYYstate = 0;

#if YYPURE
    memset(&RegYYstack, 0, sizeof(RegYYstack));
#endif

    if (RegYYstack.s_base == NULL && RegYYgrowstack(&RegYYstack)) goto RegYYoverflow;
    RegYYstack.s_mark = RegYYstack.s_base;
    RegYYstack.l_mark = RegYYstack.l_base;
    RegYYstate = 0;
    *RegYYstack.s_mark = 0;

RegYYloop:
    if ((RegYYn = RegYYdefred[RegYYstate]) != 0) goto RegYYreduce;
    if (RegYYchar < 0)
    {
        if ((RegYYchar = YYLEX) < 0) RegYYchar = 0;
#if YYDEBUG
        if (RegYYdebug)
        {
            RegYYs = 0;
            if (RegYYchar <= YYMAXTOKEN) RegYYs = RegYYname[RegYYchar];
            if (!RegYYs) RegYYs = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, RegYYstate, RegYYchar, RegYYs);
        }
#endif
    }
    if ((RegYYn = RegYYsindex[RegYYstate]) && (RegYYn += RegYYchar) >= 0 &&
            RegYYn <= YYTABLESIZE && RegYYcheck[RegYYn] == RegYYchar)
    {
#if YYDEBUG
        if (RegYYdebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, RegYYstate, RegYYtable[RegYYn]);
#endif
        if (RegYYstack.s_mark >= RegYYstack.s_last && RegYYgrowstack(&RegYYstack))
        {
            goto RegYYoverflow;
        }
        RegYYstate = RegYYtable[RegYYn];
        *++RegYYstack.s_mark = RegYYtable[RegYYn];
        *++RegYYstack.l_mark = RegYYlval;
        RegYYchar = YYEMPTY;
        if (RegYYerrflag > 0)  --RegYYerrflag;
        goto RegYYloop;
    }
    if ((RegYYn = RegYYrindex[RegYYstate]) && (RegYYn += RegYYchar) >= 0 &&
            RegYYn <= YYTABLESIZE && RegYYcheck[RegYYn] == RegYYchar)
    {
        RegYYn = RegYYtable[RegYYn];
        goto RegYYreduce;
    }
    if (RegYYerrflag) goto RegYYinrecovery;

    RegYYerror("syntax error");

    goto RegYYerrlab;

RegYYerrlab:
    ++RegYYnerrs;

RegYYinrecovery:
    if (RegYYerrflag < 3)
    {
        RegYYerrflag = 3;
        for (;;)
        {
            if ((RegYYn = RegYYsindex[*RegYYstack.s_mark]) && (RegYYn += YYERRCODE) >= 0 &&
                    RegYYn <= YYTABLESIZE && RegYYcheck[RegYYn] == YYERRCODE)
            {
#if YYDEBUG
                if (RegYYdebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *RegYYstack.s_mark, RegYYtable[RegYYn]);
#endif
                if (RegYYstack.s_mark >= RegYYstack.s_last && RegYYgrowstack(&RegYYstack))
                {
                    goto RegYYoverflow;
                }
                RegYYstate = RegYYtable[RegYYn];
                *++RegYYstack.s_mark = RegYYtable[RegYYn];
                *++RegYYstack.l_mark = RegYYlval;
                goto RegYYloop;
            }
            else
            {
#if YYDEBUG
                if (RegYYdebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *RegYYstack.s_mark);
#endif
                if (RegYYstack.s_mark <= RegYYstack.s_base) goto RegYYabort;
                --RegYYstack.s_mark;
                --RegYYstack.l_mark;
            }
        }
    }
    else
    {
        if (RegYYchar == 0) goto RegYYabort;
#if YYDEBUG
        if (RegYYdebug)
        {
            RegYYs = 0;
            if (RegYYchar <= YYMAXTOKEN) RegYYs = RegYYname[RegYYchar];
            if (!RegYYs) RegYYs = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, RegYYstate, RegYYchar, RegYYs);
        }
#endif
        RegYYchar = YYEMPTY;
        goto RegYYloop;
    }

RegYYreduce:
#if YYDEBUG
    if (RegYYdebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, RegYYstate, RegYYn, RegYYrule[RegYYn]);
#endif
    RegYYm = RegYYlen[RegYYn];
    if (RegYYm)
        RegYYval = RegYYstack.l_mark[1-RegYYm];
    else
        memset(&RegYYval, 0, sizeof RegYYval);
    switch (RegYYn)
    {
case 1:
	{
      CurrentExp = RegYYstack.l_mark[0].exp;
    }
break;
case 2:
	{
      CurrentExp = RegYYstack.l_mark[0].exp;
    }
break;
case 3:
	{
      RegYYval.exp = NULL;
    }
break;
case 4:
	{
      RegYYval.exp = RegYYstack.l_mark[0].exp;
    }
break;
case 5:
	{
      RegYYval.exp = RegYYstack.l_mark[0].exp;
    }
break;
case 6:
	{
      if ((RegYYstack.l_mark[-2].exp != NULL) && (RegYYstack.l_mark[0].exp != NULL)) RegYYval.exp = RegOrNew (RegYYstack.l_mark[-2].exp, RegYYstack.l_mark[0].exp);
      else RegYYval.exp = RegYYstack.l_mark[-2].exp;
    }
break;
case 7:
	{
      if (RegYYstack.l_mark[-2].exp != NULL) RegExpSetNext (RegYYstack.l_mark[-2].exp, RegYYstack.l_mark[-1].exp);
      if (RegYYstack.l_mark[0].exp != NULL) RegExpSetNext (RegYYstack.l_mark[-1].exp, RegYYstack.l_mark[0].exp);

      if (RegYYstack.l_mark[-2].exp != NULL) RegYYval.exp = RegYYstack.l_mark[-2].exp;
      else RegYYval.exp = RegYYstack.l_mark[-1].exp;
    }
break;
case 8:
	{
      RegYYval.exp = RegBolNew ();
    }
break;
case 9:
	{
      RegYYval.exp = RegEolNew ();
    }
break;
case 10:
	{
      RegExp exp;

      exp = RegBolNew ();
      RegExpSetNext (exp, RegEolNew());
      RegYYval.exp = exp;
    }
break;
case 11:
	{
      RegYYval.exp = NULL;
    }
break;
case 12:
	{
      RegYYval.exp = RegBolNew ();
    }
break;
case 13:
	{
      RegYYval.exp = NULL;
    }
break;
case 14:
	{
      RegYYval.exp = RegEolNew ();
    }
break;
case 15:
	{
      RegYYval.exp = RegYYstack.l_mark[0].exp;
    }
break;
case 16:
	{
      RegExpSetNext (RegYYstack.l_mark[-1].exp, RegYYstack.l_mark[0].exp);
      RegYYval.exp = RegYYstack.l_mark[-1].exp;
    }
break;
case 17:
	{
      RegYYval.exp = RegYYstack.l_mark[0].exp;
    }
break;
case 18:
	{
      if (RegYYstack.l_mark[-1].exp != NULL) RegYYval.exp = RegUnaryNew (RegStar, RegYYstack.l_mark[-1].exp);
      else RegYYval.exp = NULL;
    }
break;
case 19:
	{
      if (RegYYstack.l_mark[-1].exp != NULL) RegYYval.exp = RegUnaryNew (RegQuestion, RegYYstack.l_mark[-1].exp);
      else RegYYval.exp = NULL;
    }
break;
case 20:
	{
      if (RegYYstack.l_mark[-1].exp != NULL) RegYYval.exp = RegUnaryNew (RegPlus, RegYYstack.l_mark[-1].exp);
      else RegYYval.exp = NULL;
    }
break;
case 21:
	{
      RegYYval.exp = RegYYstack.l_mark[-1].exp;
    }
break;
case 22:
	{
      RegYYval.exp = RegCharNew (RegYYstack.l_mark[0].character);
    }
break;
case 23:
	{
      RegYYval.exp = RegYYstack.l_mark[0].exp;
    }
break;
case 24:
	{
      RegYYval.exp = RegDotNew ();
    }
break;
case 25:
	{
      RegYYval.exp = RegListNew (RegYYstack.l_mark[-1].exp);
    }
break;
case 26:
	{
      RegYYerror ("Bad primary");
      RegYYval.exp = NULL;
    }
break;
case 27:
	{
      RegYYval.exp = (RegExp) CurrentSet;
      CurrentSet = NULL;
    }
break;
case 28:
	{
      RegYYval.exp = (RegExp) CurrentSet;
      CurrentSet = NULL;
    }
break;
case 29:
	{
      CurrentSet = (RegSet) RegSetNew (RegSetOn);
    }
break;
case 30:
	{
      CurrentSet = (RegSet) RegSetNew (RegSetOff);
    }
break;
case 35:
	{
      RegSetAddChar (CurrentSet, RegYYstack.l_mark[0].character);
    }
break;
case 36:
	{
      char c;

      for (c = RegYYstack.l_mark[0].charRange->c1; c <= RegYYstack.l_mark[0].charRange->c2; c++)
      {
        RegSetAddChar (CurrentSet, c);
      }
    }
break;
case 37:
	{
      RegYYerror ("error in set");
    }
break;
    }
    RegYYstack.s_mark -= RegYYm;
    RegYYstate = *RegYYstack.s_mark;
    RegYYstack.l_mark -= RegYYm;
    RegYYm = RegYYlhs[RegYYn];
    if (RegYYstate == 0 && RegYYm == 0)
    {
#if YYDEBUG
        if (RegYYdebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        RegYYstate = YYFINAL;
        *++RegYYstack.s_mark = YYFINAL;
        *++RegYYstack.l_mark = RegYYval;
        if (RegYYchar < 0)
        {
            if ((RegYYchar = YYLEX) < 0) RegYYchar = 0;
#if YYDEBUG
            if (RegYYdebug)
            {
                RegYYs = 0;
                if (RegYYchar <= YYMAXTOKEN) RegYYs = RegYYname[RegYYchar];
                if (!RegYYs) RegYYs = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, RegYYchar, RegYYs);
            }
#endif
        }
        if (RegYYchar == 0) goto RegYYaccept;
        goto RegYYloop;
    }
    if ((RegYYn = RegYYgindex[RegYYm]) && (RegYYn += RegYYstate) >= 0 &&
            RegYYn <= YYTABLESIZE && RegYYcheck[RegYYn] == RegYYstate)
        RegYYstate = RegYYtable[RegYYn];
    else
        RegYYstate = RegYYdgoto[RegYYm];
#if YYDEBUG
    if (RegYYdebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *RegYYstack.s_mark, RegYYstate);
#endif
    if (RegYYstack.s_mark >= RegYYstack.s_last && RegYYgrowstack(&RegYYstack))
    {
        goto RegYYoverflow;
    }
    *++RegYYstack.s_mark = (short) RegYYstate;
    *++RegYYstack.l_mark = RegYYval;
    goto RegYYloop;

RegYYoverflow:
    RegYYerror("yacc stack overflow");

RegYYabort:
    RegYYfreestack(&RegYYstack);
    return (1);

RegYYaccept:
    RegYYfreestack(&RegYYstack);
    return (0);
}
