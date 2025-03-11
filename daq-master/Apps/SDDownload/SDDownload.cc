#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "IkC.h"
#include "SDDownloadIk.h"
#include "SDDownloadPm.h"
#include "SDDownload.h"

#include "UiCmdLine.h"

string PmName;
// Vector list of participating stations
vector<int> Participating;
char *file;
char *fileName;
int fileSize;
int broadcast_init=0;
int DownloadId;

int main(int argc, char* argv[]) {
  struct stat statinfo;
  int stationid=0;

  // Building constants
  string name;
  name="SDDownload";
  PmName=PM;
  PmAddr="192.168.1.110";
  PmPort=10600;

  // Initialisation to CDAS
  CDASInit(name.c_str());

  // Reading downloadid
  FILE*f=fopen("/Raid/var/SDDownload","r");
  if (f==NULL) DownloadId=1;
  else {
    fscanf(f,"%d",&DownloadId);
    fclose(f);
  }
  DownloadId+=10;
  if (DownloadId>=65000) DownloadId=1;
  f=fopen("/Raid/var/SDDownload","w");
  if (f==NULL) {
    IkFatalSend("Cannot open /Raid/var/SDDownload for writing");
    exit(1);
  }
  fprintf(f,"%d\n",DownloadId);
  fclose(f);

  cout << "Welcome to the interactive download sequence." << endl;

  cout << "WARNING!!!!! TEST version with UUUUUUUUUUULLLLLLLLLLLTTTTTTTTTRRRRRRRRRRRAAAAAAAAAAA high speed!!!!!" << endl;
  cout << endl;
  cout << "YES!!!! You read it correctly!!!!" << endl;
  cout << "We work at 2 packets per second!!!" << endl;

  cout << "This is " << name << endl;
  cout << "You are goind to connect to " << PmName << endl << endl;
  cout << "Please answer the following questions:" << endl;
  cout << "Do you want to download a file to a specific station or to the whole array?" << endl;
  cout << "Enter the station id or 0 if you want a BROADCAST: ";
  cin >> stationid;
  // Broadcast: empty vector of stations
  if (stationid!=0) Participating.push_back(stationid);
  else broadcast_init=1;
  cout << "Enter the name of the file you want to download (with complete or relative path)" << endl;
  string s;
  cin >> s;
  file=(char *)s.c_str();
  if (file[0]=='~') {
    file=(char*)malloc(256);
    snprintf(file,256,"%s/%s",getenv("HOME"),((char *)s.c_str())+1);
  }
  if(stat(file,&statinfo)!=0 || !(S_ISLNK(statinfo.st_mode)||S_ISREG(statinfo.st_mode))) {
    cerr << "File not found or invalid. Sorry" << endl;
    return 1;
  }
  fileSize=statinfo.st_size;

  cout << "Enter the name the file should have in the Station" << endl;
  string t;
  cin >> t;
  fileName=(char *)t.c_str();
  cout << endl << endl;

  cout << "Good, you're going to send ";
  if (Participating.size()==0) cout <<  "in BROADCAST mode";
  else cout << "to station " << Participating[0];
  cout << " the file " << endl << file << " --> " << fileName  << " ("
  << NBSLICE << " slices)" << endl << endl;

  cout << "Is this correct? [yes|NO]" << endl;
  string answer;
  cin >> answer;
  if (strcmp(answer.c_str(),"yes")!=0) {
    cerr << "Goodbye" << endl;
    return 1;
  }

  // Asking for Ik Messages
  IkMonitor(SDDIk,"destination is \"%s\" or type is %d",name.c_str(),IKLSDOWNLOADACK);

  // Connect to Pm
  if (! SDDPmConnect()) {
    cerr << "Cannot connect to Pm." << endl;
    exit(1);
  }
  IkInfoSend("Starting Download");

  cerr << "Treating file " << file << ", " << fileSize << " bytes, " << NBSLICE << " slices."<< endl;
  // First Send
  SDDPmSendSlice(HEADSLICE);
  // Be nice with LS...
  sleep(5);
  SDDIkCheckPackets(HEADSLICE);

  for (int slice=0;slice<NBSLICE;slice++) {
    //	Listen to IkMessages.
    IkMessageCheck();
    // Check every CHECKEVERY messages sent
    if(slice%CHECKEVERY==0 && slice!=0) SDDIkCheckPackets(slice);
    // Be extra nice with LSX
    //sleep(2);
    // Send the new slice
    SDDPmSendSlice(slice);
    // Be nice with LSX
    if (slice%20==0) sleep(1);
    // Be TOO nice with LSX
    //sleep(2);
    // Answers a ping if necessary
    SDDIkPong();
  }
  SDDIkCheckPackets(NBSLICE);
  SDDPmClose();
  SDDIkSummary();
  return 0;
}
