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
/*
        lists.c
         Created           : 18-OCT-1989 by Christian Arnault
*/
 
/*-----------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <LkListP.h>
/*-----------------------------------------------------------------------*/

#ifdef __cplusplus
typedef int (*LkListSortFunction) (const void *, const void *);
#else
typedef int (*LkListSortFunction) ();
#endif

static int AllLkListCount   = 0;
static int LkListArrayCount = 0;
static int LkEntryCount     = 0;
static int LkIteratorCount  = 0;


/*=======================================================================*/
/*                                                                       */
/*  LkList                                                               */
/*                                                                       */
/*=======================================================================*/

/*-----------------------------------------------------------------------*/
void LkListCleanup ()
/*-----------------------------------------------------------------------*/
{
  if (AllLkListCount != 0)
    {
      printf ("AllLkListCount = %d\n", AllLkListCount);
    }
  if (LkListArrayCount != 0)
    {
      printf ("LkListArrayCount = %d\n", LkListArrayCount);
    }
  if (LkEntryCount != 0)
    {
      printf ("LkEntryCount = %d\n", LkEntryCount);
    }
  if (LkIteratorCount != 0)
    {
      printf ("LkIteratorCount = %d\n", LkIteratorCount);
    }
}

/*-----------------------------------------------------------------------*/
LkList LkListNew ()
/*-----------------------------------------------------------------------*/
{
  LkList This;
  
  This = (LkList) malloc (sizeof (LkListRec));
  AllLkListCount++;
  
  LkListInit (This);

  return (This);
}

/*-----------------------------------------------------------------------*/
void LkListDelete (LkList This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;
  if (_LkListFirstEntry (This))
    {
      printf ("LkList> Trying to delete a non empty list. Clear it first\n");
      return;
    }
  
  free ((char*) This);
  AllLkListCount--;
}

/*-----------------------------------------------------------------------*/
void LkListInit (LkList This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;
  
  _LkListSetFirstEntry (This, 0);
  _LkListSetLastEntry (This, 0);
}
 
/*-----------------------------------------------------------------------*/
void LkListConnectEntry (LkList This, LkEntry entry)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;
  if (!entry) return;

  if (_LkListLastEntry (This))
    {
      _LkEntrySetNext (_LkListLastEntry (This), entry);
    }
  else
    {
      _LkListSetFirstEntry (This, entry);
    }
  _LkEntrySetPrevious (entry, _LkListLastEntry (This));
  _LkEntrySetNext (entry, 0);
  _LkEntrySetFather (entry, This);

  _LkListSetLastEntry (This, entry);
}

/*-----------------------------------------------------------------------*/
void LkListTransferEntries (LkList This, LkList dest)
/*-----------------------------------------------------------------------*/
{
  LkEntry entry;
  LkEntry previous;
  
  if (!This) return;
  if (!dest) return;
  if (This == dest) return;
  
  entry = _LkListFirstEntry (This);
  if (!entry) return;

  while (entry)
    {
      _LkEntrySetFather (entry, dest);
      entry = _LkEntryNext (entry);
    }

  if ((previous = _LkListLastEntry (dest)))
    {
      _LkEntrySetNext (previous, _LkListFirstEntry (This));
      _LkEntrySetPrevious (_LkListFirstEntry (This), previous);
      _LkListSetLastEntry (dest, _LkListLastEntry (This));
    }
  else
    {
      _LkListSetFirstEntry (dest, _LkListFirstEntry (This));
      _LkListSetLastEntry (dest, _LkListLastEntry (This));
    }
  _LkListSetFirstEntry (This, 0);
  _LkListSetLastEntry (This, 0);
}

/*-----------------------------------------------------------------------*/
void LkListClear (LkList This, LkFunction function)
/*-----------------------------------------------------------------------*/
{
  LkEntry entry;

  if (!This) return;

  if (function)
    {
      while ((entry = _LkListFirstEntry (This)))
        {
          function (entry);
          LkEntryDelete (entry);
        }
    }
  else
    {
      while ((entry = _LkListFirstEntry (This)))
        {
          LkEntryDelete (entry);
        }
    }
}

/*-----------------------------------------------------------------------*/
void LkListProcessForward (LkList This, LkFunction function)
/*-----------------------------------------------------------------------*/
{
  LkEntry entry;

  if (!This) return;
  if (!function) return;

  entry = _LkListFirstEntry (This);
  while (entry)
    {
      function (entry);
      entry = _LkEntryNext (entry);
    }
}

