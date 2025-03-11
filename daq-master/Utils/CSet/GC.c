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

#include <GC.h>

/*----------------------------------------------------------*/

typedef struct _AHasherRec* AHasher;
typedef struct _AHashPositionRec* AHashPosition;

typedef void* (*AValuator) (const void* object);
typedef int   (*ACoder) (const void* object);
typedef int   (*ATester) (const void* object, const void* other);

typedef const void* ConstPointer;

/*----------------------------------------------------------*/
/*          Array                                           */
/*----------------------------------------------------------*/

typedef struct _AArrayRec*    AArray;
typedef struct _AIteratorRec* AIterator;

typedef struct _AArrayRec
{
  void**     array;
  int        allocated;
  int        used;
  int        free;
} AArrayRec;

#define QUANTUM 10
#define HASHERSIZE 1000

/*----------------------------------------------------------*/
/*          Hasher                                          */
/*----------------------------------------------------------*/

typedef struct _AHasherRec
{
  AArrayRec  table[HASHERSIZE];
  int        used;
  AValuator  valuator;
  ACoder     coder;
  ATester    tester;
} AHasherRec;

typedef struct _AHashPositionRec
{
  int code;
  int index;
} AHashPositionRec;

/*----------------------------------------------------------*/
/*                                                          */
/*    Iterator                                              */
/*                                                          */
/*----------------------------------------------------------*/

typedef void   (*AIteratorMethod) (AIterator This, ...);
typedef int    (*AIteratorIndexMethod) (AIterator This, ...);
typedef void*  (*AIteratorItemMethod) (AIterator This, ...);

typedef struct _AIteratorClassRec* AIteratorClass;

typedef struct _AIteratorClassRec
{
  char*           name;
  AIteratorMethod first;
  AIteratorMethod forth;
  AIteratorMethod clear;
  AIteratorIndexMethod mindex;
  AIteratorItemMethod item;
} AIteratorClassRec;

typedef struct _AIteratorRec
{
  AIteratorClass Class;

  int running;
  int finished;
} AIteratorRec;

typedef struct _AArrayIteratorRec* AArrayIterator;
typedef struct _AArrayIteratorRec
{
  AIteratorRec base;

  AArray array;
  int current;
} AArrayIteratorRec;

typedef struct _AHasherIteratorRec* AHasherIterator;
typedef struct _AHasherIteratorRec
{
  AIteratorRec base;

  AHasher hasher;
  int current;
  AArrayIteratorRec iterator;
} AHasherIteratorRec;

/*----------------------------------------------------------*/
/*          classes : Class Object Reference                */
/*----------------------------------------------------------*/

typedef struct _GCClassRec
{
  char*          name;
  GCDeleteMethod erase;
  int            objects;
} GCClassRec;

typedef struct _GCObjectRec
{
  int       alive;
  GCClass   base;
  int       id;
  void*     address;
  int       references;
  int       used;   /* set when linked to an external reference. */
} GCObjectRec;

typedef struct _GCReferenceRec
{
  int      alive;
  int      id;
  void**   address;
  GCObject from;
  GCObject to;
} GCReferenceRec;

/*----------------------------------------------------------*/
static int               Initialized        = 0;

static GCClass           ClassClass         = NULL;
static GCClass           UndeclaredClass    = NULL;

static AHasherRec        ClassHasher;
static AHasherRec        ObjectHasher;
static AHasherRec        ReferenceHasher;
static AHasher           Classes            = &ClassHasher;
static AHasher           Objects            = &ObjectHasher;
static AHasher           References         = &ReferenceHasher;

static int               ObjectId           = 0;
static int               ReferenceId        = 0;

static int               ArrayDataCount     = 0;
static int               ClassCount         = 0;
static int               ClassNameCount     = 0;
static int               ObjectCount        = 0;
static int               ReferenceCount     = 0;

static int               ArrayDataAllocated = 0;
static int               ClassAllocated     = 0;
static int               ClassNameAllocated = 0;
static int               ObjectAllocated    = 0;
static int               ReferenceAllocated = 0;

static int               MallocCount        = 0;

#ifdef CTHREADS

static pthread_mutex_t ArrayMutex;
  
#endif

/*----------------------------------------------------------*/

/*----------------------------------------------------------*/
static int CStrCompare (const char* str1, const char* str2);
static int CStrCoder (const char* ptr);
static int CStrTester (const char* ptr, const char* other);

static int CPtrCoder (const void* ptr);
static int CPtrTester (const void* ptr, const void* other);

#ifdef CTHREADS
static void ArrayLock ();
static void ArrayUnlock ();
#endif

static void AArrayCheck ();
static void AArrayClear (AArray This);
static int AArrayGetUsed (AArray This);
static void AArrayInit (AArray This);
static void AArrayErase (AArray This);
static int AArrayCount (AArray This);
static void AArrayAdd (AArray This, void* object);
static void AArrayRemove (AArray This, void* object);
static void* AArrayItem (AArray This, AIterator it);
static void AArrayRemoveAt (AArray This, int index);
static void* AArrayItemAt (AArray This, int index);
static void* AArrayFetchItemAt (AArray This, int index);
static void* AArrayFind (AArray This, void* object);

static void AForwardIteratorInit (AArrayIterator This, AArray array);
static void ABackwardIteratorInit (AArrayIterator This, AArray array);
static void AHasherIteratorInit (AHasherIterator This, AHasher hasher);
static void AIteratorClear (AIterator This);
static int AIteratorFinished (AIterator This);
static int AIteratorRunning (AIterator This);
static int AIteratorIndex (AIterator This);
static void* AIteratorItem (AIterator This);
static void AIteratorFirst (AIterator This);
static void AIteratorForth (AIterator This);
static void AIteratorStop (AIterator This);

static void CPtrHasherInit (AHasher hasher, AValuator valuator);
static void CStrHasherInit (AHasher hasher, AValuator valuator);

static void AHasherInit (AHasher hasher, 
                         AValuator valuator,
                         ACoder coder,
                         ATester tester);
static void AHasherClear (AHasher This);
static void AHasherErase (AHasher This);
static void AHasherAdd (AHasher This, void* object);
static void* AHasherFind (AHasher This, const void* value);
static int AHasherHas (AHasher This, const void* value,
                       AHashPosition position);
