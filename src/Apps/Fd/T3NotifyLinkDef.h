//
// File   : T3NotifyLinkDef.h
//
// Purpose: CINT LinkDef.h file T3Notify (CDAS interface)
//
// $Id: T3NotifyLinkDef.h,v 1.2 2005/03/18 19:44:46 bertou Exp $
//

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

//
// options:
// + : generate Streamer and StreamerInfo (new format of version 3)
//   : generate Streamer and StreamerInfo
// - : don't generate Streamer -> user defined Streamer
//

//#pragma link C++ class T3Notify; // v1, v2
#pragma link C++ class T3Notify-; // v3++
//#pragma link C++ class T3NotifyStat-;

#endif
