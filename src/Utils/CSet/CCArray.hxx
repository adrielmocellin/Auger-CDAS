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
#ifndef __CArray_hxx__
#define __CArray_hxx__

#include <GCC.hxx>
#include <CSet.h>

GCCPOINTER (CCArray);
GCCPOINTER (CCStack);
GCCPOINTER (CCQueue);
GCCPOINTER (CCLList);
GCCPOINTER (CCHasher);
// GCCPOINTER (CCIterator);

//---------------------------------------------------------------

class CCIterator  // : public GCCObject
{
  public:
  CCIterator (CIterator it) { _it = it; }
  ~CCIterator () { CIteratorDelete (_it); }
  
  int finished () { return (CIteratorFinished (_it)); }
  void first () { CIteratorFirst (_it); }
  void forth () { CIteratorForth (_it); }
  void* item () { return (CIteratorItem (_it)); }
  void stop () { CIteratorStop (_it); }

  CIterator iterator () { return (_it); }
  
  protected:
  CIterator _it;
};

//---------------------------------------------------------------

class CCArray : public GCCObject
{
  public:
  CCArray () { GCReferenceSet ((void**) &_array, this, CArrayNew ()); }
  ~CCArray () { GCReferenceSet ((void**) &_array, this, 0); }
  void erase () { CArrayErase (_array); }
  int count () { return (CArrayCount (_array)); }
  void add (void* any_object) { CArrayAdd (_array, any_object); }
  void remove (void* any_object) { CArrayRemove (_array, any_object); }
  void* item (CCIterator* it) { return (CArrayItem (_array, it->iterator())); }
  void* find (void* any_object) { return (CArrayFind (_array, any_object)); }
  CCIterator* iterator () { return (new CCIterator (CArrayGiveIterator (_array))); }
  CCIterator* backward_iterator () { return (new CCIterator (CArrayGiveBackwardIterator (_array))); }

  static void cleanup () { CArrayCleanup (); }

  CArray array () { return (_array); }

  private:
  CArray _array;
};

//---------------------------------------------------------------

class CCStack : public GCCObject
{
  public:
  CCStack () { GCReferenceSet ((void**) &_stack, this, CStackNew ()); }
  ~CCStack () { GCReferenceSet ((void**) &_stack, this, 0); }
  void erase () { CStackErase (_stack); }
  int count () { return (CStackCount (_stack)); }
  void push (void* any_object) { CStackPush (_stack, any_object); }
  void* pop () { return (CStackPop (_stack)); }
  void* top () { return (CStackTop (_stack)); }

  private:
  CStack _stack;
};

//---------------------------------------------------------------

class CCQueue : public GCCObject
{
  public:
  CCQueue () { GCReferenceSet ((void**) &_queue, this, CQueueNew ()); }
  ~CCQueue () { GCReferenceSet ((void**) &_queue, this, 0); }
  void erase () { CQueueErase (_queue); }
  int count () { return (CQueueCount (_queue)); }
  void push (void* any_object) { CQueuePush (_queue, any_object); }
  void* pop () { return (CQueuePop (_queue)); }
  void* top () { return (CQueueTop (_queue)); }

  private:
  CQueue _queue;
};

//---------------------------------------------------------------

class CCLList : public GCCObject
{
  public:
  CCLList () { GCReferenceSet ((void**) &_list, this, CLListNew ()); }
  ~CCLList () { GCReferenceSet ((void**) &_list, this, 0); }
  void erase () { CLListErase (_list); }
  int count () { return (CLListCount (_list)); }
  void add (void* any_object) { CLListAdd (_list, any_object); }
  void remove (void* any_object) { CLListRemove (_list, any_object); }
  void* find (void* any_object) { return (CLListFind (_list, any_object)); }
  CCIterator* iterator () { return (new CCIterator (CLListGiveIterator (_list))); }
  CCIterator* backward_iterator () { return (new CCIterator (CLListGiveBackwardIterator (_list))); }

  private:
  CLList _list;
};

//---------------------------------------------------------------

class CCHasher : public GCCObject
{
public:
  ~CCHasher () { GCReferenceSet ((void**) &_hasher, this, 0); }
  void erase () { CHasherErase (_hasher); }
  void add (void* any_object) { CHasherAdd (_hasher, any_object); }
  void remove (void* any_object) { CHasherRemove (_hasher, any_object); }
  void* find (void* value) { return (CHasherFind (_hasher, value)); }
  CCIterator* iterator () { return (new CCIterator (CHasherGiveIterator (_hasher))); }

  CHasher hasher () { return (_hasher); }
  
protected:
  
  CCHasher () {}
  
  CHasher _hasher;
};

class CCPtrHasher : public CCHasher
{
public:
  CCPtrHasher (int size, CValuator valuator) { GCReferenceSet ((void**) &_hasher, this, CPtrHasherNew (size, valuator)); }
};

class CCStrHasher : public CCHasher
{
public:
  CCStrHasher (int size, CValuator valuator) { GCReferenceSet ((void**) &_hasher, this, CStrHasherNew (size, valuator)); }
};

#endif

