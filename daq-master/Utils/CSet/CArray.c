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
/*----------------------------------------------------------*/
/* Author : ARNAULT Christian                               */
/*                                                          */
/* arnault@lal.in2p3.fr                                     */
/* Orsay, Laboratoire d'Accelerateur Lineaire (L.A.L.)      */
/*                                                          */
/*----------------------------------------------------------*/

/*- Header -------------------------------------------------*/
#ifdef CTHREADS
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <CSet.h>

/*----------------------------------------------------------*/

/*----------------------------------------------------------*/
static int             Initialized        = 0;

static GCClass  ArrayClass     = NULL;
static GCClass  StackClass     = NULL;
static GCClass  QueueClass     = NULL;
static GCClass  ListClass      = NULL;
static GCClass  HasherClass    = NULL;

static int             ArrayCount         = 0;
static int             ArrayDataCount     = 0;
static int             StackCount         = 0;
static int             StackEntryCount    = 0;
static int             QueueCount         = 0;
static int             QueueEntryCount    = 0;
static int             ListCount          = 0;
static int             ListEntryCount     = 0;
static int             IteratorCount      = 0;
static int             HasherCount        = 0;
static int             HasherTableCount   = 0;
static int             MallocCount        = 0;

#ifdef CTHREADS

static pthread_mutex_t ArrayMutex;
  
#endif

/*------------------------------------------------------------*/

static CIterator CForwardIteratorNew (CArray array);
static CIterator CBackwardIteratorNew (CArray array);
static CIterator CForwardLListIteratorNew (CLList list);
static CIterator CBackwardLListIteratorNew (CLList list);
static CIterator CHasherIteratorNew (CHasher hasher);

/*------------------------------------------------------------*/
/*          Array                                             */
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
typedef struct _CArrayRec
{
  void**  array;
  int     allocated;
  int     used;
  int     free;
} CArrayRec;

#define QUANTUM 20
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
static void CArrayDelete (CArray This);
static void CArrayClear (CArray This);
static int CArrayGetUsed (CArray This);
static void* CArrayFetchItemAt (CArray This, int index);

static int CLStrCompare (char* str1, char* str2);
static int CLStrCoder (char* ptr);
static int CLStrTester (char* ptr, char* other);

static int CStrCompare (char* str1, char* str2);
static void CStrLower (char* str);
static int CStrCoder (char* ptr);
static int CStrTester (char* ptr, char* other);

static int CPtrCoder (void* ptr);
static int CPtrTester (void* ptr, void* other);

static void CHasherStartup ();
static void CHasherCleanup ();

static void CStackStartup ();
static void CStackCleanup ();

static void CQueueStartup ();
static void CQueueCleanup ();

static void CLListStartup ();
static void CLListCleanup ();

static int CIteratorIndex (CIterator This);
/*------------------------------------------------------------*/

/*---------------------------------------------------*/
/*                                                   */
/*    malloc/free                                    */
/*                                                   */
/*---------------------------------------------------*/

/*------------------------------------------------------------*/
static void* CMalloc (size_t bytes)
/*------------------------------------------------------------*/
{
  void* result = NULL;

  MallocCount++;
  result = malloc (bytes);
  return (result);
}

/*------------------------------------------------------------*/
static void CFree (void* ptr)
/*------------------------------------------------------------*/
{
  MallocCount--;
  free (ptr);
}

/*------------------------------------------------------------*/
void CArrayStartup ()
/*------------------------------------------------------------*/
{
  if (!Initialized)
    {
      Initialized = 1;

#ifdef CTHREADS

      {
        pthread_mutexattr_t attr;
        
        pthread_mutexattr_create (&attr);
#ifdef CTHREADSV3
        pthread_mutexattr_setkind_np (&attr, MUTEX_RECURSIVE_NP);
        pthread_mutex_init (&ArrayMutex, attr);
#else
        pthread_mutexattr_setkind_np (&attr, PTHREAD_MUTEX_RECURSIVE_NP);
        pthread_mutex_init (&ArrayMutex, &attr);
#endif
      }
  
#endif

      ArrayClass = GCClassNew ("Array", (CMethod) CArrayDelete);
      CHasherStartup ();
      CStackStartup ();
      CQueueStartup ();
      CLListStartup ();
    }
}

/*------------------------------------------------------------*/
void CArrayCleanup ()
/*------------------------------------------------------------*/
{
  if (Initialized)
    {
      GCClassDelete (ArrayClass);
      ArrayClass = NULL;
      CStackCleanup ();
      CQueueCleanup ();
      CLListCleanup ();
      CHasherCleanup ();
      
#ifdef CTHREADS

      pthread_mutex_destroy (&ArrayMutex);
  
#endif

      Initialized = 0;
    }
}

/*------------------------------------------------------------*/
static void ArrayLock ()
/*------------------------------------------------------------*/
{
#ifdef CTHREADS
  pthread_mutex_lock (&ArrayMutex);
#endif
}
  
/*------------------------------------------------------------*/
static void ArrayUnlock ()
/*------------------------------------------------------------*/
{
#ifdef CTHREADS
  pthread_mutex_unlock (&ArrayMutex);
#endif
}
  

/*------------------------------------------------------------*/
void CArrayCheck ()
/*------------------------------------------------------------*/
{
  if (MallocCount != 0)
    {
      fprintf (stderr, "Bad count for MallocCount : %d\n",
               MallocCount);
      MallocCount = 0;
    }
  if (ArrayCount != 0)
    {
      fprintf (stderr, "Bad count for ArrayCount : %d\n",
               ArrayCount);
      ArrayCount = 0;
    }
  if (ArrayDataCount != 0)
    {
      fprintf (stderr, "Bad count for ArrayDataCount : %d\n",
               ArrayDataCount);
      ArrayDataCount = 0;
    }
  if (StackCount != 0)
    {
      fprintf (stderr, "Bad count for StackCount : %d\n",
               StackCount);
      StackCount = 0;
    }
  if (StackEntryCount != 0)
    {
      fprintf (stderr, "Bad count for StackEntryCount : %d\n",
               StackEntryCount);
      StackEntryCount = 0;
    }
  if (QueueCount != 0)
    {
      fprintf (stderr, "Bad count for QueueCount : %d\n",
               QueueCount);
      QueueCount = 0;
    }
  if (QueueEntryCount != 0)
    {
      fprintf (stderr, "Bad count for QueueEntryCount : %d\n",
               QueueEntryCount);
      QueueEntryCount = 0;
    }
  if (ListCount != 0)
    {
      fprintf (stderr, "Bad count for ListCount : %d\n",
               ListCount);
      ListCount = 0;
    }
  if (ListEntryCount != 0)
    {
      fprintf (stderr, "Bad count for ListEntryCount : %d\n",
               ListEntryCount);
      ListEntryCount = 0;
    }
  if (IteratorCount != 0)
    {
      fprintf (stderr, "Bad count for IteratorCount : %d\n",
               IteratorCount);
      IteratorCount = 0;
    }
  if (HasherCount != 0)
    {
      fprintf (stderr, "Bad count for HasherCount : %d\n",
               HasherCount);
      HasherCount = 0;
    }
  if (HasherTableCount != 0)
    {
      fprintf (stderr, "Bad count for HasherTableCount : %d\n",
               HasherTableCount);
      HasherTableCount = 0;
    }
}

