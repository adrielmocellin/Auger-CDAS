#define PARENTCLASS IkPmMessage

class IkT3 {
#include "IkPmInclude.inc" /* where mode = MLIST and addresses = list of stations*/
 unsigned id /* T3 ident */
 unsigned refSecond /* central t3 time, integer part */
 unsigned refuSecond /* fractionnal part in useconds */
 /* WARNING, both offsets and window should be UNSIGNED char */
 vector<int> offsets /* 1 int per station  , time correction */
 vector<int> window /* 1 per station */ 
 float SDPAngle /* shower detector plane from fd's estimation */
 string algo
}
