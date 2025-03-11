#ifndef __Hash_ph__
#define __Hash_ph__

#ifdef __cplusplus
extern "C" {
#endif

void HTableCleanup  ();
HTable HTableNew  (int size, HNamer namer);
void HTableDelete  (HTable This);
void HTableClear  (HTable This, HFunction clear);
void HTableProcess  (HTable This, HFunction action);
int HTableEncode  (HTable This, char* name);
int HTablePosition  (HTable This, char* name);
int HTablePositions  (HTable This, int code);
HReference HTableFindReference  (HTable This, char* name);
HReference HTableGetReference  (HTable This, int code, int position);
void HTableAddReference  (HTable This, HReference reference);
void HTableRemoveReference  (HTable This, HReference reference);
HIterator HIteratorNew  (HTable table);
void HIteratorDelete  (HIterator This);
void HIteratorReset  (HIterator This);
HReference HIteratorNext  (HIterator This);
HReference HIteratorPrevious  (HIterator This);

#ifdef __cplusplus
}
#endif

#endif

