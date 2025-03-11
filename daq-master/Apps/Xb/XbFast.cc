/*! \file
  \brief Main program
*/

#include "IkC.h"
#include "XbIk.h"
#include "XbPm.h"
#include "XbArray.h"
#include "XbAlgo.h"

#include "UiCmdLine.h"
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <set>

// turn on saturation trigger?
#define SATUR 0
#ifdef SATUR
void SaturationTrigger(struct t2*input,int nb);
#endif

// turn on AERALET?
#define AERALET 1
#ifdef AERALET
int aeraletid[] = {11, 12, 27, 28, 29, 30, 42, 47, 50, 54, 97, 99, 688, 734, 1622, 1764, 1765, 1769, 1773};
set<int> aeraletidset;
#endif

// turn on lightning trigger?
//#define LIGHTNING 1

// turn on new lightning trigger?
//#define NEWLIGHTNING 1

// turn on Celeste trigger?
//#define EASIER 1

// turn on AMBER/UDP transmit?
#define AMBER 1
int trigamb = 0;

// turn on lightning UDP trigger?
#define UDPLIGHTNING 1

#ifdef UDPLIGHTNING
using boost::asio::ip::udp;
using namespace std;

struct t2p {
  int time;
  unsigned short ID;
  unsigned short energy;
  int sec;
};

struct light {
  int sec;
  int nanosec;
  int X;
  int Y;
};

class udp_server {
public:
  udp_server(boost::asio::io_service& io_service)
    : socket_(io_service, udp::endpoint(udp::v4(), 54448)) {
    t2buf=(struct t2p *)malloc(NT2MAX*60*sizeof(struct t2p));
    lbuf=(struct light *)malloc(30*sizeof(struct light));
    pos=0;
    lpos=0;
    delay=0;
    for (int i=0;i<NT2MAX*60;i++) t2buf[i].sec=0;
    for (int i=0;i<30;i++) lbuf[i].sec=0;
    start_receive();
    // lightning are referred to UTMPoint centralCampusUTM(6075445*m, 446954*m, 1444*m, 19, 'H',wgs84);
    double Alpha=0.00252;
    double Beta=0.000603;
    double N0=6075445;
    double E0=446954;
    for (int i=0;i<2000;i++) {
      arrayX[i]=-999999999;
      arrayY[i]=-999999999;
    }
    for (int i=0;i<nstat;i++) {
      arrayX[sdarray[i].Id]=(1 + Beta) * (sdarray[i].Easting - E0) + Alpha * (sdarray[i].Northing - N0);
      arrayY[sdarray[i].Id]=(1 + Beta) * (sdarray[i].Northing - N0) - Alpha * (sdarray[i].Easting - E0);
    }
  }

  int pos,lpos;
  struct t2p *t2buf;
  struct light *lbuf;
  // t3 stuff
  int delay;
  int arrayX[2000];
  int arrayY[2000];

