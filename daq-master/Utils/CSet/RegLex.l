%{
#include <stdio.h>
#include <stdlib.h>

#include <Reg.h>

#include <RegParse.h>

int RegLexInput ();

#ifndef YY_NO_INPUT

#undef input
#define input RegLexInput

#else

#undef YY_INPUT
#define YY_INPUT(buffer,result,max_size) \
{ \
  int c = '*', n; \
  for ( n = 0; n < max_size && \
               ((c = RegLexInput()) != 0) && \
               (c != '\n'); ++n ) \
    { \
      buffer[n] = (char) c; \
    } \
  if ( c == '\n' ) buffer[n++] = (char) c; \
  result = n; \
}

#endif

#undef unput
#define unput RegLexUnput
static void RegLexUnput (int c);

int yywrap ();
int yylook ();
int yyback ();

static int SetPosition = 0;
static int Level = 0;
static int* Positions = 0;
static int Position = 0;
static int Levels = 0;
static char* CurrentText = 0;
static int CurrentPos = 0;

void IncLevel ();
void DecLevel ();
void IncPosition ();
void ResetPosition ();

%}

%start EXP
%start SET

number [0-9]+

%%

<EXP>[\\][.] {
  IncPosition ();
  yylval.character = yytext[1];
  return (CHARACTER);
}

<EXP>[\\][*] {
  IncPosition ();
  yylval.character = yytext[1];
  return (CHARACTER);
}

<EXP>[\\][?] {
  IncPosition ();
  yylval.character = yytext[1];
  return (CHARACTER);
}

<EXP>[\\][\\] {
  IncPosition ();
  yylval.character = yytext[1];
  return (CHARACTER);
}

<EXP>[\\][\^] {
  IncPosition ();
  yylval.character = yytext[1];
  return (CHARACTER);
}

<EXP>[\\][$] {
  IncPosition ();
  yylval.character = yytext[1];
  return (CHARACTER);
}

<EXP>[\\][(] {
  IncPosition ();
  yylval.character = yytext[1];
  return (CHARACTER);
}

<EXP>[\\][)] {
  IncPosition ();
  yylval.character = yytext[1];
  return (CHARACTER);
}

<EXP>[\\][{]{number}([,]({number})?)?[\\][}] {
  static RegValueRangeRec range;

  IncPosition ();
  sscanf ((char*) &yytext[2], "%d,%d", &range.val1, &range.val2);
  yylval.valueRange = &range;
  return (VALUERANGE);
}

<EXP>[*] {
  return (STAR);
}

<EXP>[?] {
  return (QUESTION);
}

<EXP>[+] {
  return (PLUS);
}

<EXP>[(] {
  IncLevel ();
  return (OPENPAR);
}

<EXP>[)] {
  DecLevel ();
  return (CLOSEPAR);
}

<EXP>[\[][\^] {
  IncPosition ();
  SetPosition = 0;
  BEGIN SET;
  return (OPENNOSET);
}

<EXP>[\[] {
  IncPosition ();
  SetPosition = 0;
  BEGIN SET;
  return (OPENSET);
}

<SET>[\]] {
  if (SetPosition == 0)
  {
    yylval.character = yytext[0];
    return (CHARACTER);
  }
  else
  {
    BEGIN EXP;
    return (CLOSESET);
  }
}

<SET>[^-][-][^-\]] {
  static RegCharRangeRec range;

  SetPosition++;

  range.c1 = yytext[0];
  range.c2 = yytext[2];
  yylval.charRange = &range;
  return (CHARRANGE);
}

<SET>[\\][t] {
  SetPosition++;
  yylval.character = '\t';
  return (CHARACTER);
}

<SET>. {
  SetPosition++;
  yylval.character = yytext[0];
  return (CHARACTER);
}

<EXP>[|] {
  ResetPosition ();
  return (VERTICAL);
}

<EXP>[\n] {
  IncPosition ();
/*
  return (NEWLINE);
*/
  return (0);
}

<EXP>[.] {
  IncPosition ();
  return (DOT);
}

<EXP>[\^] {
  if (!Position)
  {
    IncPosition ();
    return (CARRET);
  }
  else
  {
    IncPosition ();
    yylval.character = yytext[0];
    return (CHARACTER);
  }
}

<EXP>[$] {
  IncPosition ();
  return (DOLLAR);
}

<EXP>. {
  IncPosition ();
  yylval.character = yytext[0];
  return (CHARACTER);
}

%%

void IncLevel ()
{
  if (Level == Levels)
  {
    Levels += 10;
    if (Positions)
    {
      Positions = (int*) realloc (Positions, Levels*sizeof(int));
    }
    else
    {
      Positions = (int*) malloc (Levels*sizeof(int));
    }
  }

  Level++;
  Positions[Level-1] = Position;
}

void DecLevel ()
{
  if (Level > 0) Level--;
}

void IncPosition ()
{
  if (!Level) IncLevel ();

  Position++;
}

void ResetPosition ()
{
  if (!Level) IncLevel ();

  Position = Positions[Level-1];
}

void RegLexStartup (char* text)
{
  yyin = stdin;
  yyout = stdout;

  BEGIN EXP;
  Level = 0;
  Position = 0;
  CurrentText = text;
  CurrentPos = 0;
}

int RegLexInput ()
{
  char c;

  c = CurrentText[CurrentPos];
  CurrentPos++;

  return (c);
}

static void RegLexUnput (int c)
{
  CurrentPos--;
  CurrentText[CurrentPos] = c;
}