static void* AHasherItemAt (AHasher This, AHashPosition position);
static void AHasherRemove (AHasher This, void* object);

static GCClass GCClassItem (AIterator it);
static GCClass GCClassFind (char* name);
static GCDeleteMethod GCClassGetErase (
    GCClass This);
static void* GCClassValuator (GCClass This);
static void GCClassDump (GCClass This);

static GCObject GCObjectItem (AIterator it);
static GCObject GCObjectFind (const void* address);
static void GCObjectReference (GCObject object);
static void GCObjectUnreference (GCObject object);
static void GCObjectDelete (GCObject object);
static void GCObjectDump (GCObject object);
static void GCObjectDumpId (GCObject object);
static void* GCObjectValuator (GCObject This);

static GCReference GCReferenceNew (void* address);
static void GCReferenceDump (GCReference reference);
static GCReference GCReferenceItem (AIterator it);
static GCReference GCReferenceFind (void** address);
static void GCReferenceDelete (GCReference reference);
static void* GCReferenceValuator (GCReference reference);

#ifdef GCREFERENCEDEBUG
static void* AMalloc (size_t bytes);
static void* ARealloc (void* ptr, size_t bytes);
static void AFree (void* ptr);
#else
#define AMalloc  malloc
#define ARealloc realloc
#define AFree    free
#endif

/*----------------------------------------------------------*/

/*---------------------------------------------------*/
/*                                                   */
/*    malloc/free                                    */
/*                                                   */
/*---------------------------------------------------*/

/*------------------------------------------------------------*/
static void* GCMalloc (size_t bytes)
/*------------------------------------------------------------*/
{
  void* result = NULL;

  MallocCount++;
  result = malloc (bytes);
  return (result);
}

/*------------------------------------------------------------*/
static void GCFree (void* ptr)
/*------------------------------------------------------------*/
{
  MallocCount--;
  free (ptr);
}

/*------------------------------------------------------------*/
void GCStartup ()
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

        /*
          Then we create the basic management arrays.
          They do not manage their items as  objects.
          */
      
      CPtrHasherInit (&ReferenceHasher, (AValuator) GCReferenceValuator);
      CPtrHasherInit (&ObjectHasher, (AValuator) GCObjectValuator);
      CStrHasherInit (&ClassHasher, (AValuator) GCClassValuator);
      
      GCClassNew (
          "Class",
          (GCDeleteMethod) GCClassDelete);

      UndeclaredClass = GCClassNew (
          "GCUndeclared",
          (GCDeleteMethod) free);
    }
}

/*------------------------------------------------------------*/
void GCReferenceCleanup ()
/*------------------------------------------------------------*/
{
  if (Initialized)
    {
      GCCleanup ();

      GCClassDelete (UndeclaredClass);
      UndeclaredClass = NULL;

      AHasherClear (Classes);

      GCClassDelete (ClassClass);
      ClassClass = NULL;

      AHasherClear (Objects);      
      AHasherClear (References);
      
#ifdef CTHREADS

      pthread_mutex_destroy (&ArrayMutex);
  
#endif

      Initialized = 0;
    }
}

/*----------------------------------------------------*/
static int CStrCompare (const char* str1, const char* str2)
/*----------------------------------------------------*/
{
  const char* src;
  const char* tst;
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
static int CStrCoder (const char* text)
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
static int CStrTester (const char* text, const char* other)
/*------------------------------------------------------------*/
{
  if (text == NULL) return (0);
  if (other == NULL) return (0);

  return (CStrCompare (text, other));
}

/*------------------------------------------------------------*/
static int CPtrTester (const void* ptr, const void* other)
/*------------------------------------------------------------*/
{
  return ((ptr == other));
}

/*------------------------------------------------------------*/
static int CPtrCoder (const void* ptr)
/*------------------------------------------------------------*/
{
  int len = sizeof (void*);
  int code;
  int pos;
  union
  {
    unsigned char b[sizeof(void*)];
    const void* p;
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

#ifdef CTHREADS
/*------------------------------------------------------------*/
static void ArrayLock ()
/*------------------------------------------------------------*/
{
  pthread_mutex_lock (&ArrayMutex);
}
  
/*------------------------------------------------------------*/
static void ArrayUnlock ()
/*------------------------------------------------------------*/
{
  pthread_mutex_unlock (&ArrayMutex);
}
#else
#define ArrayLock()
#define ArrayUnlock()
#endif
  

/*------------------------------------------------------------*/
static void AArrayCheck ()
/*------------------------------------------------------------*/
{
  if (ArrayDataCount != 0)
    {
      fprintf (stderr, "Bad count for ArrayDataCount : %d\n",
               ArrayDataCount);
      ArrayDataCount = 0;
    }
  if (ClassCount != 0)
    {
      fprintf (stderr, "Bad count for ClassCount : %d\n",
               ClassCount);
      ClassCount = 0;
    }
  if (ClassNameCount != 0)
    {
      fprintf (stderr, "Bad count for ClassNameCount : %d\n",
               ClassNameCount);
      ClassNameCount = 0;
    }
  if (ObjectCount != 0)
    {
      fprintf (stderr, "Bad count for ObjectCount : %d\n",
               ObjectCount);
      ObjectCount = 0;
    }
  if (ReferenceCount != 0)
    {
      fprintf (stderr, "Bad count for ReferenceCount : %d\n",
               ReferenceCount);
      ReferenceCount = 0;
    }
}

/*----------------------------------------------------------*/
static void AArrayInit (AArray This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  This->array       = NULL;
  This->allocated   = 0;
  This->used        = 0;
  This->free        = 0;
}

/*----------------------------------------------------------*/
static void AArrayClear (AArray This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  AArrayErase (This);

  if (This->array != NULL)
    {
      GCFree (This->array);
      ArrayDataAllocated -= This->allocated * sizeof (void*);
      ArrayDataCount--;
      
      This->array     = NULL;
      This->allocated = 0;
    }
}

/*----------------------------------------------------------*/
static void AArrayErase (AArray This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  if (This->array != NULL)
    {
      GCFree (This->array);
      ArrayDataAllocated -= This->allocated * sizeof (void*);
      ArrayDataCount--;
      
      This->array     = NULL;
      This->allocated = 0;

      This->free = 0;
      This->used = 0;
    }
}

/*----------------------------------------------------------*/
static int AArrayGetUsed (AArray This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (0);

  return (This->used);
}

/*----------------------------------------------------------*/
static int AArrayCount (AArray This)
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
static void AArrayAdd (AArray This, void* object)
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
                  GCMalloc (This->allocated * sizeof (void*));
              ArrayDataAllocated += This->allocated * sizeof (void*);
              ArrayDataCount++;
            }
          else
            {
              ArrayLock ();
              
              This->array = (void**)
                  ARealloc (This->array,
                           This->allocated * sizeof (void*));
              
              ArrayDataAllocated += QUANTUM * sizeof (void*);
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

  This->array[pos] = object;
}

/*----------------------------------------------------------*/
static void AArrayRemove (AArray This, void* object)
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
          This->array[pos] = NULL;
          
          This->free++;

          if (This->free == This->used)
            {
              GCFree (This->array);
              ArrayDataAllocated -= This->allocated * sizeof (void*);
              ArrayDataCount--;
      
              This->array     = NULL;
              This->allocated = 0;

              This->free = 0;
              This->used = 0;
            }
          else if (This->free > This->used)
            {
              fprintf (stderr, "Too much remove...\n");
            }
          break;
        }
    }
}

