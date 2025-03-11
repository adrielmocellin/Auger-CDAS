#include <DbCArray.h>
#include <IkC.h>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
using namespace std;

int main()
{
	int i = 0;
	CDASInit("TdbTest");
	SdTank *array = MySqlDbLoadArray();
	if (array)
	{
	cout << "\n\n *****************  Traveler Data Base (Mdb) content  *********" << endl;
     cout << "|======================================================================================|\n";
		  printf("| %-4s | %-20s | %-15s | %-10s | %-10s | %-10s | %-10s\n",
				  "Id","Name","Domain","Northing","Easting","Altitude","Ekit");
     cout << "|======================================================================================|\n";
	  while (array[i].Id != -1) 
	  {
		  if (i)
     cout << "|--------------------------------------------------------------------------------------|\n";
		  printf("| %-4d | %-20s | %-15s | %-10.2f | %-10.2f | %-10.2f | %-10d \n",
				array[i].Id,array[i].Name,array[i].Domain,
				array[i].Northing,array[i].Easting,array[i].Altitude,array[i].Valid) ;
	        ++i;
	  }
     cout << "|======================================================================================|\n";

	}
	else cout << "Mdb could not be loaded !" << endl;
}

