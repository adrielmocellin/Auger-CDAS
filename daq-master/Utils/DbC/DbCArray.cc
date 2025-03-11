#include <dirent.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <math.h>

#include "DbCArray.h"

#include <cstring>
#include <vector>
using namespace std;

#define NO_CLIENT_LONG_LONG
#include "mysql/mysql.h"
#include <stdarg.h>

// Main entry points .........

int DbCSize(SdTank *array) {
  int size=0;
  while(array[size].Id != -1) {
    size++;
  }
  return size;
}

SdTank* DbCLoadArray        () {
  return MySqlDbLoadArray();
}


// --------------------------------------------------
// Former entry point kept for backward compatibility

int DbCArrayGetNbStat(struct DbCStation * array) {
  int size=0;
  while(array[size].Id != -1) {
    size++;
  }
  return size;
}

int DbCSimpleArrayGetNbStat(struct DbCSimpleStation * array) {
  int size=0;
  while(array[size].Id != -1) {
    size++;
  }
  return size;
}

struct DbCStation * DbCArrayInit() {
  struct DbCStation *array;
  SdTank *tanks = DbCLoadArray();
  int nstat=DbCSize(tanks);
  array=(struct DbCStation *)malloc((1+nstat)*sizeof(struct DbCStation));
  for(int i = 0; i < nstat; i++) {
    array[i].Id       = tanks[i].Id;
    array[i].Northing = tanks[i].Northing;
    array[i].Easting  = tanks[i].Easting;
    array[i].Altitude = tanks[i].Altitude;
    array[i].Valid    = tanks[i].Valid;
    if (tanks[i].Name) array[i].Name     = strdup(tanks[i].Name);
    else array[i].Name="";
  }
  array[nstat].Id = -1;
  free(tanks);
  return array;
}

struct DbCSimpleStation * DbCSimpleArrayInit() {
  struct DbCSimpleStation *array;
  SdTank *tanks = DbCLoadArray();
  int    nstat = DbCSize(tanks);
  int    j = 0;

  array = (struct DbCSimpleStation *)malloc((1+nstat)*sizeof(struct DbCSimpleStation));
  for(int i = 0; i < nstat; i++) {
    if (tanks[i].Valid > 0 && tanks[i].Altitude>1000.) {
      array[j].Id       = tanks[i].Id;
      array[j].Northing = tanks[i].Northing;
      array[j].Easting  = tanks[i].Easting;
      array[j].Altitude = tanks[i].Altitude;
      j++;
    }
  }
  array[j].Id = -1;
  free(tanks);
  return array;
}

MYSQL *Connect() {
  MYSQL *traveller;
  traveller = mysql_init(NULL);
  if (!mysql_real_connect(traveller, "127.0.0.1", "mocca", "sibyll", "PMS", 0, NULL, 0)) {
    //IkWarningSend("Cannot connect to traveler database. Reason : %s", mysql_error(traveller));
    cerr << "Cannot connect to traveler database. Reason: " << mysql_error(traveller) << endl;
    mysql_close(traveller);
    return NULL;
  }
  return traveller;
}


MYSQL_RES * Query(MYSQL *db, const char *query) {
  MYSQL_RES *results;

  if (mysql_query(db, query) != 0) {
    //IkWarningSend("Traveler Query failed . Reason : %s", mysql_error(db));
    cerr << "Traveler Query failed. Reason: " << mysql_error(db) << endl;
    mysql_close(db);
    return NULL;
  }
  if (!(results = mysql_store_result(db))) {
    //IkWarningSend("Mysql store result failed . Reason : %s", mysql_error(db));
    cerr << "Mysql store result failed. Reason: " << mysql_error(db) << endl;
    mysql_close(db);
    return NULL;
  } else return results;
}

SdTank *Store(MYSQL_RES *toStore) {
  MYSQL_ROW row;
  int nbRows, i;
  SdTank *array=NULL;

  nbRows = mysql_num_rows(toStore);
  array = (SdTank *) malloc((nbRows + 1)*sizeof(SdTank));
  i = 0;
  while ((row = mysql_fetch_row(toStore))) {
    array[i].Id       = (*row ? atoi((char*)*row) : 0);           ++row;
    array[i].Name     = (*row ? strdup((char*)*row) : NULL);      ++row;
    array[i].Domain   = (*row ? strdup((char*)*row) : NULL);      ++row;
    array[i].Easting  = lrint((*row ? strtod((char*)*row, NULL) : 0.0)*100)/100.0; ++row;
    array[i].Northing = lrint((*row ? strtod((char*)*row, NULL) : 0.0)*100)/100.0; ++row;
    array[i].Altitude = lrint((*row ? strtod((char*)*row, NULL) : 0.0)*100)/100.0; ++row;
    array[i].Valid    = (*row ? atoi((char*)*row) : 0);           ++i;
  }
  array[i].Id = -1;
  mysql_free_result(toStore);
  return array;
}


SdTank *MySqlDbLoadArray() {
  SdTank *array;
  MYSQL *Mdb;
  MYSQL_RES *result;

  Mdb = Connect();
  if (!Mdb) return NULL;
  result = Query(Mdb, "SELECT Detector.ID, TankName.Name, DomainName.Name AS Domain, \
                 Detector.Easting, Detector.Northing, Detector.Altitude, UB.Name FROM Detector \
  LEFT JOIN ElectronicsKit on ElectronicsKit.ID=Detector.Electronics LEFT JOIN UB on UB.ID=ElectronicsKit.UB \
                 LEFT JOIN TankName ON TankName.ID = Detector.Name LEFT JOIN Location \
                 ON  Location.ID = Detector.Location LEFT JOIN DomainName  ON \
                 DomainName.ID = Location.DomainName WHERE Detector.Id>10 ORDER BY Detector.ID"); 
  //it had a limitation of Detector.Id>70. Because of the additional stations of AERA,
  // it needed to include the Id>10 instead (2012/Dec/05).
  // Result of Query is per station : Id Name Domain Easting Northing Altitude
  // Store function below assumes this order !
  array = Store(result);
  mysql_close(Mdb);
  return array;
}

int MySqlGetGPSOffset( int cpuid ,int uub) {
  MYSQL *Mdb;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int offset;

  Mdb = Connect();
  if (!Mdb) return 0;
  char query[256];
  char uubquery='<';
  if (uub) uubquery='>';
  snprintf(query,256,"SELECT Offset FROM GPS LEFT JOIN ElectronicsKit ON ElectronicsKit.GPS = GPS.ID LEFT JOIN UB ON UB.ID = ElectronicsKit.UB WHERE UB.Name = %d AND UB.Type %c 3;",cpuid,uubquery);
  result = Query(Mdb, query);
  int nbRows = mysql_num_rows(result);
  if (nbRows!=1) {
    //IkWarningSend("Warning, found %d answers for GPS attached to UB %d\n",nbRows,cpuid);
    cerr << "Warning, found " << nbRows << " answers for GPS attached to UB " << cpuid << endl;
    mysql_close(Mdb);
    return 0;
  }
  row = mysql_fetch_row(result);
  offset=int((*row ? strtod((char*)*row, NULL) : 0.0)*100);
  mysql_free_result(result);
  mysql_close(Mdb);
  return (offset);
}


                                                                                
// For backward compatibility: when Rc was updating CDAS Db
int DbCCreateEntry(SdTank *) {
  return 1;
}
int DbCUpdateEntry(SdTank *) {
  return 1;
}
