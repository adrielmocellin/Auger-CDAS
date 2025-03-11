/*---------------------------------------------------------------------------*/
/*           Copyright (c) 1996 LAL Orsay, IN2P3-CNRS (France).              */
/*                                                                           */
/* Redistribution and use in source and binary forms, with or without        */
/* modification, are permitted provided that the following conditions        */
/* are met:                                                                  */
/* 1. Redistributions of source code must retain the above copyright         */
/*    notice, this list of conditions and the following disclaimer.          */
/* 2. Redistributions in binary form must reproduce the above copyright      */
/*    notice, this list of conditions and the following disclaimer in the    */
/*    documentation and/or other materials provided with the distribution.   */
/* 3. All advertising materials mentioning features or use of this software  */
/*    must display the following acknowledgement:                            */
/*      This product includes software developed by the Computer Application */
/*      Development Group at LAL Orsay (Laboratoire de l'Accelerateur        */
/*      Linaire - IN2P3-CNRS).                                               */
/* 4. Neither the name of the Institute nor of the Laboratory may be used    */
/*    to endorse or promote products derived from this software without      */
/*    specific prior written permission.                                     */
/*                                                                           */
/* THIS SOFTWARE IS PROVIDED BY THE LAL AND CONTRIBUTORS ``AS IS'' AND       */
/* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE     */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR        */
/* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LAL OR CONTRIBUTORS BE      */
/* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR       */
/* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF      */
/* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS  */
/* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   */
/* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   */
/* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF    */
/* THE POSSIBILITY OF SUCH DAMAGE.                                           */
/*---------------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <Reg.h>

RegExp RegExpParse (char* text);

/*------- Exp ---------------------*/

#define BadMove (-1)

static int RegExpLastIndex  = 0;
static int RegExpLastLength = 0;

/*--------------------------------------------------*/
RegExp RegExpNew (char* text)
/*--------------------------------------------------*/
{
  RegExp This;

  if (!text) return (0);

  This = RegExpParse (text);
  return (This);
}

/*--------------------------------------------------*/
void RegExpDelete (RegExp This)
/*--------------------------------------------------*/
{
  if (!This) return;

  if (This->next)
    {
      RegExpDelete (This->next);
      This->next = 0;
    }

  This->Class->clear (This);
  free (This);
}

/*--------------------------------------------------*/
void RegExpSetNext (RegExp This, RegExp exp)
/*--------------------------------------------------*/
{
  RegExp next;

  if (!This) return;

  if ((next = This->next)) RegExpSetNext (next, exp);
  else This->next = exp;
}

/*--------------------------------------------------*/
RegExp RegExpNext (RegExp This)
/*--------------------------------------------------*/
{
  if (!This) return (0);

  return (This->next);
}

/*--------------------------------------------------*/
int RegExpExecute (RegExp This, char* text, int pos)
/*--------------------------------------------------*/
{
  if (!This) return (0);
  if (!text) return (0);

  return (This->Class->execute (This, text, pos));
}

/*--------------------------------------------------*/
int RegExpMatch (RegExp This, char* text)
/*--------------------------------------------------*/
{
  int pos;
  int length;

  if (!This) return (0);
  if (!text) return (0);

  for (pos = 0; pos <= strlen(text); pos++)
    {
      length = RegExpExecute (This, text, pos);
      if (length != BadMove)
	{
	  RegExpLastIndex = pos;
	  RegExpLastLength = length;
	  return (1);
	}
    }
  RegExpLastIndex  = -1;
  RegExpLastLength = -1;
  return (0);
}

/*--------------------------------------------------*/
void RegExpDump (RegExp This)
/*--------------------------------------------------*/
{
  if (!This) return;

  This->Class->dump (This);
  RegExpDump (This->next);
}

/*--------------------------------------------------*/
int RegExpIndex ()
/*--------------------------------------------------*/
{
  return (RegExpLastIndex);
}

/*--------------------------------------------------*/
int RegExpLength ()
/*--------------------------------------------------*/
{
  return (RegExpLastLength);
}



/*------- Set ---------------------*/

static RegExpClassRec RegSetClass = {
  "RegSet",
  (RegFunction) RegSetClear,
  (RegExFunction) RegSetExecute,
  (RegFunction) RegSetDump
};

/*--------------------------------------------------*/
RegExp RegSetNew (RegSetMode mode)
/*--------------------------------------------------*/
{
  RegSet This;

  This = (RegSet) malloc (sizeof(RegSetRec));
  This->base.Class = &RegSetClass;
  This->base.next = 0;

  This->mode = mode;
  This->set = 0;

  return ((RegExp) This);
}