/*----------------------------------------------------------*/
static void* AArrayItem (AArray This, AIterator it)
/*----------------------------------------------------------*/
{
  int index;

  if (This == NULL) return (NULL);
  if (This->array == NULL) return (NULL);
  if (it == NULL) return (NULL);
  
  if (AIteratorFinished(it)) return (NULL);

  index = AIteratorIndex (it);

  if (index < 0) return (NULL);
  if (index >= This->used) return (NULL);

  return (This->array[index]);
}

/*----------------------------------------------------------*/
static void AArrayRemoveAt (AArray This, int index)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;
  if (This->array == NULL) return;
  if ((index < 0) || (index > This->used)) return;

  if (This->array[index] == NULL) return;
  
  This->array[index] = NULL;
  
  This->free++;

  if (This->free == This->used)
    {
      GCFree (This->array);
      ArrayDataAllocated -= This->allocated * sizeof (void*);
      ArrayDataCount--;
      
      This->array     = NULL;
      This->allocated = 0;
      
      This->free = 0;
      This->used = 0;
    }
  else if (This->free > This->used)
    {
      fprintf (stderr, "Too much remove...\n");
    }
}

/*----------------------------------------------------------*/
static void* AArrayItemAt (AArray This, int index)
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
static void* AArrayFetchItemAt (AArray This, int index)
/*----------------------------------------------------------*/
{
  void* item = NULL;
  
  ArrayLock ();
  item = This->array[index];
  ArrayUnlock ();

  return (item);
}

/*----------------------------------------------------------*/
static void* AArrayFind (AArray This, void* object)
/*----------------------------------------------------------*/
{
  AArrayIteratorRec iterator;
  AIterator it = (AIterator) &iterator;
  void* result = NULL;
  
  if (This == NULL) return (NULL);
  if (object == NULL) return (NULL);

  for (AForwardIteratorInit (&iterator, This); 
       !AIteratorFinished (it); 
       AIteratorForth (it))
    {
      void* item;
      
      item = AIteratorItem (it);
      if (item == object)
        {
          result = item;
          break;
        }
    }

  return (result);
}

/*---------------------------------------------------*/
/*                                                   */
/*    Hasher                                         */
/*                                                   */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
static void CPtrHasherInit (AHasher hasher, AValuator valuator)
/*---------------------------------------------------*/
{
  AHasherInit (hasher, 
               valuator,
               (ACoder) CPtrCoder,
               (ATester) CPtrTester);
}

/*---------------------------------------------------*/
static void CStrHasherInit (AHasher hasher, AValuator valuator)
/*---------------------------------------------------*/
{
  AHasherInit (hasher,
               valuator,
               (ACoder) CStrCoder,
               (ATester) CStrTester);
}

/*---------------------------------------------------*/
static void AHasherInit (AHasher hasher, 
                         AValuator valuator,
                         ACoder coder,
                         ATester tester)
/*---------------------------------------------------*/
{
  int index;
  AArray array;

  if (hasher == NULL) return;

  if (valuator == NULL) return;
  if (coder == NULL) return;
  if (tester == NULL) return;

  hasher->used     = 0;
  hasher->valuator = valuator;
  hasher->coder    = coder;
  hasher->tester   = tester;

  array = &hasher->table[0];
  for (index = 0; index < HASHERSIZE; index++)
    {
      AArrayInit (array);
      array++;
    }
}

/*---------------------------------------------------*/
static void AHasherClear (AHasher hasher)
/*---------------------------------------------------*/
{
  int index;
  AArray array;
  
  if (hasher == NULL) return;

  array = &hasher->table[0];
  for (index = 0; index < HASHERSIZE; index++)
    {
      AArrayClear (array);
      array++;
    }
  
  hasher->used = 0;

  hasher->valuator = NULL;
  hasher->coder    = NULL;
  hasher->tester   = NULL;
}

/*---------------------------------------------------*/
static void AHasherErase (AHasher hasher)
/*---------------------------------------------------*/
{
  int index;
  AArray array;
  
  if (hasher == NULL) return;

  array = &hasher->table[0];
  for (index = 0; index < HASHERSIZE; index++)
    {
      AArrayErase (array);
      array++;
    }
  hasher->used = 0;
}

/*---------------------------------------------------*/
static void AHasherAdd (AHasher hasher, void* object)
/*---------------------------------------------------*/
{
  int code;
  const void* value;

  value = hasher->valuator(object);
  if (value == NULL) return;

  code = hasher->coder (value);
  code %= HASHERSIZE;

  AArrayAdd (&hasher->table[code], object);
  hasher->used++;
}

/*---------------------------------------------------*/
static void* AHasherFind (AHasher hasher, const void* value)
/*---------------------------------------------------*/
{
  void* result = NULL;
  AHashPositionRec position;
  int status;

  status = AHasherHas (hasher, value, &position);
  if (status)
    {
      result = AHasherItemAt (hasher, &position);
    }

  return (result);
}

/*---------------------------------------------------*/
static int AHasherHas (AHasher hasher, const void* value,
                       AHashPosition position)
