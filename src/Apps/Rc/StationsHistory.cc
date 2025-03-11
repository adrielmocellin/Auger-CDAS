/* To show stations that went down in the last n minutes */

#include <stdio.h>
#include <string.h>
//#include <stream.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include <DbC.h>
#include <IkC.h>

void StationIdtoName(int StationId,char *Name) {
static SdTank*  tankList = DbCLoadArray(); int i = 0;
strcpy(Name,"Unknown");
while (tankList[i].Id != -1 && tankList[i].Id != StationId) ++i;
if (tankList[i].Id == StationId) strcpy(Name, tankList[i].Name);
return;
}
  
int logicd(int StationId,char *Name){
static SdTank* tankList = DbCLoadArray(); int i = 0;
while (tankList[i].Id != -1 && tankList[i].Id != StationId) ++i;
return tankList[i].Valid;
}

int main(int argc, char *argv[]) {

  long int gpstime,aux1,firstvalidGPStime;
  int StationId,CountT2,validGPSlines=0,TargetStation=0;
  long int SumT2[1601], /* Sum of T2 */
    EntriesT2[1601], /* Number o entries of T2 */
    SumT22[1601], /* Sum of squared T2 */
    Nfailures[1601]; /* Number of failures */
  long int Tfailure[1601], /* Total failure time */
    Lastfailure[1601], /* GPS Time of last failure */
    Lastseen[1601]; /* GPS Time of last time station sent T2 */
  char *tmpname; /* Name of temporary file */
  FILE *f1; /* Stream descriptor of temporary file */
  char shellcommand[500];
  int Interval=900;
  int SystemFailure=0;
  int CountSystemFailureLines=0;
  int CountSystemFailures=0;
  int lastvalidGPStime=0;
  int charsafter=0;
  int Valid;
  char Name[200];
  char InputFileName[200];

  if ((argc>1) && (argv[1]=="-h")) {
    cout << "Usage: StationsDown [timeinterval_in_seconds] [station id (0=all)] [input_file]\n";
    exit(0);
  }
   CDASInit("RcHistory");
  if (argc>1) sscanf(argv[1],"%d",&Interval);
  if (Interval<=0) Interval=900;
  if (argc>2) sscanf(argv[2],"%d",&TargetStation);
  if (argc==4) {
    strncpy(InputFileName,argv[2],200);
    tmpname=InputFileName;
  } else {
    tmpname="/tmp/T2_XXXXXX";
    int if1=mkstemp(tmpname); // because they don't like tempnam...
    close(if1);
    sprintf(shellcommand,"%s -%d %s","/usr/bin/tail",Interval,"/Raid/var/trash/Xb > ");
    strcat(shellcommand,tmpname);
    if (system(shellcommand)) {
      cout << "Could not execute command: " << shellcommand << "\n";
      exit(-1);
    }
  }
  for (int i=1;i<=1600;i++) {
    SumT2[i]=0;
    EntriesT2[i]=0;
    SumT22[i]=0;
    Nfailures[i]=0;
    Tfailure[i]=0;
    Lastfailure[i]=0;
    Lastseen[i]=0;
  }
  char firstime=1;
  if (f1=fopen(tmpname,"r")) {
    while (! feof(f1)) {
      charsafter=0;
      do {                       // search for the next usefull line 
	fscanf(f1,"%ld",&aux1);
	if (SystemFailure) charsafter++;
	switch (aux1) {
	case 0:
	  SystemFailure=1;
	  charsafter=0;
	  break;
	case -1:
	  if ((SystemFailure>=1) && (charsafter==1)) {
	    SystemFailure++;
	    charsafter=0;
	  } else SystemFailure=0;
	  break;
	}
	if (SystemFailure==3) CountSystemFailureLines++;
	if ((CountSystemFailureLines==1) && (SystemFailure==3)) {
	  CountSystemFailures++;
	  SystemFailure=0;
	}
      } while ((aux1 < 100000000) && (! feof(f1)));
      SystemFailure=0;
      if ((aux1 > 100000000) && (! feof(f1))) {    // valid GPS time found
	if (firstime) {
	  lastvalidGPStime=aux1;
	  firstvalidGPStime=aux1;
	} else lastvalidGPStime=gpstime;
	gpstime=aux1;
	++validGPSlines;
	//  	cout << gpstime << "\n";
	fscanf(f1,"%d %d",&StationId,&CountT2);
      }
      while ((StationId > 0) && (StationId <=1600) && (! feof(f1)) ) {  // process tank ids and T2 until -1 is found 
	if (firstime) {                 // first usefull line in the file
	  Lastseen[StationId]=gpstime;
	  SumT2[StationId]=CountT2;
	  SumT22[StationId]=CountT2*CountT2;
	  ++(EntriesT2[StationId]);
	} else {                       // following usefull lines
	  SumT2[StationId] = SumT2[StationId]+CountT2;
	  SumT22[StationId] = SumT22[StationId]+CountT2*CountT2;
	  ++(EntriesT2[StationId]);
	  if ((Lastseen[StationId] != (gpstime - 1)) && (CountSystemFailureLines==0)) {
	    if (Lastseen[StationId]==0) Lastseen[StationId]=firstvalidGPStime;
	    Tfailure[StationId] += gpstime - Lastseen[StationId];
	    ++(Nfailures[StationId]);
	    Lastfailure[StationId] = Lastseen[StationId] + 1;
	  } else if (Lastseen[StationId] != lastvalidGPStime) {
	    Tfailure[StationId] += gpstime - Lastseen[StationId];
	    ++(Nfailures[StationId]);
	    Lastfailure[StationId] = Lastseen[StationId] + 1;
	  }
	}
	Lastseen[StationId] = gpstime;
	fscanf(f1,"%d %d",&StationId,&CountT2);
      }
     CountSystemFailureLines=0;
      firstime=0;
    }                               // next line
    if (!TargetStation) printf("%s\n","  Id  UP? InRun               Name   <#T2> stdv  #out out[s]      last time failed");
    int countstationsup=0;
    char UpOrDown[4];
    char InRun[4];
    for (int i=1;i <= 1600; i++) {
      Valid=logicd(i,Name);
      if (Lastseen[i] != 0 ) {
	countstationsup++;
	if (Lastseen[i] != gpstime ) {
	  Tfailure[i] += gpstime - Lastseen[i];
	  ++(Nfailures[i]);
	  Lastfailure[i] = Lastseen[i] + 1;
	  strcpy(UpOrDown,"NO");
	} else strcpy(UpOrDown,"YES");
	if (InRun) strcpy(InRun,"yes"); else strcpy(InRun,"no");
	double averageT2 = double(SumT2[i])/double(EntriesT2[i]);
	double sigmaT2 = double(EntriesT2[i])/double(EntriesT2[i] - 1);
	sigmaT2 = sqrt(sigmaT2 * (double(SumT22[i])/double(EntriesT2[i]) - pow(averageT2,2)));
	Lastfailure[i]=Lastfailure[i]+315964800-13;
	char StationName[500];
	StationIdtoName(i,StationName);
	if (!TargetStation) printf("%4d  %3s  %3s %20s  %3.0lf  %3.0lf  %4ld  %4ld   ",i,UpOrDown,InRun,StationName,averageT2,sigmaT2,Nfailures[i],Tfailure[i]);
	else if (TargetStation==i) printf("%d %s (%s) %d+-%d %lds out\n",i,UpOrDown,StationName,int(averageT2),int(sigmaT2),Tfailure[i]);
	if (!TargetStation && Tfailure[i]>0) {
	  printf("%24s",ctime((time_t *)(&Lastfailure[i])));
	} else if (!TargetStation) printf("%s\n","       Never down."); 
      }
    }
		if (!TargetStation) {
    printf("%s %ld %s\n","Total measured time: ",gpstime - firstvalidGPStime + 1," seconds.");
    printf("%s %d %s\n","System running for ",validGPSlines," seconds.");
    printf("%s %d\n","Total number of System failures: ",CountSystemFailures);
    gpstime=gpstime+315964800-13;
    printf("%s %24s","Last valid GPS time:",ctime((time_t *)(&gpstime )));
    printf("%s %d %s\n","There were ",countstationsup," stations up during the measured time.");
		}
    fclose(f1);
    if (argc<3) {
      strcpy(shellcommand,"rm -f ");
      strcat(shellcommand,tmpname);
      if (system(shellcommand)) cout << "Could not remove temporary file " << tmpname << ".\n";
    }
  } else cout << "Could not open temporary file " << tmpname << ".\n";
}
