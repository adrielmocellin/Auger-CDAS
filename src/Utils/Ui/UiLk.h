/************************************************************************
 *
 * FILE:
 *       UiLk.h
 * 
 ***********************************************************************/

#ifndef _UILK_H_
#define _UILK_H_

#include "LkListP.h"

/*
 * Types.
 */

/*
 * Declaration of public functions.
 */


LkEntry UiLkFind( LkList const list_ptr, const void * const data,
		  int (* const cmp)(const void * const, const void * const));

     
#endif /* _UILK_H_ */