/*----------------------------------------------------------*/
CArray CArrayNew ()
/*----------------------------------------------------------*/
{
  CArray This;
  
  if (!Initialized)
    {
      CArrayStartup ();
    }

  This = (CArray) CMalloc (sizeof (CArrayRec));
  ArrayCount++;

  This->array       = NULL;
  This->allocated   = 0;
  This->used        = 0;
  This->free        = 0;

  GCObjectNew (This, ArrayClass);
  
  return (This);
}

/*----------------------------------------------------------*/
static void CArrayDelete (CArray This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  CArrayClear (This);
  
  CFree (This);
  ArrayCount--;
}

/*----------------------------------------------------------*/
static void CArrayClear (CArray This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  CArrayErase (This);

  if (This->array != NULL)
    {
      CFree (This->array);
      ArrayDataCount--;
      
      This->array     = NULL;
      This->allocated = 0;
    }
}

/*----------------------------------------------------------*/
void CArrayErase (CArray This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  if (This->array != NULL)
    {
      int pos;

      for (pos = 0; pos < This->used; pos++)
        {
          if (This->array[pos] != NULL)
            {
              GCReferenceSet ((void**) &This->array[pos], This, NULL);
            }
        }
      
      This->used      = 0;
      This->free      = 0;
    }
}

/*----------------------------------------------------------*/
static int CArrayGetUsed (CArray This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (0);

  return (This->used);
}

/*----------------------------------------------------------*/
int CArrayCount (CArray This)
/*----------------------------------------------------------*/
{
  int pos;
  int count;

  if (This == NULL) return (0);
  if (This->array == NULL) return (0);

  count = 0;
  for (pos = 0; pos < This->used; pos++)
    {
      if (This->array[pos] != NULL) count++;
    }

  return (count);
}

/*----------------------------------------------------------*/
void CArrayAdd (CArray This, void* object)
/*----------------------------------------------------------*/
{
  int pos;

  if (This == NULL) return;
  if (object == NULL) return;

  if (This->free > 0)
    {
        /*
          'free' counts the number of empty places between '0' and 'used'
          corresponding to objects removed from this array.
          Here we reuse the first available free place.
          */
  
      for (pos = 0; pos < This->used; pos++)
	{
	  if (This->array[pos] == NULL)
	    {
	      This->free--;
	      break;
	    }
	}
    }
  else
    {
        /*
          No 'free' place thus we must extend the 'used' area.
          Extension will be done either from the 'allocated' space
          (when ther is some left) or by first extending the
          'allocated' space.
          */
      if (This->used >= This->allocated)
	{
	  This->allocated += QUANTUM;
	  if (This->array == NULL)
	    {
	      This->array = (void**)
                  CMalloc (This->allocated * sizeof (void*));
              ArrayDataCount++;
	    }
	  else
	    {
              void** old = This->array;
              
              ArrayLock ();
              
	      This->array = (void**)
                  realloc (This->array,
                           This->allocated * sizeof (void*));
              
              if (This->array != old)
                {
                  void** array;
                  
                  array = This->array;
                  
                  for (pos = 0; pos < This->used; pos++)
                    {
                      GCReferenceMove (old, array);
                      old++;
                      array++;
                    }
                }
              
              ArrayUnlock ();
              
	    }
          memset (&This->array[This->used], 0, QUANTUM * sizeof (void*));
        }
        /*
          The first available place is at the end of the 'used' place.
          */
      pos = This->used;
      This->used++;
    }

  GCReferenceSet ((void**) &This->array[pos], This, object);
}

/*----------------------------------------------------------*/
void CArrayRemove (CArray This, void* object)
/*----------------------------------------------------------*/
{
  int pos;

  if (This == NULL) return;
  if (This->array == NULL) return;
  if (object == NULL) return;

  for (pos = 0; pos < This->used; pos++)
    {
      if (This->array[pos] == object)
	{
          GCReferenceSet ((void**) &This->array[pos], This, NULL);
          
	  This->free++;
          if (This->free > This->used)
            {
              fprintf (stderr, "Too much remove...\n");
            }
	  break;
	}
    }
}

/*----------------------------------------------------------*/
void* CArrayItem (CArray This, CIterator it)
/*----------------------------------------------------------*/
{
  int index;

  if (This == NULL) return (NULL);
  if (This->array == NULL) return (NULL);
  if (it == NULL) return (NULL);
  
  if (CIteratorFinished(it)) return (NULL);

  index = CIteratorIndex (it);

  if (index < 0) return (NULL);
  if (index >= This->used) return (NULL);

  return (This->array[index]);
}

/*----------------------------------------------------------*/
void CArrayRemoveAt (CArray This, int index)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;
  if (This->array == NULL) return;
  if ((index < 0) || (index > This->used)) return;

  if (This->array[index] == NULL) return;
  
  GCReferenceSet ((void**) &This->array[index], This, NULL);
  
  This->free++;
  if (This->free > This->used)
    {
      fprintf (stderr, "Too much remove...\n");
    }
}

/*----------------------------------------------------------*/
static void* CArrayFetchItemAt (CArray This, int index)
/*----------------------------------------------------------*/
{
  void* item = NULL;
  
  item = This->array[index];

  return (item);
}

/*----------------------------------------------------------*/
void* CArrayItemAt (CArray This, int index)
/*----------------------------------------------------------*/
{
  void* item = NULL;
  
  if (This == NULL) return (NULL);
  if (This->array == NULL) return (NULL);
  if ((index < 0) || (index > This->used)) return (NULL);

  ArrayLock ();
  item = This->array[index];
  ArrayUnlock ();

  return (item);
}

