#include "Eb.h"
#include "EbEvent.h"
#include "EbManager.h"
#include "EbMain.h"

int main (int argc, char ** argv) 
{
  // Set where Data is written
  TheEbManager () -> DataFilePath="/Raid/data/Sd/";
  // Allow retries for lost data
  TheEbManager () -> MaxNumberOfTries=1;
  // Official Eb, uses real IDs for events
  TheEbManager () -> UseRealIDs=1;
  // New data file every 12h;
  TheEbManager () -> MaxTimeFileOpen=12*3600;

  // Run it
  EbMain(argc, argv);
  return 0;
}
