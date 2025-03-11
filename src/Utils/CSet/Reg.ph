#ifndef __Reg_ph__
#define __Reg_ph__

#ifdef __cplusplus
extern "C" {
#endif

RegExp RegExpNew  (char* text);
void RegExpDelete  (RegExp This);
void RegExpSetNext  (RegExp This, RegExp exp);
RegExp RegExpNext  (RegExp This);
int RegExpExecute  (RegExp This, char* text, int pos);
int RegExpMatch  (RegExp This, char* text);
void RegExpDump  (RegExp This);
int RegExpIndex  ();
int RegExpLength  ();
RegExp RegSetNew  (RegSetMode mode);
void RegSetClear  (RegSet This);
void RegSetAddChar  (RegSet This, char c);
int RegSetExecute  (RegSet This, char* text, int pos);
void RegSetDump  (RegSet This);
RegExp RegCharNew  (char c);
void RegCharClear  (RegChar This);
int RegCharExecute  (RegChar This, char* text, int pos);
void RegCharDump  (RegChar This);
RegExp RegBolNew  ();
void RegBolClear  (RegBol This);
int RegBolExecute  (RegBol This, char* text, int pos);
void RegBolDump  (RegBol This);
RegExp RegEolNew  ();
void RegEolClear  (RegEol This);
int RegEolExecute  (RegEol This, char* text, int pos);
void RegEolDump  (RegEol This);
RegExp RegDotNew  ();
void RegDotClear  (RegDot This);
int RegDotExecute  (RegDot This, char* text, int pos);
void RegDotDump  (RegDot This);
RegExp RegUnaryNew  (RegUnaryOp op, RegExp exp);
void RegUnaryClear  (RegUnary This);
int RegUnaryExecute  (RegUnary This, char* text, int pos);
void RegUnaryDump  (RegUnary This);
RegExp RegListNew  (RegExp exp);
void RegListClear  (RegList This);
int RegListExecute  (RegList This, char* text, int pos);
void RegListDump  (RegList This);
RegExp RegOrNew  (RegExp exp1, RegExp exp2);
void RegOrClear  (RegOr This);
int RegOrExecute  (RegOr This, char* text, int pos);
void RegOrDump  (RegOr This);

#ifdef __cplusplus
}
#endif

#endif

