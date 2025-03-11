#include <DbC.h>
#include <DbCArray.h>

int main() {
    DbCStation *gDBStations=DbCArrayInit();
  return DbCArrayGetNbStat(gDBStations);
}
