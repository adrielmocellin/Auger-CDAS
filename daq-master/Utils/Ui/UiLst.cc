/************************************************************************
 *
 * FILE:
 *       UiLst.c
 * 
 * DESCRIPTION: 
 *       A doubly linked list management routines.
 *
 ***********************************************************************/

/* standard include files */
#include <stdlib.h>

/* cdas specific include files */
#include "UiLst.h"
#include "UiErr.h"

int uiErrno = CDAS_SUCCESS;

/*-----------------------------------------------------------------------
 * UiLstNew
 *
 * Will create a new list and add the first item.
 *
 * Input : void *, a data element
 * Output: none
 * Return: t_uilst *, pointer to top of list or 0
 */
t_uilst *UiLstNew( void * const data_ptr )
{
  t_uilst *l_first = 0;

  uiErrno = 0;

  l_first = (t_uilst *)malloc( sizeof( t_uilst ) );

  if ( !l_first ) {
    uiErrno = CDAS_MALLOC_ERROR;
    return 0;
  }
  
  l_first->data = data_ptr;
  l_first->prev = 0;
  l_first->next = 0;

  return( l_first );
}	

/*-----------------------------------------------------------------------
 * UiLstFree
 *
 * Will free up all memory for list.
 * If option 'd' is passed the data elements are also freed.
 *
 * Input : t_uilst *, pointer to a list.
 *         char, copt = 'd', will also delete data elements.
 * Output: none
 * Return: t_uilst *, 0
 */
t_uilst *UiLstFree( t_uilst * const list_ptr, const char copt )
{
  t_uilst *l_ptr = 0;
  t_uilst *l_tmp = 0;
  t_uilst *l_first = UiLstFirst( list_ptr );

  uiErrno = 0;

  if ( !l_first ) return 0;

  l_ptr = l_first;
  while( l_ptr ) {
    if ( copt == DELETE && l_ptr->data ) {
      free ( l_ptr->data );
    }
    l_tmp = l_ptr;
    l_ptr = l_ptr->next;
    if ( l_ptr ) l_ptr->prev = 0;
    free( l_tmp );
  }
  return 0;
}

/*-----------------------------------------------------------------------
 * UiLstInsert
 *
 * Will add an item as the first item to the list.
 *
 * Input : t_uilst *, pointer to a list, if 0 a new list will be created.
 *         void *, data element for list.
 * Output: none
 * Return: t_uilst *, pointer to the first item of the list or 0
 *
 * NOTE: if passed list pointer, in successive calls, is the first item,
 * it should be pretty fast.
 */
t_uilst *UiLstInsert( t_uilst * list_ptr, void * const data_ptr )
{
  t_uilst *l_first = 0;
  t_uilst *l_new = 0;

  uiErrno = 0;

  if ( ! list_ptr ) {
    list_ptr = UiLstNew( data_ptr );
    return list_ptr;
  }

  l_first = UiLstFirst( list_ptr );

  l_new = (t_uilst *)malloc( sizeof( t_uilst ) );

  if ( !l_new ) {
    uiErrno = CDAS_MALLOC_ERROR;
    return 0;
  }

  l_new->data = data_ptr;
  
  l_first->prev = l_new;
  l_new->next = l_first;
  l_new->prev = 0;

  return l_new;
}

/*-----------------------------------------------------------------------
 * UiLstAdd
 *
 * Will add an item as the last item to the list.
 *
 * Input : t_uilst *, pointer to a list, if 0 a new list will be created.
 *         void *, data element for list.
 * Output: none
 * Return: t_uilst *, pointer to the last item of the list or 0
 *
 * NOTE: if passed list pointer, in successive calls, is the last item,
 * it should be pretty fast.
 */
t_uilst *UiLstAdd( t_uilst *list_ptr, void * const data_ptr )
{
  t_uilst *l_last = 0;
  t_uilst *l_new = 0;

  uiErrno = 0;

  if ( ! list_ptr ) {
    list_ptr = UiLstNew( data_ptr );
    return list_ptr;
  }
  
  l_last = UiLstLast( list_ptr );

  l_new = (t_uilst *)malloc( sizeof( t_uilst ) );

  if ( !l_new ) {
    uiErrno = CDAS_MALLOC_ERROR;
    return 0;
  }

  l_new->data = data_ptr;
  
  l_last->next = l_new;
  l_new->prev = l_last;
  l_new->next = 0;

  return l_new;
}

/*-----------------------------------------------------------------------
 * UiLstDelete
 *
 * Will delete item from list.
 * If option 'd' is passed the data element is also freed.
 *
 * Input : t_uilst *, pointer to a list
 *         void *, data element of item to be deleted.
 *         char, copt = 'd', will also delete data elements.
 * Output: none
 * Return: t_uilst *, pointer to top of list
 */