  int check(int sec, int nanosec, int X, int Y) {
    int ret=0,ok=0,isfile=0;
/*
    struct stat stat_buf;
    stat_buf.st_size=0;
    isfile=stat("/home/cdas/XbFast2/LightTrigger.dat.bz2", &stat_buf);
    if (stat_buf.st_size<500000000 || (isfile!=0)) {
*/
    FILE *f=NULL;
    time_t t = sec;
    t += 315964800;
    struct tm * stm=gmtime(&t);
    stringstream dirnm;
    stringstream fileend;
    stringstream command;
#define MODE (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
    dirnm << "/home/cdas/XbFast2/";
    mkdir(dirnm.str().c_str(),MODE);
    dirnm << stm->tm_year+1900 << "/";
    mkdir(dirnm.str().c_str(),MODE);
    if (stm->tm_mon<9) dirnm << "0";
    dirnm << stm->tm_mon+1 << "/";
    mkdir(dirnm.str().c_str(),MODE);
    fileend << "_" << stm->tm_year+1900 << "_";
    if (stm->tm_mon<9) fileend << "0";
    fileend << stm->tm_mon+1 << "_";
    if (stm->tm_mday<10) fileend << "0";
    fileend << stm->tm_mday << "_00h00.dat.bz2";
    command << "bzip2 -c >> " << dirnm.str() << "LightningTrigger" << fileend.str();

    for (int i=0;i<NT2MAX*60;i++) {
      if (t2buf[i].sec==sec) {
        if (!ok) {
          f=popen(command.str().c_str(),"w");
          if (f) fprintf(f,"# %d %d %d %d ( %d %d - %d %d )\n",sec,nanosec,X,Y,t2buf[(pos+(NT2MAX*60)-1)%(NT2MAX*60)].sec,t2buf[(pos+(NT2MAX*60)-1)%(NT2MAX*60)].time,t2buf[pos].sec,t2buf[pos].time);
        }
        // fprintf(f,"%d %d %d %d\n",t2buf[i].sec,t2buf[i].time,t2buf[i].ID,t2buf[i].energy); // no longer needed, we have the t2 raw files now
        ok=1;
        // t3 check
        if ((X>-20000) && (X<70000) && (Y>-20000) && (Y<70000)) { // lightning strike roughly in array
          if ((t2buf[i].time>(nanosec/1000-1000)) && (t2buf[i].time<(nanosec/1000+5))) { // time coincidence
            if (t2buf[i].ID>0 && t2buf[i].ID<2000) { // good id
              if ((X>arrayX[t2buf[i].ID]-5000) && (X<arrayX[t2buf[i].ID]+5000) && (Y>arrayY[t2buf[i].ID]-5000) && (Y<arrayY[t2buf[i].ID]+5000)) { // 5km x 5km box
                if (((X-arrayX[t2buf[i].ID])*(X-arrayX[t2buf[i].ID])+(Y-arrayY[t2buf[i].ID])*(Y-arrayY[t2buf[i].ID]))<5000*5000) { // 5km circle
                  if (time(NULL)>delay+60) { // No T3 in last minute, send one
                    // build the T3.
                    IkT3 ikm;
                    ikm.mode="LIST";
                    ikm.algo="LIGHTNING";
                    ikm.SDPAngle=0.;
                    ikm.id=XbIkGetId();
                    if (f) fprintf(f,"%d %d %d %d %d %d\n",sec,nanosec,X,Y,t2buf[i].ID,t2buf[i].time);
                    ikm.refSecond=sec;
                    ikm.refuSecond=t2buf[i].time;
                    ikm.addresses.push_back(t2buf[i].ID);
                    ikm.offsets.push_back(0);
                    ikm.window.push_back(0);
                    ikm.send("Pm");
                    delay=time(NULL);
                  }
                }
              }
            }
          }
        }
      }
      if (t2buf[i].sec>=sec) ret=1;
    }
    if (f) pclose(f);
/*
    } else {
      stringstream s;
      s << "Error: lightning file full";
      InternalLog(s.str().c_str(),IKWARNING);
      ret=1;
    }
*/
    if (ret && (!ok)) {
      stringstream s;
      s << "Error: lightning second " << sec << " in buffer but without events at T2 second " << second_;
      InternalLog(s.str().c_str(),IKWARNING);
    }
    return ret;
  }

  void fill(struct t2 *input, int nbt2) {
    for (int i=0;i<nbt2;i++) {
      t2buf[(i+pos)%(NT2MAX*60)].time=input[i].time;
      t2buf[(i+pos)%(NT2MAX*60)].ID=input[i].ID;
      t2buf[(i+pos)%(NT2MAX*60)].energy=input[i].energy;
      t2buf[(i+pos)%(NT2MAX*60)].sec=second_;
    }
    pos=(pos+nbt2)%(NT2MAX*60);
    for (int i=0;i<30;i++) {
      if (lbuf[i].sec) {
        stringstream s;
        s << "Checking lightning second " << lbuf[i].sec << " at T2 second " << second_;
        InternalLog(s.str().c_str(),IKDEBUG);
        if (check(lbuf[i].sec,lbuf[i].nanosec,lbuf[i].X,lbuf[i].Y)) lbuf[i].sec=0;
      }
    }
  }

private:
  void start_receive() {
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), remote_endpoint_,
        boost::bind(&udp_server::handle_receive, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

  void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error || error == boost::asio::error::message_size) {
      recv_buffer_[bytes_transferred]='\0';
      if (lbuf[lpos%30].sec!=0) { // erasing a lightning without T2 data
        FILE *f=popen("bzip2 -c >> XbFast2Light.dat.bz2","w");
        fprintf(f,"# %d %d %d %d lost at t2 second %d\n",lbuf[lpos%30].sec,lbuf[lpos%30].nanosec,lbuf[lpos%30].X,lbuf[lpos%30].Y,second_);
        pclose(f);
      }
      sscanf(recv_buffer_,"%d %d %d %d\n",&(lbuf[lpos%30].X),&(lbuf[lpos%30].Y),&(lbuf[lpos%30].sec),&(lbuf[lpos%30].nanosec));
      stringstream s;
      s << "Got lightning trigger: " << lbuf[lpos%30].sec << " " << lbuf[lpos%30].nanosec << " " << lbuf[lpos%30].X << " " << lbuf[lpos%30].Y << " at second " << second_;
      s << " with buffer having seconds " << t2buf[(pos+(NT2MAX*60)-1)%(NT2MAX*60)].sec << " - " << t2buf[pos].sec;
      s << " stored at position " << lpos;
      InternalLog(s.str().c_str(),IKDEBUG);
      lpos=(lpos+1)%30;
      start_receive();
    }
  }

  udp::socket socket_;
  udp::endpoint remote_endpoint_;
  char recv_buffer_[64000];
};
#endif // UDPLIGHTNING

