#include <Rc.h>
#include <DbC.h>
#include <math.h>
#include <cstdlib>
#include <cstdio>

int RunControl::SurveyIndex(int id)
{
if (id > 0 && id < MAXID) return _Db.SurveyIndex[id];
return 0;
}

int RunControl::DbLoad()
{
   SdTank *cdas = NULL;

   if (time(NULL) - _Db.lastLoad < 120) return 1;
   if ((cdas = DbCLoadArray())) {
     int i;
     i = 0; 
     memset(_Db.SurveyIndex, 0 , MAXID * sizeof(int));
    while (cdas[i].Id != -1) {
       _Db.Survey[i] = cdas[i];
       _Db.SurveyIndex[cdas[i].Id] = i;
       ++i;
     }
     _Db.Survey[i] = cdas[i];
     _Db.nSurvey = i;
     _Db.lastLoad = time(NULL);
     if (cdas) free(cdas);
     return 1;
     }
   else return 0;
}

SdTank* RunControl::SurveyDb(int reload)
{
	if (reload) DbLoad();
	        return _Db.Survey;
}


SdTank *RunControl::TankNextBestMatch(SdTank *ref, SdTank *match, SdTank *List)
{double dmin = 10000, dmin2 = 10000, d; SdTank *best = NULL; int i = 0;
	
	if (!match) return RunControl::TankBestMatch(ref, List);
	dmin2 = TankDistance(ref, match);
	while (List[i].Id != -1)
	{
	  if ( (((d = TankDistance(ref, &List[i])) < dmin) && List[i].Id>10) && (List[i].Id != match->Id) ) 
	    {dmin = d; best = &List[i];}
	  ++i;
	}
	return best;
}


SdTank *RunControl::TankBestMatch(SdTank *ref, SdTank *List)
{double dmin = 10000, d; SdTank *best = NULL; int i = 0;
	while (List[i].Id != -1)
	{
	  if (((d = TankDistance(ref, &List[i]))) < dmin && List[i].Id>10) {dmin = d; best = &List[i];}
	  ++i;
	}
	return best;
}

SdTank *RunControl::TankBestGroundMatch(SdTank *ref, SdTank *List)
{double dmin = 10000, d; SdTank *best = NULL; int i = 0;
	while (List[i].Id != -1)
	{
	  if (((d = TankGroundDistance(ref, &List[i]))) < dmin && List[i].Id>10) {dmin = d; best = &List[i];}
	  ++i;
	}
	return best;
}

double RunControl::TankGroundDistance(SdTank *t1, SdTank *t2)
{ // distance is computed in meters
	return sqrt((t1->Northing - t2->Northing)*(t1->Northing - t2->Northing) +
		    (t1->Easting  - t2->Easting )*(t1->Easting  - t2->Easting));
}

double RunControl::TankDistance(SdTank *t1, SdTank *t2)
{ // distance is computed in meters
	return sqrt((t1->Northing - t2->Northing)*(t1->Northing - t2->Northing) +
		    (t1->Easting  - t2->Easting )*(t1->Easting  - t2->Easting)  +
		    (t1->Altitude - t2->Altitude)*(t1->Altitude - t2->Altitude));
}

bool  RunControl::XyzMatch(SdTank *t1, SdTank *t2)
{ int  x1 = lrint(t1->Northing * 100.0), x2 = lrint(t2->Northing * 100.0),
       y1 = lrint(t1->Easting  * 100.0), y2 = lrint(t2->Easting  * 100.0),
       z1 = lrint(t1->Altitude * 100.0), z2 = lrint(t2->Altitude * 100.0);  
double d =  sqrt( (double)((x1-x2)*(x1-x2) + (y1 - y2)*(y1 - y2) + (z1 - z2)*(z1 - z2)));
	return ( d < 10.0 ); // cm! 
}
