#ifndef __RegLex_ph__
#define __RegLex_ph__

#ifdef __cplusplus
extern "C" {
#endif

void IncLevel  ();
void DecLevel  ();
void IncPosition  ();
void ResetPosition  ();
void RegLexStartup  (char* text);
int RegLexInput  ();

#ifdef __cplusplus
}
#endif

#endif