#ifdef AERALET
// 1000 T2 max from aera...
struct t2 aera[1000];
int nbaera=0;
// 20 ms of T2
//#define AERATIME 20000
#define AERATIME 2000000
#define DT 100
#define SDT 10
// SDT=sqrt(DT)

// note: I am limiting to 1 T3/s. It makes the algorithm simple and should not saturate bandwidth if there is lightning...
void AeraletTrigger(struct t2*input,int nb) {
  nbaera=0;
  for (int i=0;i<nb;i++) {
    if ((aeraletidset.find(input[i].ID)!=aeraletidset.end())
	&& (input[i].time<AERATIME)) {
      aera[nbaera].ID=input[i].ID;
      aera[nbaera].time=input[i].time;
      aera[nbaera].energy=input[i].energy;
      nbaera++;
      if (nbaera>999) nbaera=999;
    }
  }
  if (verbose) {
    stringstream s;
    s << nbaera << " T2 from AERALET in " << AERATIME/1000 << " ms window.";
    InternalLog(s.str().c_str(),IKDEBUG);
  }
  // just one hexagon, using brute force
  int t3time=-1;
  for (int i=0;i<nbaera;i++) {
    for (int j=i+1;j<nbaera;j++) {
      if (aera[i].ID==aera[j].ID) continue;
      for (int k=j+1;k<nbaera;k++) {
        if (aera[k].ID==aera[i].ID) continue;
        if (aera[k].ID==aera[j].ID) continue;
        // 3 stations with different ID. looking if they are within 6us
        double dt1=1.0*aera[i].time-1.0*aera[j].time;
        double dt2=1.0*aera[i].time-1.0*aera[k].time;
        double dt3=1.0*aera[k].time-1.0*aera[j].time;
        if (dt1*dt1>DT || dt2*dt2>DT || dt3*dt3>DT) continue;
        // Have one T3...
        t3time=aera[i].time;
        i=j=k=nbaera;
      }
    }
  }
  if (t3time!=-1) {
    // build the T3.
    IkT3 ikm;
    ikm.refSecond=second_;
    ikm.mode="LIST";
    ikm.algo="AERA";
    ikm.SDPAngle=0.;
    ikm.id=XbIkGetId();
    // First, get minimum time
    ikm.refuSecond=t3time;
    for (int i=0;i<nbaera;i++) {
      double dt=1.0*aera[i].time-1.0*t3time;
      if (dt*dt<=DT) {
        if (aera[i].time<(int)ikm.refuSecond) ikm.refuSecond=aera[i].time;
      }
    }
    // Then, add stations
    for (int i=0;i<nbaera;i++) {
      double dt=1.0*aera[i].time-1.0*ikm.refuSecond;
      if (dt>=0 && dt<SDT) {
        ikm.addresses.push_back(aera[i].ID);
        ikm.offsets.push_back(dt);
        ikm.window.push_back(0);
      }
    }
    // UUB doublet. Add them if KT was there
    // 21/10/2016, changing trigger to always add the extra doublet, just in case KT is down
    // 16/01/2018, changing to add extra triplet when 1764, 47 or 99 are present
    // 31/05/2018, changing to add all stations when this happens
    int ktwasthere=0;
    for (int i=0;i<nbaera;i++) {
      if (aera[i].ID==1764 || aera[i].ID==47 || aera[i].ID==99) {
        ktwasthere=1;
      }
    }
    if (ktwasthere) { // Add stations 20, 22, 39 and 41, with 0 dt and a window of 20
      ikm.addresses.push_back(20);
      ikm.offsets.push_back(0);
      ikm.window.push_back(20);

      ikm.addresses.push_back(22);
      ikm.offsets.push_back(0);
      ikm.window.push_back(20);

      ikm.addresses.push_back(39);
      ikm.offsets.push_back(0);
      ikm.window.push_back(20);

      ikm.addresses.push_back(41);
      ikm.offsets.push_back(0);
      ikm.window.push_back(20);

      set<int>::iterator it;
      for (it=aeraletidset.begin();it!=aeraletidset.end();it++) {
        int ok=1;
        for (vector<int>::iterator it2=ikm.addresses.begin(); it2!=ikm.addresses.end();it2++) {
          if (*it2==*it) ok=0;
        }
        if (ok) {
	  ikm.addresses.push_back(*it);
	  ikm.offsets.push_back(0);
	  ikm.window.push_back(20);
        }
      }
    }
    ikm.send("Pm");
  }
}
#endif

