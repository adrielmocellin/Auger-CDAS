#ifndef DBCARRAY_H
#define DBCARRAY_H

/* New version with domain names */
typedef struct {
  double Northing;
  double Easting;
  double Altitude;
  int Id;
  int Valid;
  char *Name;
  char *Domain;
}
SdTank;

// old version without domain
struct DbCStation {
  double Northing;
  double Easting;
  double Altitude;
  int Id;
  int Valid;
  char *Name;
};

// light version
struct DbCSimpleStation {
  double Northing;
  double Easting;
  double Altitude;
  int Id;
};

//Loads survey data base from traveller
SdTank* MySqlDbLoadArray        ();
SdTank* DbCLoadArray        ();

struct DbCStation* DbCArrayInit();
struct DbCSimpleStation* DbCSimpleArrayInit();

int DbCArrayGetNbStat(struct DbCStation* array);
int DbCSimpleArrayGetNbStat(struct DbCSimpleStation* array);
int DbCSize           (SdTank*);

// Gets GPS Offset from Mdb, using UB if uub==0, UUB if uub=1
int MySqlGetGPSOffset( int cpuid , int uub=0);

// For backward compatibility: when Rc was updating CDAS Db
int DbCCreateEntry(SdTank *);
int DbCUpdateEntry(SdTank *);

// To code again
//SdTank* DbCLoadDomain       (const char*);
//int DbCDomainSize     (const char*);

#endif
