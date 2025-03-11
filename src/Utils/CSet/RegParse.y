%{
#include <stdio.h>

#include <Reg.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

int yyparse ();
void yyerror (const char* text);
int yylex ();

void RegLexStartup (char* text);
RegExp RegExpParse (char* text);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

static RegSet CurrentSet = 0;
static RegExp CurrentExp = 0;

%}


%union {
  char character;
  RegValueRange valueRange;
  RegCharRange charRange;
  RegExp exp;
}

%start exprs

%token <valueRange> VALUERANGE
%token STAR
%token QUESTION
%token PLUS
%token OPENPAR
%token CLOSEPAR
%token OPENNOSET
%token OPENSET
%token <character> CHARACTER
%token CLOSESET
%token <charRange> CHARRANGE
%token VERTICAL
%token NEWLINE
%token DOT
%token CARRET
%token DOLLAR

%left PRIMARY
%left VERTICAL

%type <exp> expr
%type <exp> nonemptyexpr
%type <exp> pattern
%type <exp> optcarret
%type <exp> pat
%type <exp> optdollar
%type <exp> unary
%type <exp> primary
%type <exp> set

%%

exprs :
    expr
    {
      CurrentExp = $1;
    }
  | exprs NEWLINE expr
    {
      CurrentExp = $3;
    }
  ;

expr :
    /* empty */
    {
      $$ = NULL;
    }
  | nonemptyexpr
    {
      $$ = $1;
    }
  ;

nonemptyexpr :
    pattern
    {
      $$ = $1;
    }
  | pattern VERTICAL pattern
    {
      if (($1 != NULL) && ($3 != NULL)) $$ = RegOrNew ($1, $3);
      else $$ = $1;
    }
  ;
    
pattern :
    optcarret pat optdollar
    {
      if ($1 != NULL) RegExpSetNext ($1, $2);
      if ($3 != NULL) RegExpSetNext ($2, $3);

      if ($1 != NULL) $$ = $1;
      else $$ = $2;
    }
  | CARRET
    {
      $$ = RegBolNew ();
    }
  | DOLLAR
    {
      $$ = RegEolNew ();
    }
  | CARRET DOLLAR
    {
      RegExp exp;

      exp = RegBolNew ();
      RegExpSetNext (exp, RegEolNew());
      $$ = exp;
    }
  ;

optcarret :
    /* empty */
    {
      $$ = NULL;
    }
  | CARRET
    {
      $$ = RegBolNew ();
    }
  ;

optdollar :
    /* empty */
    {
      $$ = NULL;
    }
  | DOLLAR
    {
      $$ = RegEolNew ();
    }
  ;

pat :
    unary
    {
      $$ = $1;
    }
  | pat unary
    {
      RegExpSetNext ($1, $2);
      $$ = $1;
    }
  ;

unary :
    primary
    {
      $$ = $1;
    }
  | primary STAR
    {
      if ($1 != NULL) $$ = RegUnaryNew (RegStar, $1);
      else $$ = NULL;
    }
  | primary QUESTION
    {
      if ($1 != NULL) $$ = RegUnaryNew (RegQuestion, $1);
      else $$ = NULL;
    }
  | primary PLUS
    {
      if ($1 != NULL) $$ = RegUnaryNew (RegPlus, $1);
      else $$ = NULL;
    }
  | primary VALUERANGE
    {
      $$ = $1;
    }
  ;

primary :
    CHARACTER %prec PRIMARY
    {
      $$ = RegCharNew ($1);
    }
  | set %prec PRIMARY
    {
      $$ = $1;
    }
  | DOT %prec PRIMARY
    {
      $$ = RegDotNew ();
    }
  | OPENPAR expr CLOSEPAR %prec PRIMARY
    {
      $$ = RegListNew ($2);
    }
  | error %prec PRIMARY
    {
      yyerror ("Bad primary");
      $$ = NULL;
    }
  ;

set :
    openset optchars CLOSESET
    {
      $$ = (RegExp) CurrentSet;
      CurrentSet = NULL;
    }
  | opennoset optchars CLOSESET
    {
      $$ = (RegExp) CurrentSet;
      CurrentSet = NULL;
    }
  ;

openset :
    OPENSET
    {
      CurrentSet = (RegSet) RegSetNew (RegSetOn);
    }
  ;

opennoset :
    OPENNOSET
    {
      CurrentSet = (RegSet) RegSetNew (RegSetOff);
    }
  ;

optchars :
    /* empty */
  | chars
  ;

chars :
    char
  | chars char
  ;

char :
    CHARACTER
    {
      RegSetAddChar (CurrentSet, $1);
    }
  | CHARRANGE
    {
      char c;

      for (c = $1->c1; c <= $1->c2; c++)
      {
        RegSetAddChar (CurrentSet, c);
      }
    }
  | error
    {
      yyerror ("error in set");
    }
  ;

%%


void yyerror (const char* text)
{
  fprintf (stderr, "Reg> %s\n", text);
}

int yywrap ()
{
  return (1);
}

RegExp RegExpParse (char* text)
{
  CurrentExp = 0;

  RegLexStartup (text);
  yyparse ();

  return (CurrentExp);
}