/*----------------------------------------------------------*/
void* CArrayFind (CArray This, void* object)
/*----------------------------------------------------------*/
{
  CIterator it;
  void* result = NULL;
  
  if (This == NULL) return (NULL);
  if (object == NULL) return (NULL);
  
  it = CForwardIteratorNew (This);
  for (; !CIteratorFinished (it); CIteratorForth (it))
    {
      void* item;
      
      item = CIteratorItem (it);
      if (item == object)
        {
          result = item;
          break;
        }
    }
  CIteratorDelete (it);

  return (result);
}

/*----------------------------------------------------------*/
CIterator CArrayGiveIterator (CArray This)
/*----------------------------------------------------------*/
{
  CIterator result = NULL;
  
  if (This == NULL) return (NULL);

  result = CForwardIteratorNew (This);

  return (result);
}

/*----------------------------------------------------------*/
CIterator CArrayGiveBackwardIterator (CArray This)
/*----------------------------------------------------------*/
{
  CIterator result = NULL;
  
  if (This == NULL) return (NULL);

  result = CBackwardIteratorNew (This);

  return (result);
}

/*------------------------------------------------------------*/
/*          Stack                                             */
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
typedef struct _CStackEntryRec* CStackEntry;
typedef struct _CStackEntryRec
{
  CStackEntry next;
  
  void* object;
} CStackEntryRec;

typedef struct _CStackRec
{
  int count;
  CStackEntry first;
} CStackRec;

#define QUANTUM 20
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
static void CStackDelete (CStack This);
static void CStackClear (CStack This);
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
static void CStackStartup ()
/*------------------------------------------------------------*/
{
  StackClass = GCClassNew ("Stack", (CMethod) CStackDelete);
}

/*------------------------------------------------------------*/
static void CStackCleanup ()
/*------------------------------------------------------------*/
{
  GCClassDelete (StackClass);
  StackClass = NULL;
}

/*----------------------------------------------------------*/
CStack CStackNew ()
/*----------------------------------------------------------*/
{
  CStack This;
  
  if (!Initialized)
    {
      CArrayStartup ();
    }

  This = (CStack) CMalloc (sizeof (CStackRec));
  StackCount++;

  This->count  = 0;
  This->first  = NULL;
  
  GCObjectNew (This, StackClass);
  
  return (This);
}

/*----------------------------------------------------------*/
static void CStackDelete (CStack This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  CStackClear (This);
  
  CFree (This);
  StackCount--;
}

/*----------------------------------------------------------*/
static void CStackClear (CStack This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  CStackErase (This);
}

/*----------------------------------------------------------*/
void CStackErase (CStack This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  while (This->first) CStackPop (This);
}

/*----------------------------------------------------------*/
int CStackCount (CStack This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (0);

  return (This->count);
}

/*----------------------------------------------------------*/
void CStackPush (CStack This, void* object)
/*----------------------------------------------------------*/
{
  CStackEntry entry = NULL;
  
  if (This == NULL) return;
  if (object == NULL) return;

    /* Creating a new StackEntry */
  
  entry = (CStackEntry) CMalloc (sizeof(CStackEntryRec));
  entry->object = NULL;
  entry->next = NULL;

  GCReferenceSet ((void**) &entry->object, This, object);

  entry->next = This->first;
  This->first = entry;
  
  This->count++;
}

/*----------------------------------------------------------*/
void* CStackTop (CStack This)
/*----------------------------------------------------------*/
/*  This function reads the 'top' entry in the stack.       */
/*----------------------------------------------------------*/
{
  void* result;
  CStackEntry top;
  
  if (This == NULL) return (NULL);

  top = This->first;
  if (!top) return (NULL);

  result = top->object;

  return (result);
}

/*----------------------------------------------------------*/
void* CStackPop (CStack This)
/*----------------------------------------------------------*/
/*  This function picks up the 'top' entry in the stack.    */
/*----------------------------------------------------------*/
{
  void* result;
  CStackEntry top;
  
  if (This == NULL) return (NULL);

  top = This->first;
  if (!top) return (NULL);

  result = top->object;
  This->first = top->next;

    /* Clear and delete the old top entry */
  
  GCReferenceSet ((void**) &top->object, This, NULL);

  top->next = NULL;
  CFree (top);

  This->count--;

  return (result);
}

/*------------------------------------------------------------*/
/*          Queue                                             */
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
typedef struct _CQueueEntryRec* CQueueEntry;
typedef struct _CQueueEntryRec
{
  CQueueEntry previous;
  
  void* object;
} CQueueEntryRec;

typedef struct _CQueueRec
{
  int count;
  CQueueEntry first;
  CQueueEntry last;
} CQueueRec;

/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
static void CQueueDelete (CQueue This);
static void CQueueClear (CQueue This);
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
static void CQueueStartup ()
/*------------------------------------------------------------*/
{
  QueueClass = GCClassNew ("Queue", (CMethod) CQueueDelete);
}

/*------------------------------------------------------------*/
static void CQueueCleanup ()
/*------------------------------------------------------------*/
{
  GCClassDelete (QueueClass);
  QueueClass = NULL;
}

/*----------------------------------------------------------*/
CQueue CQueueNew ()
/*----------------------------------------------------------*/
{
  CQueue This;
  
  if (!Initialized)
    {
      CArrayStartup ();
    }

  This = (CQueue) CMalloc (sizeof (CQueueRec));
  QueueCount++;

  This->count  = 0;
  This->first  = NULL;
  This->last   = NULL;
  
  GCObjectNew (This, QueueClass);
  
  return (This);
}

/*----------------------------------------------------------*/
static void CQueueDelete (CQueue This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  CQueueClear (This);
  
  CFree (This);
  QueueCount--;
}

/*----------------------------------------------------------*/
static void CQueueClear (CQueue This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  CQueueErase (This);
}

/*----------------------------------------------------------*/
void CQueueErase (CQueue This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

/*
  printf ("CQueueTop[%lx]> count %d first %lx last %lx\n", 
          This,
          This->count,
          This->first,
          This->last);
          */

  while (This->first) CQueuePop (This);

  This->count  = 0;
  This->first  = NULL;
  This->last   = NULL;
}

/*----------------------------------------------------------*/
int CQueueCount (CQueue This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (0);

  return (This->count);
}