/*--------------------------------------------------*/
void RegSetClear (RegSet This)
/*--------------------------------------------------*/
{
  if (!This) return;

  if (This->set)
    {
      free (This->set);
      This->set = 0;
    }
}

/*--------------------------------------------------*/
void RegSetAddChar (RegSet This, char c)
/*--------------------------------------------------*/
{
  int length;

  if (!This) return;

  if (!This->set)
    {
      length = 1;
      This->set = (char*) malloc (length + 1);
    }
  else
    {
      length = strlen (This->set) + 1;
      This->set = (char*) realloc (This->set, length + 1);
    }
  
  This->set[length-1] = c;
  This->set[length]   = 0;
}

/*--------------------------------------------------*/
int RegSetExecute (RegSet This, char* text, int pos)
/*--------------------------------------------------*/
{
  char c;
  char* inSet;
  int move = 0;

  if (pos >= strlen(text)) return (BadMove);

  c = text[pos];
  inSet = strchr (This->set, c);

  if ((inSet  && (This->mode == RegSetOn)) ||
      (!inSet && (This->mode == RegSetOff)))
    {
      int moveNext;
      
      move = 1;
      
      moveNext = RegExpExecute (RegExpNext((RegExp) This), text, pos + move);
      if (moveNext != BadMove) move += moveNext;
      else move = BadMove;
    }
  else move = BadMove;
  
  return (move);
}

/*--------------------------------------------------*/
void RegSetDump (RegSet This)
/*--------------------------------------------------*/
{
  if (!This) return;

  if (This->set) printf ("[%s]", This->set);
  else printf ("[]");
}


/*------- Char ---------------------*/

static RegExpClassRec RegCharClass = {
  "RegChar",
  (RegFunction) RegCharClear,
  (RegExFunction) RegCharExecute,
  (RegFunction) RegCharDump
};

/*--------------------------------------------------*/
RegExp RegCharNew (char c)
/*--------------------------------------------------*/
{
  RegChar This;

  This = (RegChar) malloc (sizeof(RegCharRec));
  This->base.Class = &RegCharClass;
  This->base.next = 0;

  This->c = c;

  return ((RegExp) This);
}

/*--------------------------------------------------*/
void RegCharClear (RegChar This)
/*--------------------------------------------------*/
{
  if (!This) return;
}

/*--------------------------------------------------*/
int RegCharExecute (RegChar This, char* text, int pos)
/*--------------------------------------------------*/
{
  char c;
  int move = 0;

  if (pos >= strlen(text)) return (BadMove);

  c = text[pos];

  if (c == This->c)
    {
      int localMove = 1;
      
      move = RegExpExecute (RegExpNext((RegExp) This), text, pos + localMove);
      if (move != BadMove) move += localMove;
      else move = BadMove;
    }
  else move = BadMove;

  return (move);
}

/*--------------------------------------------------*/
void RegCharDump (RegChar This)
/*--------------------------------------------------*/
{
  if (!This) return;

  printf ("%c", This->c);
}


/*------- Bol ---------------------*/

static RegExpClassRec RegBolClass = {
  "RegBol",
  (RegFunction) RegBolClear,
  (RegExFunction) RegBolExecute,
  (RegFunction) RegBolDump
};

/*--------------------------------------------------*/
RegExp RegBolNew ()
/*--------------------------------------------------*/
{
  RegBol This;

  This = (RegBol) malloc (sizeof(RegBolRec));
  This->base.Class = &RegBolClass;
  This->base.next = 0;

  return ((RegExp) This);
}

/*--------------------------------------------------*/
void RegBolClear (RegBol This)
/*--------------------------------------------------*/
{
  if (!This) return;
}

/*--------------------------------------------------*/
int RegBolExecute (RegBol This, char* text, int pos)
/*--------------------------------------------------*/
{
  if (pos != 0) return (BadMove);

  return (RegExpExecute (RegExpNext((RegExp) This), text, pos));
}

/*--------------------------------------------------*/
void RegBolDump (RegBol This)
/*--------------------------------------------------*/
{
  if (!This) return;

  printf ("^");
}


/*------- Eol ---------------------*/

static RegExpClassRec RegEolClass = {
  "RegEol",
  (RegFunction) RegEolClear,
  (RegExFunction) RegEolExecute,
  (RegFunction) RegEolDump
};

