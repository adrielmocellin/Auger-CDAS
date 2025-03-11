#include <iostream>
#include "IoSd.h"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
// // command line for all files of 1, 2 and 3 may as input:
// // [shell]> ./analyse.exe $(DATAPATH)/2002/05/eb_2002_05_0[123]*.root
// // command line for all inputs:
// // [shell]> ./analyse.exe $(DATAPATH)/*/*/*.root
// /**************************************************************************/


int main(int argc, char *argv[]) {
  if (argc == 1) {
    cout << "Usage: " << argv[0] << " <files>" << endl;
    
    cout <<
    "   will produce a file named out5.root containing only multiplicity > 5  events."
    << endl;
    
    cout <<
    "   and a file named out10.root containing only multiplicity > 10  events."
    << endl;
    return 1;
  }

  IoSd input(argc - 1, argv + 1);
  IoSd out10("out10.root", "w");
  IoSd out5("out5.root", "w");

  EventPos pos; int error = 0, nevt = 0;
  for (pos = input.FirstEvent(); pos < input.LastEvent();
       pos = input.NextEvent()) {
    IoSdEvent event(pos);
    if(event.AugerId!=200915801668)continue;
    for (unsigned int i = 0; i < event.Stations.size(); i++) {
      // cout<<event.Stations[i].Id<<" "<<event.Stations[i].T2Life<<endl;
	cout<<event.Stations[i].Id<<" "<<event.Stations[i].T2Life<<" "<<event.Stations[i].T2Life120<<endl;
  }
    if (event.NumberOfErrorZeroStation > 10)
      out10.Write(event);
    if (event.NumberOfErrorZeroStation > 5)
      out5.Write(event);
    for (unsigned int i = 0 ; i < event.Stations.size(); i++) if ( event.Stations[i].Trigger.Window == 0 && event.Stations[i].Error == IoSdEvent::eDataLost) {error++; break;}
    nevt++;
    if (nevt%100 == 0) cout << nevt << endl;
  }
  cout << nevt << "  " << error << endl;
  //// Other possibility using the events Id directly through the keys
  //vector<IoSdKey> *keys=input.Keys();
  //int sz=keys->size();
  //for(int i=0;i<sz;i++) {
  // IoSdEvent event((*keys)[i].EvtId);
  //  if((*keys)[i].Multiplicity > 10) HighMult.Write(event);
  //  if(event.NumberOfErrorZeroStation > 5) out5.Write(event);
  //}

  out10.Close();
  out5.Close();
  return 0;
};
