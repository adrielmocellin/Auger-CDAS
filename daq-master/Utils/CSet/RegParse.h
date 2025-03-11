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
extern YYSTYPE RegYYlval;
