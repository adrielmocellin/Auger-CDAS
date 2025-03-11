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
#ifndef __GCC_hxx__
#define __GCC_hxx__

#include <GC.h>

class GCCClass
{
public:
  ~GCCClass ();

  static GCClass base;

  void init ();
};

class GCCObject
{
public:
  GCCObject ();
  virtual ~GCCObject ();

  static GCCClass gc_class;
  static void deleteObject (GCCObject* object);
};

#define GCCPOINTER(TTT) class TTT;\
\
class TTT ## Pointer \
{\
public:\
  TTT ## Pointer () { pointer = 0; }\
  TTT ## Pointer (const TTT* object) { pointer = 0; GCReferenceSet ((void**) &pointer, 0, object); }\
  TTT ## Pointer (const TTT* object, const void* root) { pointer = 0; GCReferenceSet ((void**) &pointer, root, object); }\
  ~TTT ## Pointer () { GCReferenceSet ((void**) &pointer, 0, 0); }\
\
  TTT& operator * () const { return (*pointer); }\
  TTT* operator -> () const { return (pointer); }\
  TTT ## Pointer& operator = (const TTT* object) { GCReferenceSet ((void**) &pointer, 0, object); return (*this); }\
  void set (const TTT* object, const void* root) { GCReferenceSet ((void**) &pointer, root, object); }\
  int operator == (void* object) { return (pointer == object); }\
  operator TTT* () { return (pointer); }\
\
  int operator ! () const { return (pointer == 0); }\
\
private:\
  TTT* pointer;\
}

#endif