#ifdef EASIER
int easier_sec=0;


void EasierTrigger(struct t2*input,int nb) {
  for (int i=0;i<nb;i++) {
    if (input[i].ID==203 && input[i].time>250000 && input[i].time<250030) {
      if (second_>easier_sec+240) {
        easier_sec=second_;
        IkT3 ikm;
        ikm.id=XbIkGetId();
        ikm.refSecond=second_;
        ikm.refuSecond=input[i].time;
        ikm.mode="LIST";
        ikm.algo="OBSERVER";
        ikm.SDPAngle=0.;
        ikm.addresses.push_back(input[i].ID);
        ikm.offsets.push_back(0);
        ikm.window.push_back(0);
        ikm.send("Pm");
      }
    }
  }
}
#endif

#ifdef NEWLIGHTNING
double avgscal[2000];
int candidate[2000];
int nblighttrig[2000];
unsigned int second_lightning=0;

int NewLightningTrigger(struct t2*input,int nb) {
  int oid=0;
  int id=0;
  int ot=0;
  int t=0;
  int k=0;
  for (int i=0;i<nb;i++) { // triggers are ordered by ID, then time
    if (input[i].energy!=1 && input[i].energy!=9) continue; // keep tot y thres
    oid=id;
    ot=t;
    id=input[i].ID;
    t=input[i].time;
    if (id==oid && t-ot>40 && t-ot<80 && input[i].energy==9) {
      if (candidate[id]) {
        stringstream s;
        s << "Lightning candidate: " << id << " " << input[i].time << " " << t-ot;
        InternalLog(s.str().c_str(),IKDEBUG);
        if (second_> second_lightning+60 && nblighttrig[id]<3) {
          second_lightning=second_;
          IkT3 ikm;
          ikm.id=XbIkGetId();
          ikm.refSecond=second_;
          ikm.refuSecond=ot;
          ikm.mode="LIST";
          ikm.algo="LIGHTNING";
          ikm.SDPAngle=0.;
          ikm.addresses.push_back(id);
          ikm.offsets.push_back(0);
          ikm.window.push_back(0);
          ikm.send("Pm");
          ikm.id=XbIkGetId();
          ikm.refuSecond=t;
          ikm.send("Pm");
        }
        nblighttrig[id]++;
      }
      k++;
    }
  }
  return k;
}
#endif

#ifdef AMBER
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SERVERS 20

#define PORT 50477 /* CHANGE */

#define INSERT_UDP(variable,datagram) {			\
    if (sizeof(variable)==2)				\
      variable = htons(variable);			\
    else if (sizeof(variable)==4)			\
      variable = htonl(variable);			\
							\
    memcpy(datagram,&variable,sizeof(variable));	\
    datagram += sizeof(variable);			\
							\
    if (sizeof(variable)==2)				\
      variable = ntohs(variable);			\
    else if (sizeof(variable)==4)			\
      variable = ntohl(variable);			\
  }

struct XbFast_Connection {
  struct sockaddr_in server;
  int s;
  int slen;
  int num_servers;
};