/*----------------------------------------------------------*/
void CQueuePush (CQueue This, void* object)
/*----------------------------------------------------------*/
{
  CQueueEntry entry = NULL;
  
  if (This == NULL) return;
  if (object == NULL) return;

/*
  printf ("CQueuePush[%lx]> object %lx\n", This, object);
  printf ("CQueue> count %d first %lx last %lx\n", 
          This->count,
          This->first,
          This->last);
          */

    /* Creating a new QueueEntry */
  
  entry = (CQueueEntry) CMalloc (sizeof(CQueueEntryRec));
  entry->object = NULL;
  entry->previous = NULL;
  
  GCReferenceSet ((void**) &entry->object, This, object);

  if (This->first) This->first->previous = entry;
  else This->last = entry;

  This->first = entry;

  This->count++;
}

/*----------------------------------------------------------*/
void* CQueueTop (CQueue This)
/*----------------------------------------------------------*/
/*  This function reads the 'top' entry in the queue.       */
/*----------------------------------------------------------*/
{
  void* result;
  CQueueEntry top;
  
  if (This == NULL) return (NULL);

/*
  printf ("CQueueTop[%lx]> count %d first %lx last %lx\n", 
          This,
          This->count,
          This->first,
          This->last);
          */

  top = This->last;
  if (!top) return (NULL);

  result = top->object;

  return (result);
}

/*----------------------------------------------------------*/
void* CQueuePop (CQueue This)
/*----------------------------------------------------------*/
/*  This function picks up the 'top' entry in the queue.    */
/*----------------------------------------------------------*/
{
  void* result;
  CQueueEntry top;
  
  if (This == NULL) return (NULL);

/*
  printf ("CQueuePop[%lx]> count %d first %lx last %lx\n", 
          This,
          This->count,
          This->first,
          This->last);
          */

  top = This->last;
  if (!top) return (NULL);

  result = top->object;

  This->last = top->previous;
  if (!This->last) This->first = NULL;

    /* Clear and delete the old top entry */
  
  GCReferenceSet ((void**) &top->object, This, NULL);

  top->previous = NULL;
  CFree (top);

  This->count--;

  return (result);
}

/*------------------------------------------------------------*/
/*          List                                              */
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
typedef struct _CLListEntryRec* CLListEntry;
typedef struct _CLListEntryRec
{
  CLListEntry next;
  CLListEntry previous;
  
  void* object;
} CLListEntryRec;

typedef struct _CLListRec
{
  int count;
  CLListEntry first;
  CLListEntry last;
} CLListRec;

/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
static void CLListDelete (CLList This);
static void CLListClear (CLList This);
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
static void CLListStartup ()
/*------------------------------------------------------------*/
{
  ListClass = GCClassNew ("List", (CMethod) CLListDelete);
}

/*------------------------------------------------------------*/
static void CLListCleanup ()
/*------------------------------------------------------------*/
{
  GCClassDelete (ListClass);
  ListClass = NULL;
}

/*----------------------------------------------------------*/
CLList CLListNew ()
/*----------------------------------------------------------*/
{
  CLList This;
  
  if (!Initialized)
    {
      CArrayStartup ();
    }

  This = (CLList) CMalloc (sizeof (CLListRec));
  ListCount++;

  This->count  = 0;
  This->first  = NULL;
  This->last   = NULL;
  
  GCObjectNew (This, ListClass);
  
  return (This);
}

/*----------------------------------------------------------*/
static void CLListDelete (CLList This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  CLListClear (This);
  
  CFree (This);
  ListCount--;
}

/*----------------------------------------------------------*/
static void CLListClear (CLList This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  CLListErase (This);
}

/*----------------------------------------------------------*/
void CLListErase (CLList This)
/*----------------------------------------------------------*/
{
  CLListEntry entry;
  CLListEntry previous;
  CLListEntry next;

  if (This == NULL) return;

  while ((entry = This->first))
    {
      previous = entry->previous;
      next     = entry->next;

      This->first = next;

      if (next) next->previous = previous;
      else This->last = previous;

        /* Clear and delete the entry */
  
      GCReferenceSet ((void**) &entry->object, This, NULL);

      entry->next     = NULL;
      entry->previous = NULL;
      CFree (entry);

      This->count--;
    }
}

/*----------------------------------------------------------*/
int CLListCount (CLList This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (0);

  return (This->count);
}

/*----------------------------------------------------------*/
void CLListAdd (CLList This, void* object)
/*----------------------------------------------------------*/
{
  CLListEntry entry = NULL;
  
  if (This == NULL) return;
  if (object == NULL) return;

    /* Creating a new ListEntry */
  
  entry = (CLListEntry) CMalloc (sizeof(CLListEntryRec));
  entry->object   = NULL;
  entry->next     = NULL;
  entry->previous = NULL;
  
  GCReferenceSet ((void**) &entry->object, This, object);

  entry->previous = This->last;

  if (This->last) This->last->next = entry;
  else This->first = entry;

  This->last = entry;

  This->count++;
}

/*----------------------------------------------------------*/
void CLListPrepend (CLList This, void* object)
/*----------------------------------------------------------*/
{
  CLListEntry entry = NULL;
  
  if (This == NULL) return;
  if (object == NULL) return;

    /* Creating a new ListEntry */
  
  entry = (CLListEntry) CMalloc (sizeof(CLListEntryRec));
  entry->object   = NULL;
  entry->next     = NULL;
  entry->previous = NULL;
  
  GCReferenceSet ((void**) &entry->object, This, object);

  entry->next = This->first;

  if (This->first) This->first->previous = entry;
  else This->last = entry;

  This->first = entry;

  This->count++;
}

/*----------------------------------------------------------*/
void* CLListFirst (CLList This)
/*----------------------------------------------------------*/
/*  This function reads the 'first' entry in the list.      */
/*----------------------------------------------------------*/
{
  void* result;
  CLListEntry first;
  
  if (This == NULL) return (NULL);

  first = This->first;
  if (!first) return (NULL);

  result = first->object;

  return (result);
}

/*----------------------------------------------------------*/
void* CLListLast (CLList This)
/*----------------------------------------------------------*/
/*  This function reads the 'last' entry in the list.       */
/*----------------------------------------------------------*/
{
  void* result;
  CLListEntry last;
  
  if (This == NULL) return (NULL);

  last = This->last;
  if (!last) return (NULL);

  result = last->object;

  return (result);
}