/*-----------------------------------------------------------------------*/
void LkListProcessBackward (LkList This, LkFunction function)
/*-----------------------------------------------------------------------*/
{
  LkEntry entry;

  if (!This) return;
  if (!function) return;
  
  entry = _LkListLastEntry (This);
  while (entry)
    {
      function (entry);
      entry = _LkEntryPrevious (entry);
    }
}

/*-----------------------------------------------------------------------*/
void LkListSort (LkList This, LkFunction function)
/*-----------------------------------------------------------------------*/
{
  LkEntry entry;
  LkEntry* array;
  int count;
  int num;

  if (!This) return;
  if (!function) return;
  count = LkListCount (This);
  if (!count) return;

  array = (LkEntry*) malloc (count * sizeof (LkEntry));
  LkListArrayCount++;

  entry = _LkListFirstEntry (This);
  num = 0;
  while (entry)
    {
      array[num] = entry;
      num++;
      entry = _LkEntryNext (entry);
    }
  qsort (array, count, sizeof(LkEntry), (LkListSortFunction) function);
  
  LkEntryMove (array[0], 0, _LkListFirstEntry (This));
  for (num = 1; num < count; num++)
    {
      LkEntryMove (array[num], array[num-1], 0);
    }
  
  free ((char*) array);
  LkListArrayCount--;
}

/*-----------------------------------------------------------------------*/
int LkListContains (LkList This, LkEntry entry)
/*-----------------------------------------------------------------------*/
{
  if (!This) return (0);
  if (!entry) return (0);

  return ((This == _LkEntryFather (entry)));
}

/*-----------------------------------------------------------------------*/
int LkListCount (LkList This)
/*-----------------------------------------------------------------------*/
{
  LkEntry entry;
  int count = 0;

  if (!This) return (0);

  entry = _LkListFirstEntry (This);
  while (entry)
    {
      count++;
      entry = _LkEntryNext (entry);
    }
  return (count);
}

/*-----------------------------------------------------------------------*/
LkEntry LkListFirstEntry (LkList This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return (0);
  return (This->firstEntry);
}

/*-----------------------------------------------------------------------*/
LkEntry LkListLastEntry (LkList This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return (0);
  return (This->lastEntry);
}

/*=======================================================================*/
/*                                                                       */
/*  LkEntry                                                              */
/*                                                                       */
/*=======================================================================*/

/*-----------------------------------------------------------------------*/
LkEntry LkEntryNew (LkList list, int size)
/*-----------------------------------------------------------------------*/
{
  LkEntry This;

  This = (LkEntry) malloc (size);
  LkEntryCount++;
  
  if (list)
    {
      if (_LkListFirstEntry (list))
        {
          _LkEntrySetNext (_LkListLastEntry (list), This);
        }
      else
        {
          _LkListSetFirstEntry (list, This);
        }
      _LkEntrySetPrevious (This, _LkListLastEntry (list));
      _LkListSetLastEntry (list, This);
    }
  else
    {
      _LkEntrySetPrevious (This, 0);
    }
  _LkEntrySetNext (This, 0);
  _LkEntrySetFather (This, list);
  
  return (This);
}

/*-----------------------------------------------------------------------*/
LkEntry LkEntryInsertNew (LkEntry before, LkEntry after, int size)
/*-----------------------------------------------------------------------*/
{
  LkList list;
  LkEntry This;

  /* syntax check */
    
  if (!before && !after) return 0;
  if (!before) before = _LkEntryPrevious (after);
  if (!after) after = _LkEntryNext (before);
  
  if (before) list = _LkEntryFather (before);
  else list = _LkEntryFather (after);

  This = (LkEntry) malloc (size);
  LkEntryCount++;
  
  if (before) _LkEntrySetNext (before, This);
  else _LkListSetFirstEntry (list, This);
  
  if (after) _LkEntrySetPrevious (after, This);
  else _LkListSetLastEntry (list, This);
  
  _LkEntrySetPrevious (This, before);
  _LkEntrySetNext (This, after);
  _LkEntrySetFather (This, list);
  
  return (This);
}


/*-----------------------------------------------------------------------*/
void LkEntryInit (LkEntry This, LkList list)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;

  _LkEntrySetPrevious (This, 0);
  _LkEntrySetNext (This, 0);
  _LkEntrySetFather (This, 0);
  
  if (list) LkListConnectEntry (list, This);
}

