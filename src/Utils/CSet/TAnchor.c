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

#include <GC.h>

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

/*-----------------------------------------------------*/
int main (int argc, char* argv[])
/*-----------------------------------------------------*/
{
  fprintf (stderr, "Creating class A\n");
  AClass = GCClassNew ("A", (GCDeleteMethod) ADelete);
  
  fprintf (stderr, "Creating class B\n");
  BClass = GCClassNew ("B", (GCDeleteMethod) BDelete);

  for (;;)
    {
      {
        A a;
        
        GCReferenceAssign (a, ANew ("a1", NULL));
        GCReferenceClear (a);
      }
 
      {
        B b;
        
        GCReferenceAssign (b, BNew ("b1", NULL));
        GCReferenceClear (b);
      }
      
      printf ("ACount = %d\n", ACount);
      printf ("BCount = %d\n", BCount);
      
      {
        A a;
        B b;
        
        GCReferenceAssign (a, ANew ("a1", NULL));
        GCReferenceAssign (b, BNew ("b1", a));
        
        ASetB (a, b);
        
        GCReferenceClear (a);
        GCReferenceClear (b);
      }
      
      printf ("ACount = %d\n", ACount);
      printf ("BCount = %d\n", BCount);
      
      {
        char line[256];
        gets (line);
        if (line[0] == 'c')
          {
            GCCleanup ();

            GCDumpBase ();
  
            printf ("ACount = %d\n", ACount);
            printf ("BCount = %d\n", BCount);
          }
        else if (line[0] == 'q')
          {
            break;
          }
      }
    }
  GCCleanup ();
  GCDumpBase ();  
  {
    char line[256];
    gets (line);
  }
  return (0);
}