/*---------------------------------------------------*/
{
  int result = 0;
  int code;
  int pos;
  AArray array;
  int max_pos;

  if (value == NULL) return (0);

  code = hasher->coder (value);
  code %= HASHERSIZE;

  array = &hasher->table[code];

  position->code = code;
  
  max_pos = AArrayGetUsed (array);
  for (pos = 0; pos < max_pos; pos++)
    {
      void* item;
      void* theValue;

      item = AArrayFetchItemAt (array, pos);
      
      theValue = hasher->valuator (item);
      if (hasher->tester (theValue, value))
        {
          position->index = pos;
  
          result = 1;
          break;
        }
    }

  return (result);
}

/*---------------------------------------------------*/
static void* AHasherItemAt (AHasher hasher, AHashPosition position)
/*---------------------------------------------------*/
{
  void* result = NULL;

    /*
      if (hasher == NULL) return (NULL);
      if (position == NULL) return (NULL);
      */
  
  result = AArrayItemAt (&hasher->table[position->code], position->index);

  return (result);
}

/*---------------------------------------------------*/
static void AHasherRemove (AHasher hasher, void* object)
/*---------------------------------------------------*/
{
  AHashPositionRec position;
  void* value;
  int status;

  if (hasher == NULL) return;
  if (object == NULL) return;

  value = hasher->valuator (object);
  if (value == NULL) return;

  status = AHasherHas (hasher, value, &position);
  if (status)
    {
      AArrayRemoveAt (&hasher->table[position.code], position.index);
      hasher->used--;
    }
}

/*----------------------------------------------------------*/
/*                                                          */
/*    Iterator                                              */
/*                                                          */
/*----------------------------------------------------------*/

/*----------------------------------------------------------*/
static int AIteratorFinished (AIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (1);

  return (This->finished);
}

/*----------------------------------------------------------*/
static int AIteratorRunning (AIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (0);

  return (This->running);
}

/*----------------------------------------------------------*/
static void AIteratorFirst (AIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  if (This->Class->first != NULL) This->Class->first (This);
}

/*----------------------------------------------------------*/
static void AIteratorForth (AIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;
  if (This->finished) return;

  if (This->Class->forth != NULL) This->Class->forth (This);
}

/*----------------------------------------------------------*/
static void AIteratorClear (AIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  if (This->Class->clear != NULL) This->Class->clear (This);
}

/*----------------------------------------------------------*/
static int AIteratorIndex (AIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (-1);

  if (This->Class->mindex != NULL) return (This->Class->mindex (This));
  return (-1);
}

/*----------------------------------------------------------*/
static void* AIteratorItem (AIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (NULL);

  if (This->Class->item != NULL) return (This->Class->item (This));
  return (NULL);
}

/*----------------------------------------------------------*/
static void AIteratorStop (AIterator This)
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

static void AArrayIteratorFirst (AArrayIterator This);
static void AArrayIteratorClear (AArrayIterator This);
static int AArrayIteratorIndex (AArrayIterator This);
static void* AArrayIteratorItem (AArrayIterator This);

static void AForwardIteratorForth (AArrayIterator This);

static AIteratorClassRec AForwardIteratorClass = 
{
  "forward",
  (AIteratorMethod) AArrayIteratorFirst,
  (AIteratorMethod) AForwardIteratorForth,
  (AIteratorMethod) AArrayIteratorClear,
  (AIteratorIndexMethod) AArrayIteratorIndex,
  (AIteratorItemMethod) AArrayIteratorItem
};

static void ABackwardIteratorForth (AArrayIterator This);

static AIteratorClassRec ABackwardIteratorClass = 
{
  "backward",
  (AIteratorMethod) AArrayIteratorFirst,
  (AIteratorMethod) ABackwardIteratorForth,
  (AIteratorMethod) AArrayIteratorClear,
  (AIteratorIndexMethod) AArrayIteratorIndex,
  (AIteratorItemMethod) AArrayIteratorItem
};

/*----------------------------------------------------------*/
static void AForwardIteratorInit (AArrayIterator This, AArray array)
/*----------------------------------------------------------*/
{
  This->base.Class = &AForwardIteratorClass;
  if (array == NULL)
    {
      This->base.running = 0;
      This->base.finished = 1;
      This->array = NULL;
    }
  else
    {
      This->base.running = 1;
      This->array = array;
      AIteratorFirst ((AIterator) This);
    }
}

/*----------------------------------------------------------*/
static void ABackwardIteratorInit (AArrayIterator This, AArray array)
/*----------------------------------------------------------*/
{
  This->base.Class = &ABackwardIteratorClass;
  if (array == NULL)
    {
      This->base.running = 0;
      This->base.finished = 1;
      This->array = NULL;
    }
  else
    {
      This->base.running = 1;
      This->array = array;
      AIteratorFirst ((AIterator) This);
    }
}

/*----------------------------------------------------------*/
static void AArrayIteratorClear (AArrayIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  This->base.finished = 1;
  This->base.running = 0;
  This->array = NULL;
  This->current = -1;
}

/*----------------------------------------------------------*/
static void AArrayIteratorFirst (AArrayIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  This->base.finished = 0;
  This->current  = -1;

  AIteratorForth ((AIterator) This);
}

/*----------------------------------------------------------*/
static void AForwardIteratorForth (AArrayIterator This)
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
static void ABackwardIteratorForth (AArrayIterator This)
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
static int AArrayIteratorIndex (AArrayIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (-1);

  return (This->current);
}

/*----------------------------------------------------------*/
static void* AArrayIteratorItem (AArrayIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (NULL);
  if (This->base.finished) return (NULL);

  return (AArrayItemAt (This->array, This->current));
}

/*---------------------------------------------------*/
/*                                                   */
/*    ArrayIterator                                  */
/*                                                   */
/*---------------------------------------------------*/

static void AHasherIteratorFirst (AHasherIterator This);
static void AHasherIteratorForth (AHasherIterator This);
static void AHasherIteratorClear (AHasherIterator This);
static int AHasherIteratorIndex (AHasherIterator This);
static void* AHasherIteratorItem (AHasherIterator This);