t_uilst *UiLstDelete( t_uilst * const list_ptr, void * const data_ptr, 
		      const char copt )
{
  t_uilst *l_ptr = 0;
  t_uilst *l_prev = 0;
  t_uilst *l_first = 0;

  uiErrno = 0;

  l_first = UiLstFirst( list_ptr );
  
  for ( l_ptr = l_first; l_ptr; l_ptr = l_ptr->next ) {
    l_prev = l_ptr->prev;

    if ( l_ptr->data == data_ptr ) {
      if ( l_ptr->next ) {
	l_ptr->next->prev = l_prev;
	if ( l_prev ) l_prev->next = l_ptr->next;
      }
      else {
	if ( l_prev ) l_prev->next = 0;
      }
      if ( copt == DELETE && l_ptr->data ) {
	free( l_ptr->data );
      }
      if ( l_ptr == l_first ) {
	l_first = l_ptr->next;
      }
      free( l_ptr );
      return l_first;
    }
  }

  /* item was not found */
  uiErrno = CDAS_NOT_FOUND;
  return 0;
}

/*-----------------------------------------------------------------------
 * UiLstDeleteSafe
 *
 * Will delete item from list. And return next item in list.
 * (it's safe to use in a loop there is traversing a list).
 * If option 'd' is passed the data element are also freed.
 *
 * Input : t_uilst *, pointer to a list
 *         void *, data element of item to be deleted.
 *         char, copt = 'd', will also delete data elements.
 * Output: none
 * Return: t_uilst *, pointer to next item in list
 */
t_uilst *UiLstDeleteSafe( t_uilst * const list_ptr, void * const data_ptr,
			  const char copt )
{
  t_uilst *l_ptr = 0;
  t_uilst *l_prev = 0, *l_next = 0;

  uiErrno = 0;

  for ( l_ptr = list_ptr; l_ptr; l_ptr = l_ptr->next ) {
    l_prev = l_ptr->prev;

    if ( l_ptr->data == data_ptr ) {
      if ( l_ptr->next ) {
	l_ptr->next->prev = l_prev;
	if ( l_prev ) l_prev->next = l_ptr->next;
      }
      else {
	if ( l_prev ) l_prev->next = 0;
      }
      if ( copt == DELETE && l_ptr->data ) {
	free( l_ptr->data );
      }
      l_next = l_ptr->next;
      free( l_ptr );
      return l_next;
    }
  }

  /* item was not found */
  uiErrno = CDAS_NOT_FOUND;
  return 0;
}

/*-----------------------------------------------------------------------
 * UiLstAddList
 *
 * Will append second list to the end of first list.
 * If passed first list is null, it will return first list element of
 * second list.
 * If passed second list is null, it will return first list item of
 * first list.
 *
 * Input : t_uilst *, pointer to first list.
 *         t_uilst *, pointer to second list to be appended.
 * Output: none
 * Return: t_uilst *, pointer to the first item of list or 0
 */
t_uilst *UiLstAddList( t_uilst * const list_ptr_1, t_uilst * const list_ptr_2 )
{
  t_uilst *l_last_1 = 0;
  t_uilst *l_first_2 = 0;

  uiErrno = 0;

  if ( !list_ptr_1 )
    return UiLstFirst( list_ptr_2 );
  else if ( !list_ptr_2 ) 
    return UiLstFirst( list_ptr_1 );
  
  l_last_1 = UiLstLast( list_ptr_1 );
  l_first_2 = UiLstFirst( list_ptr_2 );

  l_last_1->next = l_first_2;
  l_first_2->prev = l_last_1;
  
  return UiLstFirst( list_ptr_1 );
}

/*-----------------------------------------------------------------------
 * UiLstInsertList
 *
 * Will append second list to passed list element.
 * If passed first list is null, it will return last list element of
 * second list.
 * If passed second list is null, it will return passed first list element.
 *
 * Input : t_uilst *, pointer to item where second list will be appended.
 *         t_uilst *, pointer to second list to be appended.
 * Output: none
 * Return: t_uilst *, pointer to last element of second list or 0.
 */
t_uilst *UiLstInsertList( t_uilst * const list_ptr_1,
			  t_uilst * const list_ptr_2 )
{
  t_uilst *l_next_1 = 0;
  t_uilst *l_first_2 = 0;
  t_uilst *l_last_2 = 0;

  uiErrno = 0;

  if ( !list_ptr_1 )
    return UiLstLast( list_ptr_2 );
  else if ( !list_ptr_2 )
    return list_ptr_1;
  
  l_next_1 = list_ptr_1->next;
  l_first_2 = UiLstFirst( list_ptr_2 );
  l_last_2 = UiLstLast( list_ptr_2 );
    
  list_ptr_1->next = l_first_2;
  l_first_2->prev = list_ptr_1;
    
  l_last_2->next = l_next_1;
  if ( l_next_1 ) l_next_1->prev = l_last_2;
  
  return l_last_2;
}

