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
#ifndef __GC_h__
#define __GC_h__

typedef struct _GCReferenceRec*  GCReference;
typedef struct _GCClassRec*      GCClass;
typedef struct _GCObjectRec*     GCObject;

typedef void  (*GCDeleteMethod) (void*);

#include <GC.ph>

#ifndef __CI_SPECIFIC__

#define GCReferenceAssignIn(loc,container,object) GCReferenceSet (\
(void**) &loc, \
container, \
object)

#define GCReferenceAssign(loc,object) GCReferenceSet (\
(void**) &loc, \
NULL, \
object)

#define GCReferenceClear(loc) GCReferenceSet ((void**) &loc, NULL, NULL)

#endif

#endif