/*----------------------------------------------------------*/
void CLListRemove (CLList This, void* object)
/*----------------------------------------------------------*/
{
  CLListEntry entry;
  CLListEntry previous;
  CLListEntry next;
  
  if (This == NULL) return;

  entry = This->first;
  while (entry != NULL)
    {
      if (entry->object == object) break;
      entry = entry->next;
    }
  if (entry == NULL) return;

  previous = entry->previous;
  next     = entry->next;

  if (previous) previous->next = next;
  else This->first = next;

  if (next) next->previous = previous;
  else This->last = previous;

    /* Clear and delete the entry */
  
  GCReferenceSet ((void**) &entry->object, This, NULL);

  entry->next     = NULL;
  entry->previous = NULL;
  CFree (entry);

  This->count--;
}

/*----------------------------------------------------------*/
void* CLListFind (CLList This, void* object)
/*----------------------------------------------------------*/
{
  CLListEntry entry;
  
  if (This == NULL) return (NULL);

  entry = This->first;
  while (entry != NULL)
    {
      if (entry->object == object) break;
      entry = entry->next;
    }
  if (entry == NULL) return (NULL);

  return (entry->object);
}

/*----------------------------------------------------------*/
CIterator CLListGiveIterator (CLList This)
/*----------------------------------------------------------*/
{
  CIterator result = NULL;
  
  if (This == NULL) return (NULL);

  result = CForwardLListIteratorNew (This);

  return (result);
}

/*----------------------------------------------------------*/
CIterator CLListGiveBackwardIterator (CLList This)
/*----------------------------------------------------------*/
{
  CIterator result = NULL;
  
  if (This == NULL) return (NULL);

  result = CBackwardLListIteratorNew (This);

  return (result);
}

/*---------------------------------------------------*/
/*                                                   */
/*    Hasher                                         */
/*                                                   */
/*---------------------------------------------------*/

typedef struct _CHashPositionRec* CHashPosition;

typedef struct _CHasherRec
{
  CArray*   table;
  int       size;
  CValuator valuator;
  CCoder    coder;
  CTester   tester;
} CHasherRec;

typedef struct _CHashPositionRec
{
  int code;
  int index;
} CHashPositionRec;

static void CHasherDelete (CHasher This);
static int CHasherHas (CHasher This, void* value,
                       CHashPosition position);
static void* CHasherItemAt (CHasher This, CHashPosition position);

/*------------------------------------------------------------*/
static void CHasherStartup ()
/*------------------------------------------------------------*/
{
  HasherClass = GCClassNew ("Hasher", (CMethod) CHasherDelete);
}

/*------------------------------------------------------------*/
static void CHasherCleanup ()
/*------------------------------------------------------------*/
{
  GCClassDelete (HasherClass);
  HasherClass = NULL;
}

/*---------------------------------------------------*/
static void* DefaultValuator (void* object)
/*---------------------------------------------------*/
{
  return (object);
}

/*---------------------------------------------------*/
CHasher CPtrHasherNew (int size, CValuator valuator)
/*---------------------------------------------------*/
{
  if (valuator == NULL) valuator = DefaultValuator;

  return (CHasherNew (size,
                      valuator,
                      (CCoder) CPtrCoder,
                      (CTester) CPtrTester));
}

/*---------------------------------------------------*/
CHasher CStrHasherNew (int size, CValuator valuator)
/*---------------------------------------------------*/
{
  if (valuator == NULL) valuator = DefaultValuator;

  return (CHasherNew (size,
                      valuator,
                      (CCoder) CStrCoder,
                      (CTester) CStrTester));
}

/*---------------------------------------------------*/
CHasher CLStrHasherNew (int size, CValuator valuator)
/*---------------------------------------------------*/
{
  if (valuator == NULL) valuator = DefaultValuator;

  return (CHasherNew (size,
                      valuator,
                      (CCoder) CLStrCoder,
                      (CTester) CLStrTester));
}

/*---------------------------------------------------*/
CHasher CHasherNew (int size,
                    CValuator valuator,
                    CCoder coder,
                    CTester tester)
/*---------------------------------------------------*/
{
  CHasher This;

  if (!Initialized)
    {
      CArrayStartup ();
    }

  if (size <= 0) return (NULL);
  
  if (valuator == NULL) return (NULL);
  if (coder == NULL) return (NULL);
  if (tester == NULL) return (NULL);

  This = (CHasher) CMalloc (sizeof (CHasherRec));
  HasherCount++;

  This->size     = size;
  This->valuator = valuator;
  This->coder    = coder;
  This->tester   = tester;

  This->table = (CArray*) CMalloc (size * sizeof(CArray));
  HasherTableCount++;
  
  memset (This->table, 0, size * sizeof(CArray));

  GCObjectNew (This, HasherClass);
  
  return (This);
}

/*---------------------------------------------------*/
static void CHasherDelete (CHasher This)
/*---------------------------------------------------*/
{
  int index;
  
  if (This == NULL) return;

  if (This->size > 0)
    {
      for (index = 0; index < This->size; index++)
        {
          CArray array;

          array = This->table[index];
          if (array != NULL)
            {
              CArrayDelete (array);
              GCReferenceSet ((void**) &This->table[index], This, NULL);
            }
        }
      CFree (This->table);
      HasherTableCount--;
      
      This->table = NULL;
      This->size = 0;
    }
  This->valuator = NULL;
  This->coder    = NULL;
  This->tester   = NULL;

  CFree (This);
  HasherCount--;
}

/*---------------------------------------------------*/
void CHasherErase (CHasher This)
/*---------------------------------------------------*/
{
  int index;
  
  if (This == NULL) return;

  if (This->size > 0)
    {
      for (index = 0; index < This->size; index++)
        {
          CArray array;

          array = This->table[index];
          CArrayErase (array);
        }
    }
}

/*---------------------------------------------------*/
void CHasherAdd (CHasher This, void* object)
/*---------------------------------------------------*/
{
  int code;
  void* value;

  value = This->valuator(object);
  if (value == NULL) return;

  code = This->coder (value);
  code %= This->size;

  if (This->table[code] == NULL)
    {
      GCReferenceSet ((void**) &This->table[code], This, CArrayNew ());
    }

  CArrayAdd (This->table[code], object);
}

/*---------------------------------------------------*/
void* CHasherFind (CHasher This, void* value)
/*---------------------------------------------------*/
{
  CHashPositionRec position;
  int status;
  
  status = CHasherHas (This, value, &position);
  if (!status) return (NULL);
  return (CHasherItemAt (This, &position));
}

/*---------------------------------------------------*/
int CHasherFindPosition (CHasher This, void* value,
                         int* code, int* index)
