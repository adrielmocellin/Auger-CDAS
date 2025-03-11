/*! \file
    \brief Deals with the array of stations from simulation, for test purposes
*/

#include "XbArray.h"
#include "Xb.h"
#include <cstdio>
#include <cstdlib>

int nstat=0;
int maxstat=0;
struct Station* sdarray=NULL;

void XbArrayInit(int ok) {
  if (sdarray!=NULL) free(sdarray);
  nstat=1600;
  sdarray=(struct Station *)malloc((1+nstat)*sizeof(struct Station));
  for(int i = 0; i < 40; i++) {
    for(int j = 0; j < 40; j++) {
      sdarray[i*40+j].Id       = i*40+j+200;   // starting at 200 so that DOUBLETS are not in array, no FAKE...
      sdarray[i*40+j].Northing = j*1500*sqrt(3.)/2.;
      sdarray[i*40+j].Easting  = i*1500+(j%2)*750;
      sdarray[i*40+j].Altitude = 0;
      if (sdarray[i*40+j].Id>maxstat) maxstat=sdarray[i*40+j].Id;
    }
  }
  sdarray[nstat].Id = -1;

  BuildNeighbour();
  if (ok) for (int i=0;i<1600;i++) cout << sdarray[i].Id << " " << sdarray[i].Easting << " " << sdarray[i].Northing << endl;
  if (ok) cerr << nstat << " stations in array" << endl;
}