/*--------------------------------------------------*/
RegExp RegEolNew ()
/*--------------------------------------------------*/
{
  RegEol This;

  This = (RegEol) malloc (sizeof(RegEolRec));
  This->base.Class = &RegEolClass;
  This->base.next = 0;

  return ((RegExp) This);
}

/*--------------------------------------------------*/
void RegEolClear (RegEol This)
/*--------------------------------------------------*/
{
  if (!This) return;
}

/*--------------------------------------------------*/
int RegEolExecute (RegEol This, char* text, int pos)
/*--------------------------------------------------*/
{
  if (pos != strlen(text)) return (BadMove);
  return (0);
}

/*--------------------------------------------------*/
void RegEolDump (RegEol This)
/*--------------------------------------------------*/
{
  if (!This) return;

  printf ("$");
}


/*------- Dot ---------------------*/

static RegExpClassRec RegDotClass = {
  "RegDot",
  (RegFunction) RegDotClear,
  (RegExFunction) RegDotExecute,
  (RegFunction) RegDotDump
};

/*--------------------------------------------------*/
RegExp RegDotNew ()
/*--------------------------------------------------*/
{
  RegDot This;

  This = (RegDot) malloc (sizeof(RegDotRec));
  This->base.Class = &RegDotClass;
  This->base.next = 0;

  return ((RegExp) This);
}

/*--------------------------------------------------*/
void RegDotClear (RegDot This)
/*--------------------------------------------------*/
{
  if (!This) return;
}

/*--------------------------------------------------*/
int RegDotExecute (RegDot This, char* text, int pos)
/*--------------------------------------------------*/
{
  int move;

  if (pos >= strlen(text)) return (BadMove);
  move = RegExpExecute (RegExpNext((RegExp) This), text, pos + 1);
  if (move != BadMove) move++;
  return (move);
}

/*--------------------------------------------------*/
void RegDotDump (RegDot This)
/*--------------------------------------------------*/
{
  if (!This) return;

  printf (".");
}


/*------- Unary ---------------------*/

static RegExpClassRec RegUnaryClass = {
  "RegUnary",
  (RegFunction) RegUnaryClear,
  (RegExFunction) RegUnaryExecute,
  (RegFunction) RegUnaryDump
};

/*--------------------------------------------------*/
RegExp RegUnaryNew (RegUnaryOp op, RegExp exp)
/*--------------------------------------------------*/
{
  RegUnary This;

  This = (RegUnary) malloc (sizeof(RegUnaryRec));
  This->base.Class = &RegUnaryClass;
  This->base.next = 0;

  This->op = op;
  This->exp = exp;

  return ((RegExp) This);
}

/*--------------------------------------------------*/
void RegUnaryClear (RegUnary This)
/*--------------------------------------------------*/
{
  if (!This) return;
}

/*--------------------------------------------------*/
int RegUnaryExecute (RegUnary This, char* text, int pos)
/*--------------------------------------------------*/
{
  RegExp next;
  RegExp exp;
  int move = 0;

  next = RegExpNext ((RegExp) This);
  exp = This->exp;

  switch (This->op)
    {
    case RegStar :
      for (;;)
	{
	  int localMove;
	  
	  localMove = RegExpExecute (exp, text, pos);

	  if (localMove == BadMove)
	    {
	      /* Zero occurrence est autorise */

	      if (next)
		{
		  localMove = RegExpExecute (next, text, pos);
		  if (localMove != BadMove) move += localMove;
		  else move = BadMove;
		}

	      break;
	    }
	  else
	    {
	      pos += localMove;
	      move += localMove;
	    }
	}
      break;
    case RegQuestion :
      {
	int localMove;
	  
	localMove = RegExpExecute (exp, text, pos);
	if (localMove != BadMove)
	  {
	    pos += localMove;
	    move += localMove;
	  }

	if (next)
	  {
	    localMove = RegExpExecute (next, text, pos);
	    if (localMove != BadMove) move += localMove;
	    else move = BadMove;
	  }
      }
      break;
    case RegPlus :
      {
	int localMove;

	localMove = RegExpExecute (exp, text, pos);
	if (localMove != BadMove)
	  {
	    move += localMove;
	    pos += localMove;
	  }
	else
	  {
	    move = BadMove;
	    break;
	  }
      }
	  
      for (;;)
	{
	  int localMove;
	  
	  localMove = RegExpExecute (exp, text, pos);

	  if (localMove == BadMove)
	    {
	      /* Zero occurrence est autorise */

	      if (next)
		{
		  localMove = RegExpExecute (next, text, pos);
		  if (localMove != BadMove) move += localMove;
		  else move = BadMove;
		}

	      break;
	    }
	  else
	    {
	      pos += localMove;
	      move += localMove;
	    }
	}
      break;
    }
  
  return (move);
}