struct XbFast_Connection connection[MAX_SERVERS];

void diep(const char *s) {
  perror(s);
}

int XbFast_udp_init(const char *file) {
  int i,j;
  
  for(i=0;i<MAX_SERVERS;i++) {
    connection[i].num_servers = 0;
    connection[i].slen = sizeof(connection[i].server);
  }

  FILE *fp;
  char ip_buff[30];
  fp = fopen(file,"r");

  /* open file */
  if(fp==NULL) {
    fprintf(stderr,"Error reading file %s\n",file);
    return -1;
  }

  /* creat socket */
  for(i=0;i<MAX_SERVERS;i++) {
    if( (connection[i].s=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1 ) {
      diep("socket");
      return -1;
    }
  }

  /* initialize structures - may not be necessary, just following tutorial */
  /* also, set some variables (gotta do that!)*/
  for(i=0;i<MAX_SERVERS;i++) {
    memset( (char *) &(connection[i].server),0,sizeof(connection[i].server) );
    connection[i].server.sin_family = AF_INET;
    connection[i].server.sin_port = htons(PORT);
  }


  int qscp=0;
  int qport=PORT;
  char qipad[20];
  string * qtmp;
  
  /* read server ips (and perhaps ports) from file, and set the for the connections */
  j=0;
  while(fscanf(fp,"%s",ip_buff) != EOF) {
    /* Added by R.Squartini 07-Dec-2011 allow sending to multiple ports in one IP   */
    qtmp = new string (ip_buff);
    qscp=qtmp->find(':');
    if (qscp!=string::npos) {
      /* Get port number and set into connection.[].server.sin_port */
      qport=atoi((qtmp->substr(qscp+1)).c_str());
      strcpy(qipad,(qtmp->substr(0,qscp)).c_str());
      connection[j].server.sin_port = htons(qport);
      strcpy(ip_buff,qipad);
    }
    /* */
  
    /* printf("%s\n",ip_buff); */ /*for debugging*/

    if( inet_aton(ip_buff, &(connection[j].server.sin_addr)) == 0) {
      fprintf(stderr,"inet_aton() failure on ip %s\n",ip_buff);
      return -1;
    }
    
    for(i=0;i<MAX_SERVERS;i++) {
      connection[i].num_servers++;
    }
    
    j++;
  }

  return 0;
}

int XbFast_udp_send(const char *datagram, unsigned int len) {
  int i;
  for(i=0;i<connection[0].num_servers;i++) {
    /* printf("%s\n", inet_ntoa(connection[i].server.sin_addr)); */
    if( sendto(connection[i].s,datagram,len,0,(struct sockaddr *) &(connection[i].server),connection[i].slen) == -1) {
      diep("sendto()");
      return -1;
    }	
  }
  return 0;
}

int XbFast_udp_shutdown() {
  int i;
  for(i=0;i<connection[0].num_servers;i++) {
    close(connection[i].s);
  }
  return(0);
}

#endif

int newt2config=1,verbose=1,debug=0,trivial_algo=1800,carmen_miranda=600;

void AmberExtTrigger(struct t2*input,int nb);

int main(int argc, char* argv[]) {
  struct t2 input[NT2MAX];
  struct t2 scaler[NT2MAX];

  /* Reading command line */
  UiParseCommandLine(argc,argv);
  if (argc==1) {
    PmAddr="192.168.2.1";
    PmPort=10600;
  }

  /* Initialisation to CDAS  */
  CDASInit("XbFast");

  /* Asking for Ik Messages */
  IkMonitor(XbIk,"destination is \"XbFast\" or type is %d",IKSDREQUEST);

  XbArrayInit(1);

  /* Connect to Pm */
  if (! XbPmConnect("XbFast",PmT2Fast)) {
  /* if (! XbPmConnect("XbFast",PmT2)) { */
    stringstream s;
    s << "Cannot connect to Pm.";
    InternalLog(s.str().c_str(),IKFATAL);
    exit(1);
  }

  int nt2=0,nsc=0;
  //unsigned char trigamb=0;
  XbAlgoT2Init();
  XbIkInitId("/Raid/var/XbFast",8200,9000);
  XbPmSendStatus(LOG);

  #ifdef AMBER
  /* Connect to AMBER, other recievers of UDP information */
  XbFast_udp_init("/Raid/var/XbFast.servers.list");
  #endif
  #ifdef NEWLIGHTNING
  int nbsec=0;int nbcand=0;
  for (int i=0;i<2000;i++) avgscal[i]=0;
  for (int i=0;i<2000;i++) nblighttrig[i]=0;
  #endif
  #ifdef UDPLIGHTNING
  boost::asio::io_service io_service;
  udp_server server(io_service);
  #endif
  #ifdef AERALET
  aeraletidset=set<int>(aeraletid, aeraletid+sizeof(aeraletid)/sizeof(aeraletid[0]));
  #endif
  while (1) {
    //=====================
    //  Listen to IkMessages.
    IkMessageCheck();
    nt2=XbPmGetT2(input,0,scaler,&nsc);
    if (verbose) {
      stringstream s;
      s << nt2 << " T2 from Pm.";
      InternalLog(s.str().c_str(),IKDEBUG);
    }
    vector<t2list> out=XbAlgoT2Process(input,nt2);
    AmberExtTrigger(input,nt2);
#ifdef AERALET
    AeraletTrigger(input,nt2);
#endif
#ifdef EASIER
    EasierTrigger(input,nt2);
#endif
#ifdef SATUR
    SaturationTrigger(input,nt2);
#endif
#ifdef NEWLIGHTNING
    {
    if (second_%3600==0)
      for (int i=0;i<2000;i++) if (nblighttrig[i]>0) nblighttrig[i]--;
    stringstream scalstr;
    int nbcandidate=0;
    scalstr << "Scaler high rate: ";
    for (int i=0;i<2000;i++) candidate[i]=0;
    for (int i=0;i<nsc;i++) {
      if (scaler[i].time>500 && ((scaler[i].time>>14)==0) && scaler[i].ID<2000) {
        if (!avgscal[scaler[i].ID]) avgscal[scaler[i].ID]=scaler[i].time;
        if (scaler[i].time>avgscal[scaler[i].ID]) avgscal[scaler[i].ID]+=0.1;
        else avgscal[scaler[i].ID]-=0.1;
        if (scaler[i].time>avgscal[scaler[i].ID]+500 && avgscal[scaler[i].ID]>1500 && avgscal[scaler[i].ID]<2500) {
          scalstr << scaler[i].ID << ": " << scaler[i].time << ", ";
          candidate[scaler[i].ID]=1;
          nbcandidate++;
        }
      }
    }
    if (nbcandidate) InternalLog(scalstr.str().c_str(),IKDEBUG);
    nbsec++;
    nbcand+=NewLightningTrigger(input,nt2);
    if (nbsec>59) {
      stringstream s;
      s << nbcand << " candidate lightning in " << nbsec << " seconds ("<<nbcand/nbsec << "/sec)";
      InternalLog(s.str().c_str(),IKDEBUG);
      nbcand=0;
      nbsec=0;
    }
    }
#endif
#ifdef UDPLIGHTNING
    server.fill(input,nt2);
    io_service.poll();
#endif

    struct timeval tv;
    while (out.size()>0) {
  //    cout << "ALGO ===== " << out[out.size()-1].algo << endl;
      t3 res=XbAlgoT3Build(out[out.size()-1]);
      gettimeofday(&tv, NULL);
      cout << "# New event" << endl;
      cout << res.refSecond << " " << res.refuSecond << " " << tv.tv_sec-315964800+14 << " " << tv.tv_usec;

      if(strcmp(res.algo,"OBSERVER_TOT")==0){
         XbIkT3Emit(res);
      }

#ifdef LIGHTNING
    int nbscalt3=0;
    double scalt3=0;
    double maxscalt3=0;
      for (unsigned int i = 0; i < res.point.size(); i++) {
        if (res.point[i].window!=0) continue;
        if (res.point[i].ID==945 ||
            res.point[i].ID==948 ||
            res.point[i].ID==644 ||
            res.point[i].ID==648 ||
            res.point[i].ID==956 ||
            res.point[i].ID==949 ||
            res.point[i].ID==944) continue; // removing test tanks
        cout << " " << res.point[i].ID; 
        int tmpsc=0;
        for (unsigned int j=0;j<nsc;j++) 
          if (scaler[j].ID==res.point[i].ID) tmpsc=scaler[j].time;
        cout << " " << tmpsc;
        if (tmpsc>1000) {
          scalt3+=tmpsc;
          if (tmpsc>maxscalt3) maxscalt3=tmpsc;
          nbscalt3++;
        }
      }
      cout << endl;
      // compute average scaler
      if (nbscalt3>1)
        if ((scalt3-maxscalt3)/(nbscalt3-1)>2300) {
          time_t t = second_;
          t += 315964800;
          struct tm * stm=gmtime(&t);
          char fname[256];
          snprintf(fname,256,"xblightt2_%d_%02d_%02d_00h00.dat",stm->tm_year+1900,stm->tm_mon+1,stm->tm_mday);
          FILE *ft2=fopen(fname,"a");
          fprintf(ft2,"%d %d %f ",res.refSecond,res.refuSecond,(scalt3-maxscalt3)/(nbscalt3-1));
          for (unsigned int i=0;i<nt2;i++)
            fprintf(ft2,"%d %d %d ",input[i].ID,input[i].time,input[i].energy);
          fprintf(ft2,"\n");
          fclose(ft2);
          // T3???
          for (unsigned int i = 0; i < res.point.size(); i++) {
            if (res.point[i].window!=0) continue;
            for (unsigned int j=0;j<nt2;j++) {
              if (res.point[i].ID!=input[j].ID) continue;
              int ttk=res.refuSecond+res.point[i].offset;
              if (input[j].time>ttk-1000 && input[j].time<ttk-1) {
                IkT3 ikm;
                ikm.id=XbIkGetId();
                ikm.refSecond=res.refSecond;
                ikm.refuSecond=input[j].time;
                ikm.mode="LIST";
                ikm.algo="OBSERVER";
                ikm.SDPAngle=0.;
                ikm.addresses.push_back(input[j].ID);
                ikm.offsets.push_back(0);
                ikm.window.push_back(0);
                ikm.send("Pm");
              }
            }
          }
        }
#else
      cout << endl;
#endif
      // AMBER trigger
#ifdef AMBER
      //       Xavier's old AMBER stuff (for reference)

      //        FILE *famber=fopen("amber.dat","a");
      //        fprintf(famber,"!T3! %d %d %d %d %d %d",0,trigamb++,res.refSecond,res.refuSecond,tv.tv_sec-315964800+14, tv.tv_usec);
      //        int nbtrigamb=0;
      //        for (unsigned int i = 0; i < res.point.size(); i++) {
      //          if (res.point[i].window!=0) continue;
      //          nbtrigamb++;
      //        }
      //        fprintf(famber," %d",nbtrigamb);
      //        for (unsigned int i = 0; i < res.point.size(); i++) {
      //          if (res.point[i].window!=0) continue;
      //          fprintf(famber," %d %d",res.point[i].ID,res.point[i].offset);
      //        }
      //        fprintf(famber,"\n");
      //        fclose(famber);

      uint8_t header_udp[4];

      uint16_t am_payload_size_udp;
      uint32_t trigamb_udp;
      uint8_t nbtrigamb_udp;
      uint32_t refSec_udp, refuSec_udp, tv_sec_udp, tv_usec_udp;

      uint16_t station_id_udp;
      uint32_t station_usec_udp;

      int nbtrigamb=0;
      for (unsigned int i = 0; i < res.point.size(); i++) {
	if (res.point[i].window!=0) continue;
	nbtrigamb++;
      }

      // how big is the datagram?
      int am_payload_size = (1+1+1+1+2+4+4+4+4+4+1+6*nbtrigamb);

      char* am_datagram = new char[am_payload_size];
      char* am_datagram_curr = am_datagram;

      // start assembling datagram, convert to fixed width variables

      const char *header_pre = "!T3!";

      for(int i = 0;i<4;i++) {
	header_udp[i] = (uint8_t) header_pre[i];
	INSERT_UDP(header_udp[i],am_datagram_curr);
      }

      am_payload_size_udp = (uint16_t) am_payload_size;
      trigamb_udp = (uint32_t) trigamb++;
      refSec_udp = (uint32_t) res.refSecond;
      refuSec_udp = (uint32_t) res.refuSecond;
      tv_sec_udp = (uint32_t) tv.tv_sec;
      tv_usec_udp = (uint32_t) tv.tv_usec;
      nbtrigamb_udp = (uint8_t) nbtrigamb;

      INSERT_UDP(am_payload_size_udp,am_datagram_curr);
      INSERT_UDP(trigamb_udp,am_datagram_curr);
      INSERT_UDP(refSec_udp,am_datagram_curr);
      INSERT_UDP(refuSec_udp,am_datagram_curr);
      INSERT_UDP(tv_sec_udp,am_datagram_curr);
      INSERT_UDP(tv_usec_udp,am_datagram_curr);
      INSERT_UDP(nbtrigamb_udp,am_datagram_curr);

      for(unsigned int i=0;i < res.point.size();i++) {
	if (res.point[i].window!=0) continue;
	station_id_udp = (uint16_t)res.point[i].ID;
	station_usec_udp = (uint32_t)res.point[i].offset;
	INSERT_UDP(station_id_udp,am_datagram_curr);
	INSERT_UDP(station_usec_udp,am_datagram_curr);
      }

      /* Send the packet */
      XbFast_udp_send(am_datagram,am_payload_size);
       
      delete [] am_datagram;
       
#endif
      XbAlgoFree(out[out.size()-1]);
      out.pop_back();
    }
  }
  #ifdef AMBER
  /* close UDP connections */
  XbFast_udp_shutdown();
  #endif
  return 0;
}

void AmberExtTrigger(struct t2*input,int nb) { // it's ugly
  for (int i=0;i<nb;i++) {
    if (input[i].energy==5) {
cout << "Found an external trigger from "<< input[i].ID <<endl;
    uint8_t header_udp[4];

    uint16_t am_payload_size_udp;
    uint32_t trigamb_udp;
    uint8_t nbtrigamb_udp;
    uint32_t refSec_udp, refuSec_udp, tv_sec_udp, tv_usec_udp;

    uint16_t station_id_udp;
    uint32_t station_usec_udp;

    int nbtrigamb=1;

    // how big is the datagram?
    int am_payload_size = (1+1+1+1+2+4+4+4+4+4+1+6*nbtrigamb);

    char* am_datagram = new char[am_payload_size];
    char* am_datagram_curr = am_datagram;

    // start assembling datagram, convert to fixed width variables

    const char *header_pre = "!T3!";

    for(int j = 0;j<4;j++) {
      header_udp[j] = (uint8_t) header_pre[j];
      INSERT_UDP(header_udp[j],am_datagram_curr);
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);

    am_payload_size_udp = (uint16_t) am_payload_size;
    trigamb_udp = (uint32_t) trigamb++;
    refSec_udp = (uint32_t) second_;
    refuSec_udp = (uint32_t) input[i].time;
    tv_sec_udp = (uint32_t) tv.tv_sec;
    tv_usec_udp = (uint32_t) tv.tv_usec;
    nbtrigamb_udp = (uint8_t) nbtrigamb;

    INSERT_UDP(am_payload_size_udp,am_datagram_curr);
    INSERT_UDP(trigamb_udp,am_datagram_curr);
    INSERT_UDP(refSec_udp,am_datagram_curr);
    INSERT_UDP(refuSec_udp,am_datagram_curr);
    INSERT_UDP(tv_sec_udp,am_datagram_curr);
    INSERT_UDP(tv_usec_udp,am_datagram_curr);
    INSERT_UDP(nbtrigamb_udp,am_datagram_curr);

      station_id_udp = (uint16_t)input[i].ID;
      station_usec_udp = 0;
      INSERT_UDP(station_id_udp,am_datagram_curr);
      INSERT_UDP(station_usec_udp,am_datagram_curr);

    /* Send the packet */
    XbFast_udp_send(am_datagram,am_payload_size);
     
    delete [] am_datagram;
     
    }
  }
}

#ifdef SATUR

void SaturationTrigger(struct t2*input,int nb) {
  for (int i=0;i<nb;i++) {
    if (input[i].energy==8 && input[i].ID==101) {
        IkT3 ikm;
        ikm.id=XbIkGetId();
        ikm.refSecond=second_;
        ikm.refuSecond=input[i].time;
        ikm.mode="LIST";
        ikm.algo="OBSERVER";
        ikm.SDPAngle=999.;
        ikm.addresses.push_back(input[i].ID);
        ikm.offsets.push_back(0);
        ikm.window.push_back(0);
        ikm.send("Pm");
    }
  }
}
#endif

