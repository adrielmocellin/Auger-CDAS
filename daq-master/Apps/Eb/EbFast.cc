#include "Eb.h"
#include "EbEvent.h"
#include "EbManager.h"
#include "EbMain.h"

int main (int argc, char ** argv) 
{
  // Set where Data is written
  TheEbManager () -> DataFilePath="/home/cdas/EbFast/";
  // Don't allow retries for lost data, this is EbFast
  TheEbManager () -> MaxNumberOfTries=0;
  // Don't uses real IDs for events, this is EbFast
  TheEbManager () -> UseRealIDs=0;
  // New data file every hour;
  TheEbManager () -> MaxTimeFileOpen=3600;

  // Run it
  EbMain(argc, argv);
  return 0;
}