/*--------------------------------------------------*/
void RegUnaryDump (RegUnary This)
/*--------------------------------------------------*/
{
  RegExp exp;

  if (!This) return;

  exp = This->exp;

  printf ("(");
  RegExpDump (exp);
  printf (")");

  switch (This->op)
    {
    case RegStar :
      printf ("*");
      break;
    case RegQuestion :
      printf ("?");
      break;
    case RegPlus :
      printf ("+");
      break;
    }
}


/*------- List ---------------------*/

static RegExpClassRec RegListClass = {
  "RegList",
  (RegFunction) RegListClear,
  (RegExFunction) RegListExecute,
  (RegFunction) RegListDump
};

/*--------------------------------------------------*/
RegExp RegListNew (RegExp exp)
/*--------------------------------------------------*/
{
  RegList This;

  This = (RegList) malloc (sizeof(RegListRec));
  This->base.Class = &RegListClass;
  This->base.next = 0;

  This->first = exp;

  return ((RegExp) This);
}

/*--------------------------------------------------*/
void RegListClear (RegList This)
/*--------------------------------------------------*/
{
  if (!This) return;

  if (This->first)
    {
      RegExpDelete (This->first);
      This->first = 0;
    }
}

/*--------------------------------------------------*/
int RegListExecute (RegList This, char* text, int pos)
/*--------------------------------------------------*/
{
  int move = 0;

  move = RegExpExecute (This->first, text, pos);
  if (move != BadMove)
    {
      int moveNext;
      
      moveNext = RegExpExecute (RegExpNext ((RegExp) This), text, pos + move);
      if (moveNext != BadMove) move += moveNext;
      else move = BadMove;
    }
  
  return (move);
}

/*--------------------------------------------------*/
void RegListDump (RegList This)
/*--------------------------------------------------*/
{
  if (!This) return;

  printf ("(");
  RegExpDump (This->first);
  printf (")");
}


/*------- Or ---------------------*/

static RegExpClassRec RegOrClass = {
  "RegOr",
  (RegFunction) RegOrClear,
  (RegExFunction) RegOrExecute,
  (RegFunction) RegOrDump,
};

/*--------------------------------------------------*/
RegExp RegOrNew (RegExp exp1, RegExp exp2)
/*--------------------------------------------------*/
{
  RegOr This;

  This = (RegOr) malloc (sizeof(RegOrRec));
  This->base.Class = &RegOrClass;
  This->base.next = 0;

  This->left = exp1;
  This->right = exp2;

  return ((RegExp) This);
}

/*--------------------------------------------------*/
void RegOrClear (RegOr This)
/*--------------------------------------------------*/
{
  if (!This) return;

  if (This->left)
    {
      RegExpDelete (This->left);
      This->left = 0;
    }

  if (This->right)
    {
      RegExpDelete (This->right);
      This->right = 0;
    }
}

/*--------------------------------------------------*/
int RegOrExecute (RegOr This, char* text, int pos)
/*--------------------------------------------------*/
{
  int moveLeft = 0;
  int moveRight = 0;
  int move = 0;

  moveLeft = RegExpExecute (This->left, text, pos);
  moveRight = RegExpExecute (This->right, text, pos);
  
  if ((moveLeft == BadMove) &&
      (moveRight == BadMove)) return (BadMove);
  
  if (moveLeft == BadMove) move = moveRight;
  else if (moveRight == BadMove) move = moveLeft;
  else
    {
      move = (moveLeft > moveRight) ? moveLeft : moveRight;
    }
  
  if (move != BadMove)
    {
      int moveNext = 0;
      
      moveNext = RegExpExecute (RegExpNext ((RegExp) This), text, pos + move);
      if (moveNext != BadMove) move += moveNext;
      else move = BadMove;
    }
  
  return (move);
}

/*--------------------------------------------------*/
void RegOrDump (RegOr This)
/*--------------------------------------------------*/
{
  if (!This) return;

  printf ("(");
  RegExpDump (This->left);
  printf (")|(");
  RegExpDump (This->right);
  printf (")");
}


