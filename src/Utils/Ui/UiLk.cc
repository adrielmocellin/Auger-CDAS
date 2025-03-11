/************************************************************************
 *
 * FILE:
 *       UiLk.c
 * 
 * DESCRIPTION: 
 *       Additions to the linked list management routines in CSet
 *
 ***********************************************************************/

/* standard include files */
#include <stdlib.h>

/* cdas specific include files */
#include "LkListP.h"

/*-----------------------------------------------------------------------
 * UiLkFind
 *
 * Find the list entry that matches according to the passed function
 *
 * Input : LkList, pointer to a list
 *         void *, pointer to the data element(s) to match
 *         int *, pointer to a function to be called for
 *                comparing list entries.
 * Output: none
 * Return: LkEntry, pointer to found list entry or 0.
 */
LkEntry UiLkFind(LkList const list_ptr, const void * const data,
		 int (* const cmp)(const void * const, const void * const))
{
  LkIterator iterator = LkIteratorNew(list_ptr);
  LkEntry entry;

  while ((entry = LkIteratorNext(iterator)) != NULL) {
    if (!cmp( data, entry )) {
      LkIteratorDelete(iterator);
      return entry;
    }
  }
  
  LkIteratorDelete(iterator);
  return NULL;
}
