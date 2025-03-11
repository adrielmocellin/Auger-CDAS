/************************************************************************
 *
 * FILE:
 *       UiLst.h
 * 
 ***********************************************************************/

#ifndef _UILST_H_
#define _UILST_H_

/*
 * Types.
 */
typedef struct uilst
{
  struct uilst    *prev;
  void            *data;
  struct uilst    *next;
} t_uilst;

#define DELETE 'd'
#define NO_DELETE 'a'

/*
 * Declaration of public functions.
 */
t_uilst *UiLstNew( void * const data_ptr );
t_uilst *UiLstFree( t_uilst * const list_ptr, const char copt );
t_uilst *UiLstInsert( t_uilst * list_ptr, void * const data_ptr );
t_uilst *UiLstAdd( t_uilst * list_ptr, void * const data_ptr );
t_uilst *UiLstDelete( t_uilst * const list_ptr, void * const data_ptr,
		      const char copt );
t_uilst *UiLstDeleteSafe( t_uilst * const list_ptr, void * const data_ptr,
			  const char copt );
t_uilst *UiLstAddList( t_uilst * const list_ptr_1, 
		       t_uilst * const list_ptr_2 );
t_uilst *UiLstInsertList( t_uilst * const list_ptr_1,
			  t_uilst * const list_ptr_2 );
t_uilst *UiLstCopy( t_uilst * const list_ptr );
t_uilst *UiLstFirst( t_uilst * const list_ptr );
t_uilst *UiLstLast( t_uilst * const list_ptr );
t_uilst *UiLstFind( t_uilst * const list_ptr, const void * const data,
		    int (* const cmp)(const void * const, const void * const));
int     uiLstCount( t_uilst * const list_ptr );
t_uilst *UiLstSort0( t_uilst * const c,
		    int (* const cmp)(const void * const, const void * const));
     
#endif /* _UILST_H_ */






