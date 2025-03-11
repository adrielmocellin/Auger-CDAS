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
        HASH.C
         Created           : 15-SEP-1992 by Christian Arnault
*/
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <Hash.h>

/*- Private definitions -----------------------------------------------*/

typedef struct _HDArrayRec* HDArray;

typedef struct _HDArrayRec
{
  HReference* references;
  int entries;
  int size;
} HDArrayRec;

static int HDArrayQuantum = 10;

static int HashTableCount      = 0;
static int HashDArrayCount     = 0;
static int HIteratorCount      = 0;
static int HReferencesCount    = 0;

typedef struct _HTableRec
{
  int size;
  HNamer namer;

  HDArrayRec* darrays;        /* An array of HDArrayRec's     */
  
  /* Statistics infos */
  int totalEntries;
} HTableRec;

typedef struct _HIteratorRec
{
  HTable table;

  int code;
  int position;
} HIteratorRec;

#ifdef __cplusplus
extern "C"
{
#endif
static char* HDefaultNamer (HReference reference);
static void HDArrayInit (HDArray This);
static void HDArrayClear (HDArray This);
static void HDArrayAddEntry (HDArray This, HReference reference);
static HReference HDArrayEntry (HDArray This, int position);
static void HDArrayRemoveEntry (HDArray This, HReference reference);
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                                                                     */
/*  HashTable management.                                              */
/*                                                                     */
/*---------------------------------------------------------------------*/

/*- procedure ---------------------------------------------------------*/
static char* HDefaultNamer (HReference reference)
/*---------------------------------------------------------------------*/
{
  return ((char*) reference);
}

/*- procedure ---------------------------------------------------------*/
void HTableCleanup ()
/*---------------------------------------------------------------------*/
{
  if (HashTableCount != 0)
  {
  	printf ("HashTableCount = %d\n", HashTableCount);
  }
  if (HashDArrayCount != 0)
  {
  	printf ("HashDArrayCount = %d\n", HashDArrayCount);
  }
  if (HIteratorCount != 0)
  {
  	printf ("HIteratorCount = %d\n", HIteratorCount);
  }
  if (HReferencesCount != 0)
  {
  	printf ("HReferencesCount = %d\n", HReferencesCount);
  }
}

/*- procedure ---------------------------------------------------------*/
HTable HTableNew (int size, HNamer namer)
/*---------------------------------------------------------------------*/
{
  HTable This;
  
  This = (HTable) malloc (sizeof(HTableRec));
  HashTableCount++;
  
  This->size  = size;
  This->namer = (namer) ? namer : HDefaultNamer;

  This->darrays = (HDArrayRec*) malloc (sizeof(HDArrayRec) * size);
  HashDArrayCount++;
  
  memset (This->darrays, 0, sizeof(HDArrayRec) * size);
  
  This->totalEntries = 0;
  return (This);
}

/*- procedure ---------------------------------------------------------*/
void HTableDelete (HTable This)
/*---------------------------------------------------------------------*/
{
  HDArray darray;
  int number;

  if (!This) return;

  darray = &This->darrays[0];
  for (number = 0; number < This->size; number++)
  {
    HDArrayClear (darray);
    darray++;
  }
  free ((char*) This->darrays);
  This->darrays = 0;

  HashDArrayCount--;
  
  free ((char*) This);
  HashTableCount--;
}

/*- procedure ---------------------------------------------------------*/
void HTableClear (HTable This, HFunction clear)
/*---------------------------------------------------------------------*/
{
  int code;
  HReference ref;

  if (!This) return;
  
  for (code = 0; code < This->size; code++)
  {
    while ((HTablePositions (This, code)))
    {
      ref = HTableGetReference (This, code, 1);
      if (!ref) break;
      HTableRemoveReference (This, ref);
      if (ref && clear) clear (ref);
    }
  }
}

/*- procedure ---------------------------------------------------------*/
void HTableProcess (HTable This, HFunction action)
/*---------------------------------------------------------------------*/
{
  int code;
  int positions;
  int pos;
  HReference ref;

  if (!This) return;
  if (!action) return;
  
  for (code = 0; code < This->size; code++)
  {
    positions = HTablePositions (This, code);
    for (pos = 1; pos <= positions; pos++)
    {
      ref = HTableGetReference (This, code, pos);
      if (ref && action) action (ref);
    }
  }
}

/*- procedure ---------------------------------------------------------*/
int HTableEncode (HTable This, char* name)
/*---------------------------------------------------------------------*/
{
  int len;
  int code;
  int pos;
  
  if (!This) return (0);

  len = strlen (name);
  code = len;
  
  for (pos = 0; pos < len; pos++)
  {
    code = (code * 613) + (unsigned) name[pos];
  }
  code &= (1 << 30) - 1;
  code %= This->size;
  
  return (code);
}

/*- procedure ---------------------------------------------------------*/
int HTablePosition (HTable This, char* name)
/*---------------------------------------------------------------------*/
{
  int code;
  int pos;
  int position;
  HReference ref;
  char* refName;
  HDArray darray;
  
  if (!This) return (0);

  code = HTableEncode (This, name);

  position = 0;
  darray = &This->darrays[code];
  for (pos = 1; pos <= darray->entries; pos++)
  {
    ref = HDArrayEntry (darray, pos);
    refName = This->namer (ref);
    if (!strcmp (refName, name))
    {
      position = pos;
      break;
    }
  }
  return (position);
}

/*- procedure ---------------------------------------------------------*/
int HTablePositions (HTable This, int code)
/*---------------------------------------------------------------------*/
{
  HDArray darray;
  
  if (!This) return (0);
  
  if ((code < 0) || (code >= This->size)) return (0);

  darray = &This->darrays[code];
  return (darray->entries);
}

/*- procedure ---------------------------------------------------------*/
HReference HTableFindReference (HTable This, char* name)
/*---------------------------------------------------------------------*/
{
  int code;
  HReference ref;
  char* refName;
  HDArray darray;
  int position;
  
  if (!This) return (0);

  code = HTableEncode (This, name);

  ref = 0;
  darray = &This->darrays[code];
  for (position = 1; position <= darray->entries; position++)
  {
    ref = HDArrayEntry (darray, position);
    refName = This->namer (ref);
    if (!strcmp (refName, name)) break;
    ref = 0;
  }

  return (ref);
}

/*- procedure ---------------------------------------------------------*/
HReference HTableGetReference (HTable This, int code, int position)
/*---------------------------------------------------------------------*/
{
  HReference ref;
  HDArray darray;
  
  if (!This) return (0);

  if ((code < 0) || (code >= This->size)) return (0);
  
  darray = &This->darrays[code];
  ref = HDArrayEntry (darray, position);

  return (ref);
}

/*- procedure ---------------------------------------------------------*/
void HTableAddReference (HTable This, HReference reference)
/*---------------------------------------------------------------------*/
{
  int code;
  char* name;
  HDArray darray;

  /*
    The possible options here for installing a new HReference are :

     1 Assume that this HReference does not exist yet in the HTable.
       (This means that the caller should have previously 
        call HTableGetReference).
    
     2 Check whether it exists already by a match on the name.
    
     3 Check whether it exists already by a match on HReference address.
    
    Currently we implement check #3.

  */

  if (!This) return;

  name = This->namer (reference);
  if (name)
  {
    int position;
    HReference ref;

    code = HTableEncode (This, name);

    darray = &This->darrays[code];
    for (position = 1;; position++)
    {
      ref = (HReference) HDArrayEntry (darray, position);
      if (!ref) break;
      if (ref == reference) return;
    }
    HDArrayAddEntry (darray, reference);
  }
  else
  {
    printf ("Cannot encode HEntry %lx\n", (unsigned long) reference);
/*
    exit(0);
*/
  }
}

/*- procedure ---------------------------------------------------------*/
void HTableRemoveReference (HTable This, HReference reference)
/*---------------------------------------------------------------------*/
{
  int code;
  char* name;
  HDArray darray;
  
  /*
    The possible options here for finding this HReference are :
     o find a match with the exact HReference
     o find a match with the HReference name.
  
    Currently we implement the 1st.
  */

  if (!This) return;
  
  name = This->namer (reference);
  if (name)
  {
    code = HTableEncode (This, name);

    darray = &This->darrays[code];
    HDArrayRemoveEntry (darray, reference);
  }
  else
  {
    printf ("Cannot encode HEntry %lx\n", (unsigned long) reference);
/*
    exit(0);
*/
  }
}

/*=======================================================================*/
/*                                                                       */
/*  HIterator                                                            */
/*                                                                       */
/*=======================================================================*/

/*-----------------------------------------------------------------------*/
HIterator HIteratorNew (HTable table)
/*-----------------------------------------------------------------------*/
{
  HIterator This;

  This = (HIterator) malloc (sizeof(HIteratorRec));
  HIteratorCount++;
  
  This->table = table;
  This->code = -1;
  This->position = -1;

  return (This);
}

/*-----------------------------------------------------------------------*/
void HIteratorDelete (HIterator This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;

  free ((char*) This);
  HIteratorCount--;
}

/*-----------------------------------------------------------------------*/
void HIteratorReset (HIterator This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;

  This->code = -1;
  This->position = -1;
}

/*-----------------------------------------------------------------------*/
HReference HIteratorNext (HIterator This)
/*-----------------------------------------------------------------------*/
{
  int code;
  int position;
  HTable table;
  HDArray darray;
  HReference ref;

  if (!This) return (0);
  if (!(table = This->table)) return (0);

  code = This->code;
  if (code == -1)
  {
    position = -1;
  }
  else position = This->position;

  for (;;)
  {
    if (position == -1)
    {
      /*
         On cherche la premiere valeur de "code" qui contient
         au moins une entree.
          On demarre par code = (au moins) 0.
      */

      code++;
      darray = &table->darrays[code];
      for (; code < table->size; code++)
      {
        if (darray->entries) break;
        darray++;
      }
      if (code >= table->size) return (0);
    }

    /*
       Si on arrive ici, il existe au moins une entree pour la valeur
       de code.
    */

    position++;

    darray = &table->darrays[code];

    for (; position < darray->entries; position++)
    {
      if ((ref = darray->references[position]))
      {
        This->code = code;
        This->position = position;
        return (ref);
      }
    }
    
    position = -1;
  }
}

/*-----------------------------------------------------------------------*/
HReference HIteratorPrevious (HIterator This)
/*-----------------------------------------------------------------------*/
{
  int code;
  int position;
  HTable table;
  HDArray darray;
  HReference ref;

  if (!This) return (0);
  if (!(table = This->table)) return (0);

  code = This->code;
  if (code == -1)
  {
    code = table->size;
    position = -1;
  }
  else position = This->position;

  for (;;)
  {
    if (position == -1)
    {
      /*
         On cherche la derniere valeur de "code" qui contient
         au moins une entree.
          On demarre par code = (au plus) table->size - 1.
      */

      code--;
      darray = &table->darrays[code];
      for (; code >= 0; code--)
      {
        if (darray->entries) break;
        darray--;
      }
      if (code < 0) return (0);
      position = HTablePositions (table, code);
    }

    /*
       Si on arrive ici, il existe au moins une entree pour la valeur
       de code.
    */

    position--;

    darray = &table->darrays[code];

    for (; position >= 0; position--)
    {
      if ((ref = darray->references[position]))
      {
        This->code = code;
        This->position = position;
        return (ref);
      }
    }
    
    position = -1;
  }
}

/*-----------------------------------------------------------------------*/
static void HDArrayInit (HDArray This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;
  
  This->references = 0;
  This->entries    = 0;
  This->size       = 0;
}

/*-----------------------------------------------------------------------*/
static void HDArrayClear (HDArray This)
/*-----------------------------------------------------------------------*/
{
  if (!This) return;
  
  if (This->references)
  {
    free ((char*) This->references);
    This->references = 0;
    HReferencesCount--;
  }
  This->entries    = 0;
  This->size       = 0;
}

/*-----------------------------------------------------------------------*/
static void HDArrayAddEntry (HDArray This, HReference reference)
/*-----------------------------------------------------------------------*/
{
  int size;
  int position;
  
  if (!This) return;
  
  if (!This->references)
  {
    size = This->size = HDArrayQuantum;
    This->references = (HReference*) malloc (sizeof(HReference) * size);
    HReferencesCount++;
    
    memset (This->references, 0, sizeof(HReference) * size);
    This->entries = 0;
  }
  else size = This->size;
  
  /*
    Cherche la premiere entree libre.
  */
  if (This->entries < size)
  {
    position = 0;
    while (This->references[position]) position++;
  }
  else
  {
    int oldSize = This->size;

    size += HDArrayQuantum;
    This->size = size;
    This->references = (HReference*) realloc ((char*) This->references,
      sizeof(HReference) * size);
    memset (&This->references[oldSize], 0,
      sizeof(HReference) * HDArrayQuantum);
    position = oldSize;
  }
  This->references[position] = reference;
  This->entries++;
}

/*-----------------------------------------------------------------------*/
static HReference HDArrayEntry (HDArray This, int position)
/*-----------------------------------------------------------------------*/
{
  int pos;
  HReference ref;
  
  if (!This) return (0);

  if (position <= 0 || position > This->entries) return (0);

  pos = 0;
  while (position)
  {
    ref = 0;
    while ((ref == 0) && (pos < This->size))
    {
      ref = This->references[pos];
      pos++;
    }
    if (!ref) break;
    position--;
  }
  return (ref);
}

/*-----------------------------------------------------------------------*/
static void HDArrayRemoveEntry (HDArray This, HReference reference)
/*-----------------------------------------------------------------------*/
{
  int position;
  
  if (!This) return;

  for (position = 0; position < This->size; position++)
  {
    if (This->references[position] == reference)
    {
      This->references[position] = 0;
      This->entries--;
      if (This->entries < 0)
      {
        printf ("HDArray> Error : entry number < 0!!!\n");
        This->entries = 0;
      }
      break;
    }
  }
}

