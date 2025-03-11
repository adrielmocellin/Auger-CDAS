#ifndef __GC_ph__
#define __GC_ph__

#ifdef __cplusplus
extern "C" {
#endif

void GCStartup  ();
void GCReferenceCleanup  ();
GCClass GCClassNew  (char* name, GCDeleteMethod erase);
void GCClassDelete  (GCClass This);
GCObject GCObjectNew  (void* address, GCClass base);
GCReference GCReferenceSet  (void** address, const void* from, const void* to);
void GCReferenceMove  (void** address, void** newAddress);
void GCDumpBase  ();
void GCCleanup  ();

#ifdef __cplusplus
}
#endif

#endif

