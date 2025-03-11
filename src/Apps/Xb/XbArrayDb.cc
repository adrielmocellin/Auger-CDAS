/*! \file
    \brief Deals with the array of stations from Db, for CDAS Xb
*/

#include "DbCArray.h"
#include "XbArray.h"
#include "Xb.h"
#include "IkC.h"
#include <cstdlib>
#include <cstdio>


int nstat=0;
int maxstat=2040;
struct Station* sdarray=NULL;

void XbArrayInit(int ok) {
  if (sdarray!=NULL) free(sdarray);
  struct DbCSimpleStation* array2=DbCSimpleArrayInit();
  nstat=DbCSimpleArrayGetNbStat(array2);
  sdarray=(struct Station *)malloc((1+nstat)*sizeof(struct Station));
  for(int i = 0; i < nstat; i++) {
    sdarray[i].Id       = array2[i].Id;
    sdarray[i].Northing = array2[i].Northing;
    sdarray[i].Easting  = array2[i].Easting;
    sdarray[i].Altitude = array2[i].Altitude;
//    if (array[i].Id>maxstat) maxstat=array[i].Id;
  }
  sdarray[nstat].Id = -1;
  free(array2);

  BuildNeighbour();
  if (ok) {
    stringstream s;
    s << nstat << " stations in array";
    InternalLog(s.str().c_str(),IKINFO);
  }
}
