#ifndef __TArray_ph__
#define __TArray_ph__

#ifdef __cplusplus
extern "C" {
#endif

A ANew  (char* name, B b);
void ADelete  (A This);
void ASetB  (A This, B b);
B BNew  (char* name, A a);
void BDelete  (B This);
void BSetA  (B This, A a);
void my_free  (void* ptr);
int main  (int argc, char* argv[]);

#ifdef __cplusplus
}
#endif

#endif

