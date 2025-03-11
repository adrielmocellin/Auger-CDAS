#ifndef __CArray_ph__
#define __CArray_ph__

#ifdef __cplusplus
extern "C" {
#endif

void CArrayStartup  ();
void CArrayCleanup  ();
void CArrayCheck  ();
CArray CArrayNew  ();
void CArrayErase  (CArray This);
int CArrayCount  (CArray This);
void CArrayAdd  (CArray This, void* object);
void CArrayRemove  (CArray This, void* object);
void* CArrayItem  (CArray This, CIterator it);
void CArrayRemoveAt  (CArray This, int index);
void* CArrayItemAt  (CArray This, int index);
void* CArrayFind  (CArray This, void* object);
CIterator CArrayGiveIterator  (CArray This);
CIterator CArrayGiveBackwardIterator  (CArray This);
CStack CStackNew  ();
void CStackErase  (CStack This);
int CStackCount  (CStack This);
void CStackPush  (CStack This, void* object);
void* CStackTop  (CStack This);
void* CStackPop  (CStack This);
CQueue CQueueNew  ();
void CQueueErase  (CQueue This);
int CQueueCount  (CQueue This);
void CQueuePush  (CQueue This, void* object);
void* CQueueTop  (CQueue This);
void* CQueuePop  (CQueue This);
CLList CLListNew  ();
void CLListErase  (CLList This);
int CLListCount  (CLList This);
void CLListAdd  (CLList This, void* object);
void CLListPrepend  (CLList This, void* object);
void* CLListFirst  (CLList This);
void* CLListLast  (CLList This);
void CLListRemove  (CLList This, void* object);
void* CLListFind  (CLList This, void* object);
CIterator CLListGiveIterator  (CLList This);
CIterator CLListGiveBackwardIterator  (CLList This);
CHasher CPtrHasherNew  (int size, CValuator valuator);
CHasher CStrHasherNew  (int size, CValuator valuator);
CHasher CLStrHasherNew  (int size, CValuator valuator);
CHasher CHasherNew  (int size,                    CValuator valuator,                    CCoder coder,                    CTester tester);
void CHasherErase  (CHasher This);
void CHasherAdd  (CHasher This, void* object);
void* CHasherFind  (CHasher This, void* value);
int CHasherFindPosition  (CHasher This, void* value,                         int* code, int* index);
void CHasherRemove  (CHasher This, void* object);
CIterator CHasherGiveIterator  (CHasher This);
int CIteratorFinished  (CIterator This);
void CIteratorFirst  (CIterator This);
void CIteratorForth  (CIterator This);
void CIteratorDelete  (CIterator This);
void* CIteratorItem  (CIterator This);
void CIteratorStop  (CIterator This);

#ifdef __cplusplus
}
#endif

#endif

