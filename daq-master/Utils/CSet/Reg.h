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
#ifndef __Reg__
#define __Reg__

typedef struct _RegValueRangeRec* RegValueRange;
typedef struct _RegCharRangeRec* RegCharRange;

typedef struct _RegExpRec* RegExp;
typedef struct _RegExpClassRec* RegExpClass;

typedef void (*RegFunction)(RegExp This);
typedef int (*RegExFunction)(RegExp This, char* text, int pos);

typedef struct _RegValueRangeRec
{
  int val1;
  int val2;
} RegValueRangeRec;

typedef struct _RegCharRangeRec
{
  char c1;
  char c2;
} RegCharRangeRec;

/*------- Exp ---------------------*/

typedef struct _RegExpClassRec
{
  char* name;
  RegFunction clear;
  RegExFunction execute;
  RegFunction dump;
} RegExpClassRec;

typedef struct _RegExpRec
{
  RegExpClass Class;

  RegExp next;
} RegExpRec;

/*------- Set ---------------------*/

typedef struct _RegSetRec* RegSet;

typedef enum
{
  RegSetOn,
  RegSetOff
} RegSetMode;

typedef struct _RegSetRec
{
  RegExpRec base;

  RegSetMode mode;
  char* set;
} RegSetRec;

/*------- Char ---------------------*/

typedef struct _RegCharRec* RegChar;

typedef struct _RegCharRec
{
  RegExpRec base;

  char c;
} RegCharRec;

/*------- Bol ---------------------*/

typedef struct _RegBolRec* RegBol;

typedef struct _RegBolRec
{
  RegExpRec base;
} RegBolRec;

/*------- Eol ---------------------*/

typedef struct _RegEolRec* RegEol;

typedef struct _RegEolRec
{
  RegExpRec base;
} RegEolRec;

/*------- Dot ---------------------*/

typedef struct _RegDotRec* RegDot;

typedef struct _RegDotRec
{
  RegExpRec base;
} RegDotRec;

/*------- Unary ---------------------*/

typedef struct _RegUnaryRec* RegUnary;

typedef enum
{
  RegStar,
  RegQuestion,
  RegPlus
} RegUnaryOp;

typedef struct _RegUnaryRec
{
  RegExpRec base;

  RegExp exp;
  RegUnaryOp op;
} RegUnaryRec;

/*------- List ---------------------*/

typedef struct _RegListRec* RegList;

typedef struct _RegListRec
{
  RegExpRec base;

  RegExp first;
} RegListRec;

/*------- Or ---------------------*/

typedef struct _RegOrRec* RegOr;

typedef struct _RegOrRec
{
  RegExpRec base;

  RegExp left;
  RegExp right;
} RegOrRec;

#include <Reg.ph>

#endif

