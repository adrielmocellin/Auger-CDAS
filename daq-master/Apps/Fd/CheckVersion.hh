
//
// File   : CheckVersion.hh
//
// Purpose: File to be included by all *.cc files to check the software versions
//
// $Id$
//



/** @file CheckVersion.hh
  * Macro file for software version checking.
  * @author H.-J. Mathes, FzK
  */


#ifndef _CheckVersion_hh_
#define _CheckVersion_hh_

# ifndef NO_VERSION

/* --- Check against version of package FdUtil ... --- */

# include <FdUtilVersion.hh>
# if (FDUTIL_VERSION_CODE < ModuleVersionCode(3,4,0) \
      || FDUTIL_VERSION_CODE > ModuleVersionCode(4,1,255))
#  error "Version of module FdUtil does not match !"
#  error "Required version: 3.4.0 ... 4.1.x"
# endif

# endif // NO_VERSION

#endif  // _CheckVersion_hh_