/*-----------------------------------------------------------------------
 * UiLstCopy
 *
 * Will create a new list, containing data elements from the passed list.
 *
 * Input : t_uilst *, pointer to a list.
 * Output: none
 * Return: t_uilst *, pointer to the first item of the list or 0
 */
t_uilst *UiLstCopy( t_uilst * const list_ptr )
{
  t_uilst *l_ptr = 0;
  t_uilst *l_new = 0;
  
  uiErrno = 0;

  if ( !list_ptr )
    return 0;
  
  l_ptr = UiLstFirst( list_ptr );
  for ( ; l_ptr; l_ptr = l_ptr->next ) {
    l_new = UiLstAdd( l_new, l_ptr->data );
  }

  return UiLstFirst( l_new );
}

/*-----------------------------------------------------------------------
 * UiLstFirst
 *
 * Will return first item of passed list.
 *
 * Input : t_uilst *, pointer to a list
 * Output: none
 * Return: t_uilst *, pointer to first item of list
 */
t_uilst *UiLstFirst( t_uilst * const list_ptr )
{
  t_uilst *l_ptr = 0;
  
  uiErrno = 0;

  if ( !list_ptr ) return 0;

  for ( l_ptr = list_ptr; l_ptr->prev; l_ptr = l_ptr->prev ) {}

  return l_ptr;
}

/*-----------------------------------------------------------------------
 * UiLstLast
 *
 * Will return last item of passed list.
 *
 * Input : t_uilst *, pointer to a list
 * Output: none
 * Return: t_uilst *, pointer to last item of list
 */
t_uilst *UiLstLast( t_uilst * const list_ptr )
{
  t_uilst *l_ptr = 0;
  
  uiErrno = 0;

  if ( !list_ptr ) return 0;

  for ( l_ptr = list_ptr; l_ptr->next; l_ptr = l_ptr->next ) {}

  return l_ptr;
}

/*-----------------------------------------------------------------------
 * UiLstCount
 *
 * Will return number of items in a list.
 *
 * Input : t_uilst *, pointer to a list
 * Output: none
 * Return: int, number of items in a list
 */
int UiLstCount( t_uilst * const list_ptr )
{
  int count = 0;
  t_uilst *l_ptr = UiLstFirst( list_ptr );
  
  uiErrno = 0;

  for ( ; l_ptr; l_ptr = l_ptr->next ) { count++; }

  return count;
}

/*-----------------------------------------------------------------------
 * UiLstFind
 *
 * Find a list element.
 *
 * Input : t_uilst *, pointer to a list
 *         void *, pointer to the data element to find
 *         int *, pointer to a function to be called for
 *                comparing list items.
 * Output: none
 * Return: t_uilst *, pointer to found list element or 0.
 */
t_uilst *UiLstFind( t_uilst * const list_ptr, const void * const data,
		    int (* const cmp)(const void * const, const void * const) )
{
  t_uilst *l_ptr = UiLstFirst( list_ptr );

  uiErrno = 0;

  for ( ; l_ptr; l_ptr = l_ptr->next ) {
    if ( !cmp( data, l_ptr->data ) )
      return l_ptr;	 
  }
  
  return 0;
}

/*-----------------------------------------------------------------------
 * UiLstSort0
 *
 * Sorting a list. This is just a insertion sort. We should implement
 * something better if we need to sort large (>20 items) lists.
 *
 * Input : t_uilst *, pointer to a list
 *         int *, pointer to a function to be called for comparing list items.
 * Output: none
 * Return: t_uilst *, pointer to first element in list.
 */
t_uilst *UiLstSort0( t_uilst * const list_ptr,
		     int (* const cmp)(const void * const, const void * const))
{
  t_uilst *l1 = UiLstFirst( list_ptr ), *l2 = 0;
  void *data;

  uiErrno = 0;

  if ( !l1 || !l1->next )
    return l1;
  
  for ( l1 = l1->next; l1; l1 = l1->next ) {
    data = l1->data;
    l2 = l1;
    while ( l2->prev && cmp( l2->prev->data, data ) > 0 ) {
      l2->data = l2->prev->data;
      l2 = l2->prev;
    }
    l2->data = data;
  }

  return UiLstFirst( l2 );
}