static AIteratorClassRec AHasherIteratorClass = 
{
  "hasher",
  (AIteratorMethod) AHasherIteratorFirst,
  (AIteratorMethod) AHasherIteratorForth,
  (AIteratorMethod) AHasherIteratorClear,
  (AIteratorIndexMethod) AHasherIteratorIndex,
  (AIteratorItemMethod) AHasherIteratorItem
};

/*----------------------------------------------------------*/
static void AHasherIteratorInit (AHasherIterator This, AHasher hasher)
/*----------------------------------------------------------*/
{
  This->base.Class = &AHasherIteratorClass;

  This->hasher   = hasher;
  This->current  = -1;
  AForwardIteratorInit (&This->iterator, NULL);

  if (hasher == NULL)
    {
      This->base.running = 0;
      This->base.finished = 1;
    }
  else
    {
      This->base.running = 1;
      AIteratorFirst ((AIterator) This);
    }
}

/*----------------------------------------------------------*/
static void AHasherIteratorClear (AHasherIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  This->base.finished = 1;
  This->base.running = 0;
  This->hasher   = NULL;
  This->current  = -1;
  AArrayIteratorClear (&This->iterator);
}

/*----------------------------------------------------------*/
static void AHasherIteratorFirst (AHasherIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return;

  This->base.finished = 0;
  This->current       = -1;

  AArrayIteratorClear (&This->iterator);
  
  AIteratorForth ((AIterator) This);
}

/*----------------------------------------------------------*/
static void AHasherIteratorForth (AHasherIterator This)
/*----------------------------------------------------------*/
{
  AIterator it;
  int pos;
  
  This->base.finished = 1;

  if (This->hasher == NULL) return;
  if (This->hasher->used == 0) return;

  it = (AIterator) &This->iterator;

  if (AIteratorRunning (it))
    {
      AIteratorForth (it);
      if (AIteratorFinished (it))
        {
          AIteratorClear (it);
        }
      else
        {
          This->base.finished = 0;
          return;
        }
    }
  
  This->current++;
  
  for (pos = This->current; pos < HASHERSIZE; pos++)
    {
      if (!AIteratorRunning (it))
        {
          AForwardIteratorInit (&This->iterator, &This->hasher->table[pos]);
        }
      else
        {
          AIteratorForth (it);
        }
      
      if (AIteratorFinished (it))
        {
          AIteratorClear (it);
        }
      else
        {
          This->base.finished = 0;
          This->current  = pos;
          break;
        }
    }
}

/*----------------------------------------------------------*/
static int AHasherIteratorIndex (AHasherIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (-1);
  if (This->base.finished) return (-1);

  return (This->current);
}

/*----------------------------------------------------------*/
static void* AHasherIteratorItem (AHasherIterator This)
/*----------------------------------------------------------*/
{
  if (This == NULL) return (NULL);
  if (This->base.finished) return (NULL);

  return (AIteratorItem ((AIterator) &This->iterator));
}

/*---------------------------------------------------*/
/*                                                   */
/*    Classes                                        */
/*                                                   */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
GCClass GCClassNew (char* name, GCDeleteMethod erase)
/*---------------------------------------------------*/
{
  char temp[20];
  static int id;
      
  GCClass This;

  if (name == NULL)
    {
      sprintf (temp, "C%d", id);
      name = (char*) temp;
      id++;
    }

  if (!Initialized)
    {
      GCStartup ();
    }

  This = GCClassFind (name);
  if (This == NULL)
    {
      This = (GCClass) GCMalloc (sizeof (GCClassRec));
      ClassAllocated += sizeof (GCClassRec);
      ClassCount++;
  
      This->name = (char*) GCMalloc (strlen(name) + 1);
      ClassNameAllocated += strlen(name) + 1;
      ClassNameCount++;
      strcpy (This->name, name);
      
      This->erase = NULL;
      This->objects = 0;

      if (ClassClass == NULL) ClassClass = This;

      GCObjectNew (This, ClassClass);
      AHasherAdd (Classes, This);
    }
  
  This->erase   = erase;

  return (This);
}

/*---------------------------------------------------*/
void GCClassDelete (GCClass This)
/*---------------------------------------------------*/
{
  if (This == NULL) return;

  if (This->objects != 0) return;

  AHasherRemove (Classes, This);
  
  if (This->name != NULL)
    {
      ClassNameAllocated -= strlen (This->name) + 1;
      GCFree (This->name);
      ClassNameCount--;
      
      This->name = NULL;
    }

  This->erase = NULL;

  GCFree (This);
  ClassAllocated -= sizeof (GCClassRec);
  ClassCount--;
}

/*---------------------------------------------------*/
static GCClass GCClassItem (AIterator it)
/*---------------------------------------------------*/
{
  return ((GCClass) AIteratorItem (it));
}

/*---------------------------------------------------*/
static GCClass GCClassFind (char* name)
/*---------------------------------------------------*/
{
  AHasherIteratorRec iterator;
  AIterator it = (AIterator) &iterator;
  GCClass This;

  for (AHasherIteratorInit (&iterator, Classes);
       !AIteratorFinished (it);
        AIteratorForth (it))
    {
      This = GCClassItem (it);
      if (!strcmp (This->name, name))
        {
          return (This);
        }
    }
  return (NULL);
}

/*---------------------------------------------------*/
static GCDeleteMethod GCClassGetErase (GCClass This)
/*---------------------------------------------------*/
{
  if (This == NULL) return (NULL);

  return (This->erase);
}

/*---------------------------------------------------*/
static void* GCClassValuator (GCClass This)
/*---------------------------------------------------*/
{
  if (This == NULL) return (NULL);
  
  return (This->name);
}

/*---------------------------------------------------*/
static void GCClassDump (GCClass This)
/*---------------------------------------------------*/
{
  if (This == NULL) return;

  fprintf (stderr, "Class<%lx> %s (%3.3d objects)", 
           (unsigned long) This, This->name,
           This->objects);
}

/*---------------------------------------------------*/
/*                                                   */
/*    Objects                                        */
/*                                                   */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
GCObject GCObjectNew (void* address, GCClass base)
/*---------------------------------------------------*/
{
  GCObject object = NULL;

  if (!Initialized)
    {
      GCStartup ();
    }

  if (Objects == NULL) return (NULL);

  object = (GCObject) GCMalloc (sizeof (GCObjectRec));
  ObjectAllocated += sizeof (GCObjectRec);
  ObjectCount++;

  ObjectId++;

  object->alive      = 1;
  object->base       = NULL;
  object->id         = ObjectId;
  object->address    = address;
  object->references = 0;
  object->used       = 0;

  AHasherAdd (Objects, object);

  object->base = base;
  if (base != NULL) base->objects++;

  return (object);
}