/*---------------------------------------------------*/
{
  CHashPositionRec position;
  int status;
  
  status = CHasherHas (This, value, &position);
  if (!status) return (0);
  if (code != NULL) *code = position.code;
  if (index != NULL) *index = position.index;
  return (1);
}

/*---------------------------------------------------*/
static int CHasherHas (CHasher This, void* value,
                       CHashPosition position)
/*---------------------------------------------------*/
{
  int result = 0;
  int code;
  int pos;
  CArray array;
  int max_pos;
  
  if (value == NULL) return (0);

  code = This->coder (value);
  code %= This->size;

  array = This->table[code];
  if (array == NULL) return (0);

  position->code = code;
  
  max_pos = CArrayGetUsed (array);
  for (pos = 0; pos < max_pos; pos++)
    {
      void* item;
      void* theValue;

      item = CArrayFetchItemAt (array, pos);
      
      theValue = This->valuator (item);
      if (This->tester (theValue, value))
        {
          position->index = pos;
  
          result = 1;
          break;
        }
    }

  return (result);
}

/*---------------------------------------------------*/
static void* CHasherItemAt (CHasher This, CHashPosition position)
/*---------------------------------------------------*/
{
  void* item;

    /*
      if (This == NULL) return (NULL);
      if (position == NULL) return (NULL);
      */
  
  item = CArrayItemAt (This->table[position->code], position->index);
  return (item);
}

/*---------------------------------------------------*/
void CHasherRemove (CHasher This, void* object)
/*---------------------------------------------------*/
{
  CHashPositionRec position;
  void* value;
  int status;

  if (This == NULL) return;
  if (object == NULL) return;

  value = This->valuator (object);
  if (value == NULL) return;

  status = CHasherHas (This, value, &position);
  if (status)
    {
      CArrayRemoveAt (This->table[position.code], position.index);
    }
}

/*----------------------------------------------------------*/
CIterator CHasherGiveIterator (CHasher This)
/*----------------------------------------------------------*/
{
  CIterator result = NULL;
  
  if (This == NULL) return (NULL);

  result = CHasherIteratorNew (This);

  return (result);
}

/*----------------------------------------------------------*/
/*                                                          */
/*    Iterator                                              */
/*                                                          */
/*----------------------------------------------------------*/

typedef void   (*CIteratorMethod) (CIterator This, ...);
typedef int    (*CIteratorIndexMethod) (CIterator This, ...);
typedef void*  (*CIteratorItemMethod) (CIterator This, ...);

typedef struct _CIteratorClassRec* CIteratorClass;

typedef struct _CIteratorClassRec
{
  char*           name;
  CIteratorMethod first;
  CIteratorMethod forth;
  CIteratorMethod clear;
  CIteratorIndexMethod mindex;
  CIteratorItemMethod item;
} CIteratorClassRec;

typedef struct _CIteratorRec
{
  CIteratorClass Class;

  int finished;
} CIteratorRec;

static void CIteratorClear (CIterator This);

/*----------------------------------------------------------*/
int CIteratorFinished (CIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (1);

  return (This->finished);
}

/*----------------------------------------------------------*/
void CIteratorFirst (CIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  if (This->Class->first != NULL) This->Class->first (This);
}

/*----------------------------------------------------------*/
void CIteratorForth (CIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;
  if (This->finished) return;

  if (This->Class->forth != NULL) This->Class->forth (This);
}

/*----------------------------------------------------------*/
void CIteratorDelete (CIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  CIteratorClear (This);
  CFree (This);
  IteratorCount--;
}

/*----------------------------------------------------------*/
static void CIteratorClear (CIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  if (This->Class->clear != NULL) This->Class->clear (This);
}

/*----------------------------------------------------------*/
static int CIteratorIndex (CIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (-1);

  if (This->Class->mindex != NULL) return (This->Class->mindex (This));
  return (-1);
}

/*----------------------------------------------------------*/
void* CIteratorItem (CIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (NULL);

  if (This->Class->item != NULL) return (This->Class->item (This));
  return (NULL);
}

/*----------------------------------------------------------*/
void CIteratorStop (CIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;
  
  This->finished = 1;
}

/*---------------------------------------------------*/
/*                                                   */
/*    ArrayIterator                                  */
/*                                                   */
/*---------------------------------------------------*/

typedef struct _CArrayIteratorRec* CArrayIterator;
typedef struct _CArrayIteratorRec
{
  CIteratorRec base;

  CArray array;
  int current;
} CArrayIteratorRec;

static void CArrayIteratorFirst (CArrayIterator This);
static void CArrayIteratorClear (CArrayIterator This);
static int CArrayIteratorIndex (CArrayIterator This);
static void* CArrayIteratorItem (CArrayIterator This);

static void CForwardIteratorForth (CArrayIterator This);

static CIteratorClassRec CForwardIteratorClass = 
{
  "forward",
  (CIteratorMethod) CArrayIteratorFirst,
  (CIteratorMethod) CForwardIteratorForth,
  (CIteratorMethod) CArrayIteratorClear,
  (CIteratorIndexMethod) CArrayIteratorIndex,
  (CIteratorItemMethod) CArrayIteratorItem
};

static void CBackwardIteratorForth (CArrayIterator This);

static CIteratorClassRec CBackwardIteratorClass = 
{
  "backward",
  (CIteratorMethod) CArrayIteratorFirst,
  (CIteratorMethod) CBackwardIteratorForth,
  (CIteratorMethod) CArrayIteratorClear,
  (CIteratorIndexMethod) CArrayIteratorIndex,
  (CIteratorItemMethod) CArrayIteratorItem
};

/*----------------------------------------------------------*/
static CIterator CForwardIteratorNew (CArray array)
/*----------------------------------------------------------*/
{
  CArrayIterator This;

  if (!Initialized)
    {
      GCStartup ();
    }

  This = (CArrayIterator) CMalloc (sizeof (CArrayIteratorRec));
  IteratorCount++;

  This->base.Class = &CForwardIteratorClass;
  This->array = array;
  CIteratorFirst ((CIterator) This);

  return ((CIterator) This);
}

/*----------------------------------------------------------*/
static CIterator CBackwardIteratorNew (CArray array)
/*----------------------------------------------------------*/
{
  CArrayIterator This;

  if (!Initialized)
    {
      GCStartup ();
    }

  This = (CArrayIterator) CMalloc (sizeof (CArrayIteratorRec));
  IteratorCount++;

  This->base.Class = &CBackwardIteratorClass;
  This->array = array;
  CIteratorFirst ((CIterator) This);

  return ((CIterator) This);
}