/*-----------------------------------------------------------------------*/
void LkEntryDelete (LkEntry This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;

  LkEntryDisconnect (This);
  free ((char*) This);
  LkEntryCount--;
}

/*-----------------------------------------------------------------------*/
void LkEntryMove (LkEntry This, LkEntry before, LkEntry after)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;
  LkEntryTransfer (This, _LkEntryFather (This), before, after);
}


/*-----------------------------------------------------------------------*/
void LkEntryDisconnect (LkEntry This)
/*-----------------------------------------------------------------------*/
{
  LkList list;
  LkEntry prev;
  LkEntry next;

  if (!This) return;
  list = _LkEntryFather (This);
  if (!list) return;
  
  prev = _LkEntryPrevious (This);
  next = _LkEntryNext (This);
  
  if (prev) _LkEntrySetNext (prev, next);
  else _LkListSetFirstEntry (list, next);
  
  if (next) _LkEntrySetPrevious (next, prev);
  else _LkListSetLastEntry (list, prev);

  _LkEntrySetPrevious (This, 0);
  _LkEntrySetNext (This, 0);
  _LkEntrySetFather (This, 0);
}

/*-----------------------------------------------------------------------*/
void LkEntryTransfer (LkEntry This, LkList destList, LkEntry before,
  LkEntry after)
/*-----------------------------------------------------------------------*/
{
  /* syntax check */

  if (before)
    {
      if (destList != _LkEntryFather (before)) return;
      if (before == This) return;
    }
  if (after)
    {
      if (destList != _LkEntryFather (after)) return;
      if (after == This) return;
    }
  if (before && after)
    {
      if (_LkEntryNext (before) != after ||
          before != _LkEntryPrevious (after)) return;
    }

  LkEntryDisconnect (This);
  
  if (!before && !after) after = _LkListFirstEntry (destList);
  if (!before && after) before = _LkEntryPrevious (after);
  if (!after && before) after  = _LkEntryNext (before);
  
  /* Now put entry back. */
  
  if (before) _LkEntrySetNext (before, This);
  else _LkListSetFirstEntry (destList, This);
  
  if (after) _LkEntrySetPrevious (after, This);
  else _LkListSetLastEntry (destList, This);
  
  _LkEntrySetNext (This, after);
  _LkEntrySetPrevious (This, before);
  _LkEntrySetFather (This, destList);
}


/*-----------------------------------------------------------------------*/
int LkEntryPosition (LkEntry This)
/*-----------------------------------------------------------------------*/
{
  LkEntry entry;
  int count = 0;

  if (!This) return (-1);

  entry = This;
  while ((entry = _LkEntryPrevious (entry))) count++;
  return (count);
}

/*-----------------------------------------------------------------------*/
LkEntry LkEntryPrevious (LkEntry This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return (0);
  return (This->prevEntry);
}

/*-----------------------------------------------------------------------*/
LkEntry LkEntryNext (LkEntry This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return (0);
  return (This->nextEntry);
}

/*-----------------------------------------------------------------------*/
LkList LkEntryFather (LkEntry This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return (0);
  return (This->fatherList);
}

/*=======================================================================*/
/*                                                                       */
/*  LkIterator                                                           */
/*                                                                       */
/*=======================================================================*/

/*-----------------------------------------------------------------------*/
LkIterator LkIteratorNew (LkList list)
/*-----------------------------------------------------------------------*/
{
  LkIterator This;

  This = (LkIterator) malloc (sizeof(LkIteratorRec));
  LkIteratorCount++;
  
  This->list = list;
  This->current = 0;

  return (This);
}

/*-----------------------------------------------------------------------*/
void LkIteratorDelete (LkIterator This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;

  free ((char*) This);
  LkIteratorCount--;
}

/*-----------------------------------------------------------------------*/
void LkIteratorReset (LkIterator This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;

  This->current = 0;
}

/*-----------------------------------------------------------------------*/
LkEntry LkIteratorNext (LkIterator This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return (0);

  if (This->current)
    {
      This->current = _LkEntryNext (This->current);
    }
  else
    {
      if (!This->list) This->current = 0;
      else This->current = _LkListFirstEntry (This->list);
    }
  return (This->current);
}

/*-----------------------------------------------------------------------*/
LkEntry LkIteratorPrevious (LkIterator This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return (0);

  if (This->current)
    {
      This->current = _LkEntryPrevious (This->current);
    }
  else
    {
      if (!This->list) This->current = 0;
      else This->current = _LkListLastEntry (This->list);
    }
  return (This->current);
}