/*---------------------------------------------------*/
static GCObject GCObjectItem (AIterator it)
/*---------------------------------------------------*/
{
  return ((GCObject) AIteratorItem (it));
}

/*---------------------------------------------------*/
static GCObject GCObjectFind (const void* address)
/*---------------------------------------------------*/
{
  GCObject object;

  object = (GCObject) AHasherFind (Objects, address);

  return (object);
}

/*---------------------------------------------------*/
static void GCObjectReference (GCObject object)
/*---------------------------------------------------*/
{
  if (object == NULL) return;
  if (!object->alive) return;

  object->references++;
}

/*---------------------------------------------------*/
static void GCObjectUnreference (GCObject object)
/*---------------------------------------------------*/
{
  if (object == NULL) return;
  if (!object->alive) return;

  if (object->references <= 0)
    {
      fprintf (stderr, "Unreferencing too much the object %lx\n",
               (unsigned long) object->address);
      return;
    }
  
  object->references--;

  if (object->references == 0)
    {
      GCObjectDelete (object);
    }
}

/*---------------------------------------------------*/
static void GCObjectDelete (GCObject object)
/*---------------------------------------------------*/
{
  if (object == NULL) return;
  if (!object->alive) return;

  ArrayLock ();
  
  AHasherRemove (Objects, object);

  if (object->base != NULL)
    {
      GCDeleteMethod erase;

      erase = GCClassGetErase (object->base);

      if (erase != NULL) erase (object->address);

      object->base->objects--;
    }
  
  object->alive = 0;
  object->base  = NULL;

  object->address    = NULL;
  object->references = 0;

  GCFree (object);
  ObjectAllocated -= sizeof (GCObjectRec);
  ObjectCount--;

  ArrayLock ();  
}

/*---------------------------------------------------*/
static void GCObjectSetUsed (GCObject object)
/*---------------------------------------------------*/
{
  if (object == NULL) return;
  if (!object->alive) return;

  object->used = 1;
}

/*---------------------------------------------------*/
static void GCObjectResetUsed (GCObject object)
/*---------------------------------------------------*/
{
  if (object == NULL) return;
  if (!object->alive) return;

  object->used = 0;
}

/*---------------------------------------------------*/
static int GCObjectGetUsed (GCObject object)
/*---------------------------------------------------*/
{
  if (object == NULL) return (0);
  if (!object->alive) return (0);

  return (object->used);
}

/*---------------------------------------------------*/
static void GCObjectDump (GCObject object)
/*---------------------------------------------------*/
{
  if (object == NULL) return;
  if (!object->alive) return;

  fprintf (stderr, "Object<%lx>", (unsigned long) object);
  fprintf (stderr, " @%lx (%3.3d)", (unsigned long) object->address, 
           object->references);
  fprintf (stderr, " id=%3.3d", object->id);
  
  if (object->base != NULL)
    {
      fprintf (stderr, ":%s", object->base->name);
    }
  else
    {
      fprintf (stderr, ":");
    }
  
  if (object->used) fprintf (stderr, " (used)");
}

/*---------------------------------------------------*/
static void GCObjectDumpId (GCObject object)
/*---------------------------------------------------*/
{
  if (object == NULL) return;
  if (!object->alive) return;

  fprintf (stderr, "%3.3d", object->id);
  
  if (object->base != NULL)
    {
      fprintf (stderr, ":%s", object->base->name);
    }
  else
    {
      fprintf (stderr, ":");
    }
}

/*---------------------------------------------------*/
static void* GCObjectValuator (GCObject object)
/*---------------------------------------------------*/
{
  if (object == NULL) return (NULL);
  
  return (object->address);
}

/*---------------------------------------------------*/
/*                                                   */
/*    References                                     */
/*                                                   */
/*---------------------------------------------------*/

/*---------------------------------------------------*/
static GCReference GCReferenceNew (void* address)
/*---------------------------------------------------*/
{
  GCReference reference;

  if (!Initialized)
    {
      GCStartup ();
    }

  reference = (GCReference) GCMalloc (sizeof (GCReferenceRec));
  ReferenceAllocated += sizeof (GCReferenceRec);
  ReferenceCount++;

  ReferenceId++;

  reference->alive      = 1;
  reference->id         = ReferenceId;
  reference->address    = (void**) address;
  reference->from       = NULL;
  reference->to         = NULL;

  AHasherAdd (References, reference);

  return (reference);
}

/*---------------------------------------------------*/
GCReference GCReferenceSet (void** address, const void* from, const void* to)
/*---------------------------------------------------*/
{
  GCReference reference = NULL;
  GCObject oFrom;
  GCObject oTo;
  ConstPointer* cpointer = (ConstPointer*) address;

  if (!Initialized)
    {
      GCStartup ();
    }
  
  if (address == NULL) return (NULL); /* Not a real reference. */
  
  if (*address == to) return (NULL);  /* The reference is already installed. */

  reference = GCReferenceFind (address);

  if (to == NULL)
    {
        /*
          Clearing a reference.

          The 'from' argument is unused here.
          */
        /*
      if (to != NULL)
        {
          fprintf (stderr, "Warning:"
                           " trying to reference an undeclared object.\n");
        }
        */
      if (reference != NULL) GCReferenceDelete (reference);
      return (NULL);
    }

    /*
      Installing a reference.
      */
  
  oTo = GCObjectFind (to);
  if (oTo == NULL)
    {
        /*
          This is an undeclared object. We declare it under the class
          'GCUndeclared'
          */
      oTo = GCObjectNew ((void*) to, UndeclaredClass);
    }

  oFrom = GCObjectFind (from);
  if ((from != NULL) && (oFrom == NULL))
    {
        /*
          This is an undeclared object. We declare it under the class
          'GCUndeclared'
          */
      oFrom = GCObjectNew ((void*) from, UndeclaredClass);
    }

  ArrayLock ();
  
  if (reference == NULL)
    {
        /*
          A new reference.
          */
      reference = GCReferenceNew (address);

        /* Install this new reference */
      
      GCObjectReference (oFrom);
      reference->from = oFrom;

      GCObjectReference (oTo);
      reference->to = oTo;
      
      if (reference->to != NULL) *cpointer = to;
    }
  else
    {
        /* Change this reference */
      
      if (oTo != reference->to)
        {
          if (reference->to != NULL)
            {
              *address = NULL;
              GCObjectUnreference (reference->to);
              reference->to = NULL;
            }

          GCObjectReference (oTo);
          reference->to = oTo;
          *cpointer = to;
        }

      if (oFrom != reference->from)
        {
          AHasherIteratorRec iterator;
          AIterator it = (AIterator) &iterator;
          GCReference a;
          
          fprintf (stderr, "A reference has changed its origin.\n");

          GCReferenceDump (reference);
          fprintf (stderr, "\n");
          
          for (AHasherIteratorInit (&iterator, References); 
               !AIteratorFinished (it); 
               AIteratorForth (it))
            {
              a = GCReferenceItem (it);
              GCReferenceDump (a);
              fprintf (stderr, "\n");
            }
        }
  
    }
  
  ArrayUnlock ();
  
  return (reference);
}