/*----------------------------------------------------------*/
static void CArrayIteratorClear (CArrayIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  This->base.finished = 1;
  This->array = NULL;
  This->current = -1;
}

/*----------------------------------------------------------*/
static void CArrayIteratorFirst (CArrayIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  This->base.finished = 0;
  This->current  = -1;

  CIteratorForth ((CIterator) This);
}

/*----------------------------------------------------------*/
static void CForwardIteratorForth (CArrayIterator This)
/*----------------------------------------------------------*/
{
  int pos;
  
  This->base.finished = 1;

  if (This->array == NULL) return;
  if (This->array->array == NULL) return;

  This->current++;

  for (pos = This->current; pos < This->array->used; pos++)
    {
      if (This->array->array[pos] != NULL)
        {
          This->base.finished = 0;
          This->current  = pos;
          break;
        }
    }
}

/*----------------------------------------------------------*/
static void CBackwardIteratorForth (CArrayIterator This)
/*----------------------------------------------------------*/
{
  int pos;
  
  This->base.finished = 1;

  if (This->array == NULL) return;
  if (This->array->array == NULL) return;
  
  if (This->current < 0) This->current = This->array->used;
  This->current--;

  for (pos = This->current; pos >= 0; pos--)
    {
      if (This->array->array[pos] != NULL)
        {
          This->base.finished = 0;
          This->current  = pos;
          break;
        }
    }
}

/*----------------------------------------------------------*/
static int CArrayIteratorIndex (CArrayIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (-1);

  return (This->current);
}

/*----------------------------------------------------------*/
static void* CArrayIteratorItem (CArrayIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (NULL);
  if (This->base.finished) return (NULL);

  return (CArrayItemAt (This->array, This->current));
}

/*---------------------------------------------------*/
/*                                                   */
/*    HasherIterator                                 */
/*                                                   */
/*---------------------------------------------------*/

typedef struct _CHasherIteratorRec* CHasherIterator;
typedef struct _CHasherIteratorRec
{
  CIteratorRec base;

  CHasher hasher;
  int current;
  CIterator iterator;
} CHasherIteratorRec;

static void CHasherIteratorFirst (CHasherIterator This);
static void CHasherIteratorForth (CHasherIterator This);
static void CHasherIteratorClear (CHasherIterator This);
static int CHasherIteratorIndex (CHasherIterator This);
static void* CHasherIteratorItem (CHasherIterator This);

static CIteratorClassRec CHasherIteratorClass = 
{
  "hasher",
  (CIteratorMethod) CHasherIteratorFirst,
  (CIteratorMethod) CHasherIteratorForth,
  (CIteratorMethod) CHasherIteratorClear,
  (CIteratorIndexMethod) CHasherIteratorIndex,
  (CIteratorItemMethod) CHasherIteratorItem
};

/*----------------------------------------------------------*/
static CIterator CHasherIteratorNew (CHasher hasher)
/*----------------------------------------------------------*/
{
  CHasherIterator This;

  if (!Initialized)
    {
      GCStartup ();
    }

  This = (CHasherIterator) CMalloc (sizeof (CHasherIteratorRec));
  IteratorCount++;

  This->base.Class = &CHasherIteratorClass;
  This->hasher   = hasher;
  This->current  = -1;
  This->iterator = NULL;
  CIteratorFirst ((CIterator) This);

  return ((CIterator) This);
}

/*----------------------------------------------------------*/
static void CHasherIteratorClear (CHasherIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  This->base.finished = 1;
  This->hasher   = NULL;
  This->current  = -1;
  if (This->iterator != NULL)
    {
      CIteratorDelete (This->iterator);
      This->iterator = NULL;
    }
}

/*----------------------------------------------------------*/
static void CHasherIteratorFirst (CHasherIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  This->base.finished = 0;
  This->current       = -1;

  if (This->iterator != NULL)
    {
      CIteratorDelete (This->iterator);
      This->iterator = NULL;
    }
  
  CIteratorForth ((CIterator) This);
}

/*----------------------------------------------------------*/
static void CHasherIteratorForth (CHasherIterator This)
/*----------------------------------------------------------*/
{
  int pos;
  
  This->base.finished = 1;

  if (This->hasher == NULL) return;

  if (This->iterator != NULL)
    {
      CIteratorForth (This->iterator);
      if (CIteratorFinished (This->iterator))
        {
          CIteratorDelete (This->iterator);
          This->iterator = NULL;
        }
      else
        {
          This->base.finished = 0;
          return;
        }
    }
  
  This->current++;
  
  for (pos = This->current; pos < This->hasher->size; pos++)
    {
      if (This->hasher->table[pos] != NULL)
        {
          if (This->iterator == NULL)
            {
              This->iterator = CForwardIteratorNew (This->hasher->table[pos]);
            }
          else
            {
              CIteratorForth (This->iterator);
            }
          
          if (CIteratorFinished (This->iterator))
            {
              CIteratorDelete (This->iterator);
              This->iterator = NULL;
            }
          else
            {
              This->base.finished = 0;
              This->current  = pos;
              break;
            }
        }
    }
}

/*----------------------------------------------------------*/
static int CHasherIteratorIndex (CHasherIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (-1);
  if (This->base.finished) return (-1);

  return (This->current);
}

/*----------------------------------------------------------*/
static void* CHasherIteratorItem (CHasherIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (NULL);
  if (This->base.finished) return (NULL);

  return (CIteratorItem (This->iterator));
}

/*---------------------------------------------------*/
/*                                                   */
/*    LListIterator                                  */
/*                                                   */
/*---------------------------------------------------*/

typedef struct _CLListIteratorRec* CLListIterator;
typedef struct _CLListIteratorRec
{
  CIteratorRec base;

  CLList list;
  CLListEntry current;
} CLListIteratorRec;

static void CLListIteratorFirst (CLListIterator This);
static void CLListIteratorClear (CLListIterator This);
static void* CLListIteratorItem (CLListIterator This);

static void CForwardLListIteratorForth (CLListIterator This);

static CIteratorClassRec CForwardLListIteratorClass = 
{
  "forward",
  (CIteratorMethod) CLListIteratorFirst,
  (CIteratorMethod) CForwardLListIteratorForth,
  (CIteratorMethod) CLListIteratorClear,
  (CIteratorIndexMethod) NULL,
  (CIteratorItemMethod) CLListIteratorItem
};

static void CBackwardLListIteratorForth (CLListIterator This);

