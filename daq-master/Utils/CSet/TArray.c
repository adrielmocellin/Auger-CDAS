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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CSet.h>

typedef struct _ARec* A;
typedef struct _BRec* B;

typedef struct _ARec
{
  char* name;
  B b;
} ARec;
static int ACount = 0;
static GCClass AClass = NULL;

typedef struct _BRec
{
  char* name;
  A a;
} BRec;
static int BCount = 0;
static GCClass BClass = NULL;

static GCClass StrClass = NULL;

/*-----------------------------------------------------*/
A ANew (char* name, B b)
/*-----------------------------------------------------*/
{
  A This;

  This = (A) malloc (sizeof (ARec));
  ACount++;
  
  This->name = (char*) malloc (strlen(name) + 1);
  strcpy (This->name, name);

  This->b = NULL;

  GCObjectNew (This, AClass);

  GCReferenceAssignIn (This->b, This, b);

  return (This);
}

/*-----------------------------------------------------*/
void ADelete (A This)
/*-----------------------------------------------------*/
{
  if (!This) return;

  if (This->name)
    {
      free (This->name);
      This->name = NULL;
    }
  
  GCReferenceClear (This->b);

  free (This);
  ACount--;
}

/*-----------------------------------------------------*/
void ASetB (A This, B b)
/*-----------------------------------------------------*/
{
  if (!This) return;

  GCReferenceAssignIn (This->b, This, b);
}

/*-----------------------------------------------------*/
B BNew (char* name, A a)
/*-----------------------------------------------------*/
{
  B This;

  This = (B) malloc (sizeof (BRec));
  BCount++;
  
  This->name = (char*) malloc (strlen(name) + 1);
  strcpy (This->name, name);

  This->a = NULL;

  GCObjectNew (This, BClass);

  GCReferenceAssignIn (This->a, This, a);

  return (This);
}

/*-----------------------------------------------------*/
void BDelete (B This)
/*-----------------------------------------------------*/
{
  if (!This) return;

  if (This->name)
    {
      free (This->name);
      This->name = NULL;
    }
  
  GCReferenceClear (This->a);

  free (This);
  BCount--;
}

/*-----------------------------------------------------*/
void BSetA (B This, A a)
/*-----------------------------------------------------*/
{
  if (!This) return;

  GCReferenceAssignIn (This->a, This, a);
}

static int my_malloc_count = 0;
/*-----------------------------------------------------*/
void my_free (void* ptr)
/*-----------------------------------------------------*/
{
  my_malloc_count--;
  free (ptr);
}

#define OBJECTS 10
/*-----------------------------------------------------*/
int main (int argc, char* argv[])
/*-----------------------------------------------------*/
{
  int loops;
  int a_count;
  int b_count;

  AClass = GCClassNew ("A", (GCDeleteMethod) ADelete);
  BClass = GCClassNew ("B", (GCDeleteMethod) BDelete);
  StrClass = GCClassNew ("Str", (GCDeleteMethod) my_free);

  if (argc > 1)
    sscanf (argv[1], "%d", &loops);
  else
    loops = 1;

  for (; loops > 0; loops--)
    {
      fprintf (stderr, "TArray loop %d\n", loops);

      {
        CHasher hasher = NULL;
        int i;
        
        GCReferenceAssign (hasher, CStrHasherNew (20, NULL));
        
        for (i = 0; i < OBJECTS; i++)
          {
            char* text;
            
            text = (char*) malloc (30);
            my_malloc_count++;
            sprintf (text, "%d", i);

            GCObjectNew (text, StrClass);
            
            CHasherAdd (hasher, text);
          }
        
        CHasherErase (hasher);
        GCReferenceClear (hasher);
      }

      {
        CArray array = NULL;
        B b = NULL;
        int i;
        
        GCReferenceAssign (b, BNew ("b", NULL));
        GCReferenceAssign (array, CArrayNew ());
        
        for (i = 0; i < OBJECTS; i++)
          {
            CArrayAdd (array, ANew ("a", b));
          }
        
        CArrayErase (array);
        
        GCReferenceClear (b);
        GCReferenceClear (array);
      }
      
      {
        CStack stack = NULL;
        B b = NULL;
        int i;
        
        GCReferenceAssign (b, BNew ("b", NULL));
        GCReferenceAssign (stack, CStackNew ());
        
        for (i = 0; i < OBJECTS; i++)
          {
            CStackPush (stack, ANew ("a", b));
          }
        
        CStackErase (stack);
        
        GCReferenceClear (b);
        GCReferenceClear (stack);
      }
      
      a_count = ACount;
      b_count = BCount;

      GCCleanup ();
      GCDumpBase ();

      CArrayCheck ();
  
      fprintf (stderr, "ACount = %3.3d ", a_count);
      fprintf (stderr, "BCount = %3.3d ", b_count);

      fprintf (stderr, "ACount = %3.3d ", ACount);
      fprintf (stderr, "BCount = %3.3d\n", BCount);

      fprintf (stderr, "malloc count = %3.3d\n", my_malloc_count);

      fprintf (stderr, "TArray end loop\n");
    }

  return (0);
}