/*---------------------------------------------------*/
static GCReference GCReferenceItem (AIterator it)
/*---------------------------------------------------*/
{
  return ((GCReference) AIteratorItem (it));
}

/*---------------------------------------------------*/
static GCReference GCReferenceFind (void** address)
/*---------------------------------------------------*/
{
  GCReference reference;

  reference = (GCReference) AHasherFind (References, address);

  return (reference);
}

/*---------------------------------------------------*/
static void GCReferenceDelete (GCReference reference)
/*---------------------------------------------------*/
{
  if (reference == NULL) return;
  if (!reference->alive) return;

  ArrayLock ();

  AHasherRemove (References, reference);

  reference->alive = 0;

  *(reference->address) = NULL;
  
  GCObjectUnreference (reference->to);
  reference->to = NULL;
  
  GCObjectUnreference (reference->from);
  reference->from = NULL;
  
  GCFree (reference);
  ReferenceAllocated -= sizeof (GCReferenceRec);
  ReferenceCount--;

  ArrayUnlock ();
}

/*---------------------------------------------------*/
void GCReferenceMove (void** address, void** newAddress)
/*---------------------------------------------------*/
{
  GCReference reference;

  if (!Initialized)
    {
      GCStartup ();
    }

  if (address == NULL) return;
  if (newAddress == NULL) return;
  if (newAddress == address) return;
  
  reference = GCReferenceFind (address);
  if (reference == NULL) return;

  ArrayLock ();
  
  AHasherRemove (References, reference);
  reference->address = newAddress;
  AHasherAdd (References, reference);
  
  ArrayUnlock ();
}

/*---------------------------------------------------*/
static void GCReferenceDump (GCReference reference)
/*---------------------------------------------------*/
{
  if (reference == NULL) return;
  if (!reference->alive) return;

  fprintf (stderr, "Reference<%lx> @%lx id:%3.3d between [",
           (unsigned long) reference, 
           (unsigned long) reference->address, 
           reference->id);
  GCObjectDumpId (reference->from);
  fprintf (stderr, "] and [");
  GCObjectDumpId (reference->to);
  fprintf (stderr, "]");
}

/*---------------------------------------------------*/
static void* GCReferenceValuator (GCReference reference)
/*---------------------------------------------------*/
{
  if (reference == NULL) return (NULL);
  if (!reference->alive) return (NULL);
  
  return (reference->address);
}

/*---------------------------------------------------*/
void GCDumpBase ()
/*---------------------------------------------------*/
{
  int number;

  AHasherIteratorRec iterator;
  AIterator it = (AIterator) &iterator;

  if (!Initialized)
    {
      GCStartup ();
    }

  fprintf (stderr, "- classes --------------------------\n");

  number = 0;
  for (AHasherIteratorInit (&iterator, Classes); 
       !AIteratorFinished (it); 
       AIteratorForth (it))
    {
      GCClass c;

      number++;
      c = GCClassItem (it);
      GCClassDump (c);
      fprintf (stderr, "\n");
    }

  fprintf (stderr, "-( %3.3d - %3.3d )--------------\n", 
           number,
           Classes->used);

  fprintf (stderr, "- objects --------------------------\n");

  number = 0;
  for (AHasherIteratorInit (&iterator, Objects); 
       !AIteratorFinished (it); 
       AIteratorForth (it))
    {
      GCObject object;

      number++;
      object = GCObjectItem (it);
      GCObjectDump (object);
      fprintf (stderr, "\n");
    }

  fprintf (stderr, "-( %3.3d - %3.3d )--------------\n", 
           number,
           Objects->used);

  fprintf (stderr, "- references --------------------------\n");

  number = 0;
  for (AHasherIteratorInit (&iterator, References); 
       !AIteratorFinished (it); 
       AIteratorForth (it))
    {
      GCReference reference;

      number++;
      reference = GCReferenceItem (it);
      GCReferenceDump (reference);
      fprintf (stderr, "\n");
    }

  fprintf (stderr, "-( %3.3d - %3.3d )--------------\n", 
           number,
           References->used);

  fprintf (stderr, "- allocated %d %d %d %d %d -\n", 
           ArrayDataAllocated,
           ClassAllocated,
           ClassNameAllocated,
           ObjectAllocated,
           ReferenceAllocated);
  fprintf (stderr, "- mallocs %d -\n", MallocCount);
}

