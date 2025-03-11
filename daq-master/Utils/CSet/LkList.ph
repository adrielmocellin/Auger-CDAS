#ifndef __LkList_ph__
#define __LkList_ph__

#ifdef __cplusplus
extern "C" {
#endif

void LkListCleanup  ();
LkList LkListNew  ();
void LkListDelete  (LkList This);
void LkListInit  (LkList This);
void LkListConnectEntry  (LkList This, LkEntry entry);
void LkListTransferEntries  (LkList This, LkList dest);
void LkListClear  (LkList This, LkFunction function);
void LkListProcessForward  (LkList This, LkFunction function);
void LkListProcessBackward  (LkList This, LkFunction function);
void LkListSort  (LkList This, LkFunction function);
int LkListContains  (LkList This, LkEntry entry);
int LkListCount  (LkList This);
LkEntry LkListFirstEntry  (LkList This);
LkEntry LkListLastEntry  (LkList This);
LkEntry LkEntryNew  (LkList list, int size);
LkEntry LkEntryInsertNew  (LkEntry before, LkEntry after, int size);
void LkEntryInit  (LkEntry This, LkList list);
void LkEntryDelete  (LkEntry This);
void LkEntryMove  (LkEntry This, LkEntry before, LkEntry after);
void LkEntryDisconnect  (LkEntry This);
void LkEntryTransfer  (LkEntry This, LkList destList, LkEntry before,  LkEntry after);
int LkEntryPosition  (LkEntry This);
LkEntry LkEntryPrevious  (LkEntry This);
LkEntry LkEntryNext  (LkEntry This);
LkList LkEntryFather  (LkEntry This);
LkIterator LkIteratorNew  (LkList list);
void LkIteratorDelete  (LkIterator This);
void LkIteratorReset  (LkIterator This);
LkEntry LkIteratorNext  (LkIterator This);
LkEntry LkIteratorPrevious  (LkIterator This);

#ifdef __cplusplus
}
#endif

#endif