static CIteratorClassRec CBackwardLListIteratorClass = 
{
  "backward",
  (CIteratorMethod) CLListIteratorFirst,
  (CIteratorMethod) CBackwardLListIteratorForth,
  (CIteratorMethod) CLListIteratorClear,
  (CIteratorIndexMethod) NULL,
  (CIteratorItemMethod) CLListIteratorItem
};

/*----------------------------------------------------------*/
static CIterator CForwardLListIteratorNew (CLList list)
/*----------------------------------------------------------*/
{
  CLListIterator This;

  if (!Initialized)
    {
      GCStartup ();
    }

  This = (CLListIterator) CMalloc (sizeof (CLListIteratorRec));
  IteratorCount++;

  This->base.Class = &CForwardLListIteratorClass;
  This->list = list;
  CIteratorFirst ((CIterator) This);

  return ((CIterator) This);
}

/*----------------------------------------------------------*/
static CIterator CBackwardLListIteratorNew (CLList list)
/*----------------------------------------------------------*/
{
  CLListIterator This;

  if (!Initialized)
    {
      GCStartup ();
    }

  This = (CLListIterator) CMalloc (sizeof (CLListIteratorRec));
  IteratorCount++;

  This->base.Class = &CBackwardIteratorClass;
  This->list = list;
  CIteratorFirst ((CIterator) This);

  return ((CIterator) This);
}

/*----------------------------------------------------------*/
static void CLListIteratorClear (CLListIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  This->base.finished = 1;
  This->list = NULL;
  This->current = NULL;
}

/*----------------------------------------------------------*/
static void CLListIteratorFirst (CLListIterator This)
/*----------------------------------------------------------*/
{
  This->base.finished = 1;

  if (This == NULL) return;
  if (This->list == NULL) return;

  This->base.finished = 0;
  This->current  = NULL;

  CIteratorForth ((CIterator) This);
}

/*----------------------------------------------------------*/
static void CForwardLListIteratorForth (CLListIterator This)
/*----------------------------------------------------------*/
{
  if (This->base.finished) return;

  if (This->current == NULL)
    {
      This->current = This->list->first;
    }
  else
    {
      This->current = This->current->next;
    }

  This->base.finished = (This->current == NULL);
}

/*----------------------------------------------------------*/
static void CBackwardLListIteratorForth (CLListIterator This)
/*----------------------------------------------------------*/
{
  if (This->base.finished) return;

  if (This->current == NULL)
    {
      This->current = This->list->last;
    }
  else
    {
      This->current = This->current->previous;
    }

  This->base.finished = (This->current == NULL);
}

/*----------------------------------------------------------*/
static void* CLListIteratorItem (CLListIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (NULL);
  if (This->base.finished) return (NULL);
  if (This->current == NULL) return (NULL);

  return (This->current->object);
}

/*---------------------------------------------------*/
/*                                                   */
/*    Str                                            */
/*                                                   */
/*---------------------------------------------------*/

/*------------------------------------------------------------*/
static int CStrCompare (char* str1, char* str2)
/*------------------------------------------------------------*/
{
  char* src;
  char* tst;
  char c1;
  char c2;
  
  if (!str1) return (0);
  if (!str2) return (0);

  src = str1;
  tst = str2;

  for (;;)
    {
      c1 = *src;
      c2 = *tst;
      
      if ((c1 == 0) && (c2 == 0)) return (1);
      if ((c1 == 0) || (c2 == 0)) return (0);
      
      if (c1 != c2) return (0);
      src++;
      tst++;
    }
  return (0);
}

/*------------------------------------------------------------*/
static int CStrCoder (char* text)
/*------------------------------------------------------------*/
{
  int len;
  int code;
  int pos;
  int c;

  if (text == NULL) return (0);

  len = strlen (text);
  code = len;
  
  for (pos = 0; pos < len; pos++)
  {
    c = text[pos];
    code = (code * 613) + (unsigned) c;
  }
  code &= (1 << 30) - 1;
  
  return (code);
}

/*------------------------------------------------------------*/
static int CStrTester (char* text, char* other)
/*------------------------------------------------------------*/
{
  if (text == NULL) return (0);
  if (other == NULL) return (0);

  return (CStrCompare (text, other));
}

/*------------------------------------------------------------*/
static int CLStrCompare (char* str1, char* str2)
/*------------------------------------------------------------*/
{
  char* src;
  char* tst;
  char c1;
  char c2;
  
  if (!str1) return (0);
  if (!str2) return (0);

  src = str1;
  tst = str2;

  for (;;)
    {
      c1 = *src;
      c2 = *tst;
      
      if ((c1 == 0) && (c2 == 0)) return (1);
      if ((c1 == 0) || (c2 == 0)) return (0);
      
      c1 = tolower(c1);
      c2 = tolower(c2);
      if (c1 != c2) return (0);
      src++;
      tst++;
    }
  return (0);
}

/*------------------------------------------------------------*/
static void CStrLower (char* str)
/*------------------------------------------------------------*/
{
  int p;
  int c;
  
  if (str == NULL) return;
  
  for (p = 0; p <= strlen(str); p++)
    {
      c = str[p];
      str[p] = tolower(c);
    }
}

/*------------------------------------------------------------*/
static int CLStrCoder (char* text)
/*------------------------------------------------------------*/
{
  int len;
  int code;
  int pos;
  int c;

  if (text == NULL) return (0);

  len = strlen (text);
  code = len;
  
  for (pos = 0; pos < len; pos++)
  {
    c = text[pos];
    code = (code * 613) + (unsigned) tolower(c);
  }
  code &= (1 << 30) - 1;
  
  return (code);
}

/*------------------------------------------------------------*/
static int CLStrTester (char* text, char* other)
/*------------------------------------------------------------*/
{
  if (text == NULL) return (0);
  if (other == NULL) return (0);

  return (CLStrCompare (text, other));
}

/*------------------------------------------------------------*/
static int CPtrTester (void* ptr, void* other)
/*------------------------------------------------------------*/
{
  return ((ptr == other));
}

/*------------------------------------------------------------*/
static int CPtrCoder (void* ptr)
/*------------------------------------------------------------*/
{
  int len = sizeof (void*);
  int code;
  int pos;
  union
  {
    unsigned char b[sizeof(void*)];
    void* p;
  } buffer;
  
  code = len;
  buffer.p = ptr;
  
  for (pos = 0; pos < len; pos++)
  {
    code = (code * 613) + (unsigned) buffer.b[pos];
  }
  code &= (1 << 30) - 1;
  
  return (code);
}