/*---------------------------------------------------*/
void GCCleanup ()
/*---------------------------------------------------*/
/*                                                   */
/*  The principle :                                  */
/*    References that contain a reference to NULL    */
/*    (either 'from' or 'to') are considered to be   */
/*    attached to external references.               */
/*                                                   */
/*   Therefore they cannot be dropped. Others        */
/*   chain internal objects to themselves thus       */
/*   the entire chain is considered to be unlinked   */
/*   and can be dropped.                             */
/*                                                   */
/*---------------------------------------------------*/
{
  AHasherIteratorRec iterator;
  AIterator it;

  if (!Initialized)
    {
      GCStartup ();
    }

  AHasherIteratorInit (&iterator, Objects);
  it = (AIterator) &iterator;

  for (AHasherIteratorInit (&iterator, Objects); 
       !AIteratorFinished (it); 
       AIteratorForth (it))
    {
      GCObject object;

      object = GCObjectItem (it);
      GCObjectResetUsed (object);
    }

  for (AHasherIteratorInit (&iterator, References); 
       !AIteratorFinished (it); 
       AIteratorForth (it))
    {
      GCReference reference;

      reference = GCReferenceItem (it);
      if (reference->from == NULL) GCObjectSetUsed (reference->to);
      if (reference->to   == NULL) GCObjectSetUsed (reference->from);
    }

  for (;;)
    {
      GCReference reference;
      int found = 0;

      for (AHasherIteratorInit (&iterator, References); 
           !AIteratorFinished (it); 
           AIteratorForth (it))
        {
          reference = GCReferenceItem (it);
          
          if (GCObjectGetUsed (reference->from))
            {
              if ((reference->to != NULL) && 
                  !GCObjectGetUsed (reference->to))
                {
                  GCObjectSetUsed (reference->to);
                  found = 1;
                }
            }
          else if (GCObjectGetUsed (reference->to))
            {
              if ((reference->from != NULL) && 
                  !GCObjectGetUsed (reference->from))
                {
                  GCObjectSetUsed (reference->from);
                  found = 1;
                }
            }
        }
      if (!found) break;
    }

  for (AHasherIteratorInit (&iterator, References); 
       !AIteratorFinished (it); 
       AIteratorForth (it))
    {
      GCReference reference;

      reference = GCReferenceItem (it);
      if (!GCObjectGetUsed (reference->from) &&
          !GCObjectGetUsed (reference->to))
        {
          GCReferenceDelete (reference);
        }
    }
}


#ifdef GCREFERENCEDEBUG

typedef struct _AllocRec* Alloc;
typedef struct _AllocRec
{
  size_t size;
  char* ptr;
} AllocRec;

static AllocRec A[1000];
static int Allocs = 0;
typedef double AMarker;
static AMarker AMagic = 1234.5678;

/*---------------------------------------------------*/
static void AMCheck ()
/*---------------------------------------------------*/
{
  int i;
  Alloc a;

  a = &A[0];
  
  for (i = 0; i < Allocs - 1; i++)
    {
      AMarker* lp;
      
      if (a[0].ptr == NULL) continue;
      if (a[0].ptr + a[0].size > a[1].ptr)
        {
          fprintf (stderr, "Overlap in memory\n");
        }

      lp = (AMarker*) a[0].ptr;
      if (*lp != AMagic)
        {
          fprintf (stderr, "Wrong header\n");
        }
      lp = (AMarker*) (a[0].ptr + a[0].size - sizeof(AMarker));
      if (*lp != AMagic)
        {
          fprintf (stderr, "Wrong tail\n");
        }
    
      a++;
    }
}

/*---------------------------------------------------*/
static void AMDump ()
/*---------------------------------------------------*/
{
  int i;
  Alloc a;

  a = &A[0];
  
  fprintf (stderr, "Allocations--------------\n");
  for (i = 0; i < Allocs; i++)
    {
      if (a->ptr == NULL) continue;
      fprintf (stderr, "Alloc from %lx to %lx (%d bytes)\n",
               a->ptr,
               a->ptr + a->size - 1,
               a->size);
      a++;
    }
  fprintf (stderr, "Allocations--------------\n");
  AMCheck ();
}

/*---------------------------------------------------*/
static int AMCompare (Alloc a1, Alloc a2)
/*---------------------------------------------------*/
{
  if (a1->ptr == a2->ptr) return (0);
  if (a1->ptr < a2->ptr) return (-1);
  return (1);
}

/*---------------------------------------------------*/
static void AMRemove (void* ptr)
/*---------------------------------------------------*/
{
  Alloc a;
  int i;

  a = &A[0];
  for (i = 0; i < Allocs; i++)
    {
      if (a->ptr == ptr)
        {
          a->ptr = NULL;
          a->size = 0;
          break;
        }
      a++;
    }

  qsort ((void*) &A[0], Allocs, sizeof(AllocRec), AMCompare);
}
  
/*---------------------------------------------------*/
static void AMStore (void* ptr, size_t bytes)
/*---------------------------------------------------*/
{
  Alloc a;
  int i;
  
  a = &A[0];
  for (i = 0; i < Allocs; i++)
    {
      if (a->ptr == NULL)
        {
          break;
        }
      a++;
    }
  
  a->size = bytes;
  a->ptr  = ptr;

  if (i == Allocs) Allocs++;

  qsort ((void*) &A[0], Allocs, sizeof(AllocRec), AMCompare);
}

/*---------------------------------------------------*/
static void* AMalloc (size_t bytes)
/*---------------------------------------------------*/
{
  char* p;

  bytes += 2 * sizeof (AMarker);
  p = (char*) malloc (bytes);

  AMStore (p, bytes);

  {
    AMarker* lp;

    lp = (AMarker*) p;
    *lp = AMagic;
    lp = (AMarker*) (p + bytes - sizeof(AMarker));
    *lp = AMagic;
  }

    /*
      fprintf (stderr, "AMalloc>---------------------------------------\n");
      AMDump ();
      */
  AMCheck ();
  
  return (p + sizeof(AMarker));
}

/*---------------------------------------------------*/
static void* ARealloc (void* ptr, size_t bytes)
/*---------------------------------------------------*/
{
  char* p;
  
  AMRemove (ptr);
  
  bytes += 2 * sizeof (AMarker);
  p = (char*) realloc (ptr, bytes);

  AMStore (p, bytes);

  {
    AMarker* lp;

    lp = (AMarker*) p;
    *lp = AMagic;
    lp = (AMarker*) (p + bytes - sizeof(AMarker));
    *lp = AMagic;
  }

    /*
      fprintf (stderr, "ARealloc>---------------------------------------\n");
      AMDump ();
      */
  AMCheck ();
  
  return (p + sizeof(AMarker));
}

/*---------------------------------------------------*/
static void AFree (void* ptr)
/*---------------------------------------------------*/
{
  AMRemove (ptr);
  
  free (ptr);

    /*
      fprintf (stderr, "AFree>---------------------------------------\n");
      AMDump ();
      */
  AMCheck ();
}

#endif  /* GCREFERENCEDEBUG */



