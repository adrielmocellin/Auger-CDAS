#include <fstream>
#include <arpa/inet.h>
#include <signal.h>
#include <netinet/in.h> 
#include <dirent.h>
#include <errno.h> 
#include <memory>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "EbEvent.h"
#include "EbManager.h"

#include "T3_uub.h"

#include <Pm.h>
#include <IkC.h>
#include <IkInfo.hxx>
#include <IkWarning.hxx>
#include <IkSevere.hxx>
#include <IkFatal.hxx>
#include <IkSDRequest.hxx>
#include <central_local.h>

// BOOST bzip2
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/write.hpp>

// tmp XB
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
#include <unistd.h>

#define EBMAXSAMPLE 1024
/*-----------------------------------------------------------------------*/ 
/* Global variables                                                      */
/*-----------------------------------------------------------------------*/ 
extern int gDebug, gVerbose, gSuperVerbose;
extern string gXbName;
unsigned long gCurrentEventNumber = 0;
string gT3Sender;
DbCStation *gDBStations;
int gDBNbStations;
IoSdHeader gHeader;
/*-----------------------------------------------------------------------*/ 


static void mkdirs (char *str) {
  char * buf, *tmp;
  tmp=strdup(str);
  buf=tmp;
  while(++tmp=strchr(tmp,'/')) 
    {
      *tmp='\0';
      mkdir(buf,7*8*8+7*8+7);
      *tmp='/';
    }
  free(tmp);
}
/*-----------------------------------------------------------------------*/ 
static void do_cleanup (int code)
/*-----------------------------------------------------------------------*/ 
{
  IkInfoSend ("Closing output file.");
  TheEbManager()->ResetIo("terminate");
  IkFatalSend ("Terminating with code %d.", code);

  exit (0);
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* ! This instanciate a unique EbManager which can be called as          */
/* TheEbManager().                                                       */
EbManager* EbManager::_instance = new EbManager ();
EbManager* EbManager::Instance()
/*-----------------------------------------------------------------------*/ 
{
  return _instance; 
}
/*-----------------------------------------------------------------------*/ 
EbManager* TheEbManager() 
{ 
  return EbManager::Instance();
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
EbManager::EbManager ()
/*-----------------------------------------------------------------------*/ 
{
  _PmServer = NULL;
}
/*-----------------------------------------------------------------------*/ 

const int DATALTH = 10;
/*-----------------------------------------------------------------------*/ 
/* It's used by the Gnc protocol.                                        */
int EbPmInit(CDASSERVER *server) 
/*-----------------------------------------------------------------------*/
{
  longWord metaData[100];
  int nbytes;
  int rtn;

  /* first we must create the data */
  metaData[0] = htonl(1);                  /* service version */
  metaData[1] = htonl(1);                  /* # services  */
  metaData[2] = htonl(PmEBID);               /* service  */
  metaData[3] = htonl(0);                  /* #params  */
  rtn = GncWrite(server, metaData, 16, PmEBID,&nbytes);
  if (rtn == CDAS_SUCCESS) {
    int rtn; 
    longWord fromPmData[DATALTH];
    GsStdHeader *header;
    if ((rtn = GncRead(server, (void *)fromPmData,
		       DATALTH*sizeof(longWord),&nbytes)) == CDAS_SUCCESS) {
      header = GncGetHeaderLastRead();
      IkInfoSend ("Connected to Pm.");
    } else
      IkWarningSend( "Problem with reading IAmEb reply");
  }
  else IkWarningSend("Problem with sending IAmEb");
  
  return rtn;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
cdasErrors EbManager::ConnectToPm (int port, char* addr)
/*-----------------------------------------------------------------------*/ 
{
  _Port = port; strcpy (_Addr, addr);
  if ( GncInit("Pm", PmEBID, addr, port, (int (*)(CDASSERVER *srvr))EbPmInit,
	       NULL, &_PmServer) != CDAS_SUCCESS)
    return CDAS_CONNECT_ERROR;

  _PmSocket = _PmServer->fd;
  return CDAS_SUCCESS;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void EbManager::ReconnectToPm ()
/*-----------------------------------------------------------------------*/ 
{
  GncClose (_PmServer); free (_PmServer); _PmServer = NULL;
  while ( GncInit("Pm", PmEBID, _Addr, _Port, (int (*)(CDASSERVER *srvr))EbPmInit,
		  NULL, &_PmServer) != CDAS_SUCCESS )
    {
      sleep (20); IkMessageCheck();      
    }

  _PmSocket = _PmServer->fd;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* This function initialyses the event identifier, and fixes some varia- */
/* bles to 0.                                                            */
void EbManager::Init() 
/*-----------------------------------------------------------------------*/ 
{
  /* Read last Id event */
  ReadId();

  /* Read eventually last Id event from temporary files, and send these EventList. */
  if (gDebug || gVerbose) {
    stringstream s; s << "New run, CurrentEventNumber: " << gCurrentEventNumber;
    InternalLog(s.str().c_str(),IKDEBUG);
  }
   
  /* Initialisation of some variables. */
  _CurrentTrigger = NULL;
  _CurrentStation = NULL;
  IkPingValue = 0;
  _T3IdInCurrentStation = _T3PreviousId = 0;

  /* coordinates */
  gDBStations = DbCArrayInit();
  gDBNbStations = DbCArrayGetNbStat(gDBStations);

  /* output */
  BuildRootFileName();
  mkdirs(_RootFileName);
  gHeader.StartOfRun = (unsigned int)( time ((time_t *)NULL) );
  if ( !(_Io = new IoSd(_RootFileName, "w")) ) {
    IkSevereSend("Can't open output...");
  }

  /* Signal handler */
  signal(SIGTERM, do_cleanup);
  signal(SIGQUIT, do_cleanup);
  //signal(SIGABRT, do_cleanup); 
  signal(SIGUSR1, do_cleanup);
  signal(SIGUSR2, do_cleanup);
  signal(SIGINT, do_cleanup);
  signal(SIGHUP, do_cleanup);

}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/
/* It builds the root file name using the current time of the day.       */
void EbManager::BuildRootFileName() 
/*-----------------------------------------------------------------------*/
{
  time_t current_time = time ((time_t *)NULL);
  struct tm *ptms = gmtime (&current_time);
  char filename[MAXFILENAMESIZE];

  _OpenFileTime = current_time;
  snprintf(filename,MAXFILENAMESIZE,"sd_%d_%.2d_%.2d_%.2dh%.2d.root",
	   ptms->tm_year+1900,ptms->tm_mon+1,ptms->tm_mday,ptms->tm_hour,ptms->tm_min);
  snprintf(_RootFileNameForRename,MAXFILENAMESIZE,"%s%d/%.2d/%s",DataFilePath,ptms->tm_year+1900,
	   ptms->tm_mon+1,filename);
  snprintf(_RootFileName,MAXFILENAMESIZE,"%s.nobackup",_RootFileNameForRename);
}
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/ 
/* This function reads the last event identifier.                        */
void EbManager::ReadId() 
/*-----------------------------------------------------------------------*/ 
{
  if (UseRealIDs) {
    ifstream rdr("/Raid/var/Eb");
    rdr >> gCurrentEventNumber;
    rdr.close();
  } else gCurrentEventNumber=1;
  if ( !gCurrentEventNumber ) {
    struct dirent **namelist;
    int n;
    time_t current_time = time ((time_t *)NULL);
    struct tm *ptms = gmtime (&current_time);
    char fname[256];
    snprintf(fname,256,"%s%d/%.2d/",DataFilePath,ptms->tm_year+1900,ptms->tm_mon+1);
    n = scandir(fname, &namelist, 0, alphasort);
    if (n>0) {
      strcat(fname, namelist[--n]->d_name);
      IoSd input(fname);
      list<int> ids;
      EventPos pos;
      for (pos=input.FirstEvent(); pos<input.LastEvent(); pos++) {
	IoSdEvent event(pos);
	ids.push_back(event.Id);
      }
      ids.sort();
      list<int>::iterator it = ids.end();
      gCurrentEventNumber = *(--it);
    }
    if ( !gCurrentEventNumber ) {
      IkSevereSend("Eb is unable to read last Id event. Default is 1!!!");
      gCurrentEventNumber = 0;
    }
  }

}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* This function receives new T3 message from Central Trigger via        */
/* IkMessageCheck() - which interrupts the main loop - and returns it.   */
//IkT3* EbManager::CheckNewT3FromCt() 
void EbManager::CheckNewT3FromCt() 
/*-----------------------------------------------------------------------*/ 
{
//  if (_CurrentTrigger != NULL) ClearNewT3Trigger();
  static struct timeval tv1,tv2;
  char msg_aux[100];
  int dt;
  gettimeofday(&tv1,NULL); 
  IkMessageCheck();
  gettimeofday(&tv2,NULL); 
  dt=(tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);
  if(dt>100000) {
    snprintf(msg_aux,100,"dfbdf: %d ",dt);
    InternalLog(msg_aux,IKINFO);
    cerr<<"pb:"<<(tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec)<<endl;
  }

// return _CurrentTrigger;  
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void EbManager::SaveEventNumber ()
/*-----------------------------------------------------------------------*/ 
{
  ofstream wtr("/Raid/var/Eb");
  wtr << gCurrentEventNumber;
  wtr.close();
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* This function receives a trigger and pushes it in a buffer.           */
/* It's called by EbIkHandler() via CheckNewT3().                        */
void EbManager::AddIkT3(IkT3 *ikm) 
/*-----------------------------------------------------------------------*/ 
{
  if ( ikm->get_sender() == gEbName ) return;
  SetNewT3Trigger(ikm);
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* This function does what its name says.                                */
void EbManager::ClearNewT3Trigger() 
/*-----------------------------------------------------------------------*/ 
{
//  delete _CurrentTrigger;
//  _CurrentTrigger = NULL;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* This function does what its name says.                                */
void EbManager::SetNewT3Trigger(IkT3 *ikm) 
/*-----------------------------------------------------------------------*/ 
{
  IkT3* currentTrigger = new IkT3();
  gCurrentEventNumber++;
  if (UseRealIDs) SaveEventNumber ();

  currentTrigger->id = ikm->id;
  currentTrigger->refSecond = ikm->refSecond;
  currentTrigger->refuSecond = ikm->refuSecond;
  currentTrigger->mode = ikm->mode;
  currentTrigger->algo = ikm->algo;
  currentTrigger->SDPAngle = ikm->SDPAngle;
  for (unsigned int i=0; i<ikm->addresses.size(); i++) {
    currentTrigger->addresses.push_back(ikm->addresses[i]);
    currentTrigger->window.push_back(ikm->window[i]);
    currentTrigger->offsets.push_back(ikm->offsets[i]);
  }
  gT3Sender = ikm->get_sender();
  int orph=IsEventOrphanOfTrigger(currentTrigger);
  if ( gDebug || gVerbose ) {
    stringstream s; time_t t=time(NULL); char *ct=ctime(&t); 
    string toto; toto.assign(ct,strlen(ct)-1); 
    s << "Received new trigger, #" << currentTrigger->id <<" orphan: "<<orph << " " << toto;
    InternalLog(s.str().c_str(),IKDEBUG);
  }
  if ( !orph )
    {
      /* Builds an event with the trigger */
      EbEvent Event(currentTrigger);
      /* Pushes this event in event's list */
      EventList.push_back(Event);
      IkInfoSend ("T3 #%u received, starting event %u",currentTrigger->id,Event.Id);
    }
delete currentTrigger;
}

/*-----------------------------------------------------------------------*/ 

const int BUFDATASIZE = 100000;
/*-----------------------------------------------------------------------*/ 
/* As its name says.                                                     */
cdasErrors EbManager::CheckNewT3FromPm ()
/*-----------------------------------------------------------------------*/ 
{
  int nb_bytes;
  struct timeval timeout;

  timeout.tv_sec = 0; timeout.tv_usec = 5000;
  FD_ZERO (&_Global_Fd);
  FD_SET (_PmSocket, &_Global_Fd);

  _Listen = select (_PmSocket+1, &_Global_Fd, NULL, NULL, &timeout);

  if ( _Listen == -1 && errno == EBADF ) {
    ReconnectToPm ();
    //      IkSDRequest ikm; ikm.send (gXbName);
    return CDAS_NO_SERVER;
  } else if ( _Listen == 0 ) { // no data available
    return CDAS_NO_DATA;
  } else if ( _Listen > 0 && !(FD_ISSET (_PmSocket, &_Global_Fd)) ) {
    IkFatalSend ("Reboot computers...");
    _Io->Close();
    exit (1);
  }

  union {int ibuf[BUFDATASIZE];char cbuf[BUFDATASIZE*sizeof(int)];} data;

  struct timeval tv1,tv2;
  gettimeofday(&tv1,NULL);
  if ( GncRead (_PmServer, data.ibuf, BUFDATASIZE, &nb_bytes) != CDAS_SUCCESS ) {
    // error during the read
    IkSevereSend("Error during the read of data, trying to reconnect to Pm.");
    ReconnectToPm ();
    //      IkSDRequest ikm; ikm.send (gXbName);
    return CDAS_READ_ERROR;
  }
  gettimeofday(&tv2,NULL);
  int dt;
  dt=(tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);
  if(dt>50000) {
    char msg_aux[100];
    snprintf(msg_aux,100,"pb GncRead... %d\n",dt);
    InternalLog(msg_aux,IKINFO);
    cerr<<"pb-GncRead:"<<dt << endl;
  }
 
  int rcvcode = DecodeDataType (data.cbuf);
  int errcode;
  
  delete _CurrentStation;
  _CurrentStation = new IoSdStation();

  switch ( rcvcode ) {
  case M_T3_EVT:
    gettimeofday(&tv1,NULL);
    errcode = FillCurrentStation (data.cbuf, nb_bytes);
    gettimeofday(&tv2,NULL);
    int dt;
    dt=(tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);
    if(dt>50000) {
      char msg_aux[100];
      snprintf(msg_aux,100,"pb FillCurrentStation...%d",dt);
      InternalLog(msg_aux,IKINFO);
      cerr<<"pb-FillCurrentStation:"<<dt << endl;
    }
    break;
    //	      case M_T3_MUON:
    //		break;
  default:
    IkWarningSend("Unknown type message from Pm");
    break;
  }

  return CDAS_SUCCESS;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/
/* As its name says.                                                     */ 
unsigned int EbManager::DecodeDataType(char* buffer) 
/*-----------------------------------------------------------------------*/ 
{
  PmLsRawMessage *rawMessP;
  PmLsMessage *messP;

  rawMessP = (PmLsRawMessage*)(buffer+4); 
  messP = &(rawMessP->mess);
  return ntohl(messP->type);
}
/*-----------------------------------------------------------------------*/ 

#define ERRORMASK 0x3f
#define COMPRESSION 0x100
#define BITMASK 1023
/*-----------------------------------------------------------------------*/
/* As its name says.                                                     */ 
int EbManager::FillCurrentStation(char* buffer, unsigned int nbbytes)
/*-----------------------------------------------------------------------*/ 
{
  PmLsRawMessageP rawMessP;
  PmLsMessageP messP;
  char* cP;
  short* sP;
  longWord* lP;
  unsigned int decode,ret=0;

  rawMessP=(PmLsRawMessageP)(buffer+4); /* point at the beginning of the message,
					   where start the message lenght.
					   there is a shift of 4 bytes to skip
					   the bytes 0x00 0x00 0x00 0x01 */
  /* PmLsRawMessage*/
  _CurrentStation->Id = (unsigned int)ntohl(rawMessP->lsId);
  if (gSuperVerbose) {
    stringstream s; s << "New bloc of data received from # " << _CurrentStation->Id ;
    InternalLog(s.str().c_str(),IKDEBUG);
  }

  /* PmLsMessage */
  messP = &(rawMessP->mess);
  int version; version =ntohl(messP->version);
  int type; type = ntohl(messP->type);
  /* data */
  cP = (char *)(messP->data);

  if ( !cP ) {
    stringstream s; s << "No data from Pm... " ;
    InternalLog(s.str().c_str(),IKINFO);
    return IoSdEvent::eDataLost;
  }

  sP = (short*)cP;
  /* identifiant (global variable) */
  _T3IdInCurrentStation = ntohs(*sP++) & 0x3fff;
  /* error code */
  _CurrentStation->Error = ntohs(*sP++);
  if (_CurrentStation->Error & ERRORMASK) {
    _CurrentStation->Error = _CurrentStation->Error & ERRORMASK;
    if (_CurrentStation->Error == M_T3_ALREADY) _T3PreviousId = ntohs (*sP);
    return _CurrentStation->Error;
  }
  // We have a station with Error==0
  unsigned int EventListize=0;
  // rejecting uncompressed data
  if (!(_CurrentStation->Error & COMPRESSION)) {
    _CurrentStation->Error = IoSdEvent::eBadCompress;
    return _CurrentStation->Error;
  }
    if (gSuperVerbose) {
      stringstream s; s<<"Compressed T3 - Nb bytes:" << nbbytes << 
			" mess size:" << ntohl(messP->length) << 
			" buff size?:" << ntohl(messP->length) - 16;
      InternalLog(s.str().c_str(),IKDEBUG);
    }
//#define USEBUNZIP2ANDDISK
#ifdef USEBUNZIP2ANDDISK
    char fname[100],fzname[102],command[256];
    //snprintf(fname,100,COMPRESSEDFILEPATH);
    snprintf(fname,100,"eb_tmp_evt_%d_%d", _T3IdInCurrentStation, _CurrentStation->Id);
    snprintf(fzname,102,"%s.Z",fname);
    FILE *fp=fopen(fzname,"w");
    for (unsigned int i=0;i+16<ntohl(messP->length);i++)
      fwrite(((unsigned char*)sP)+i,1,1,fp);
    fclose(fp);
    snprintf(command,256,"uncompress -c %s > %s 2>/dev/null",fzname,fname);
    if (system(command)!=0) {
      //      snprintf(command,256,"bzip2 -dc %s > %s",fzname,fname);
      snprintf(command,256,"/Raid/usr/bin/ebzip2 -dc %s > %s",fzname,fname);
      if (system(command)!=0) {
	snprintf(command,256,"mv %s /Raid/var/trash/%s",fzname,fzname);
	system(command);
	IkWarningSend("Invalid Compressed data from station %d", _CurrentStation->Id);
	_CurrentStation->Error = IoSdEvent::eBadCompress;
	return _CurrentStation->Error;
      }
    };
    fp=fopen(fname,"r");
    unsigned char buf[120000];
    while(fread(buf+EventListize,1,1,fp)&&EventListize<100000) { EventListize++; };
    fclose(fp);
    unlink(fname);
    unlink(fzname);
#else // use boost bzip2 library to do operation in memory
    std::istringstream iss(std::ios::binary);
    iss.rdbuf()->pubsetbuf((char*)sP, ntohl(messP->length)-16);

    boost::iostreams::filtering_istreambuf zdat;
    zdat.push(boost::iostreams::bzip2_decompressor());
    zdat.push(iss);

    unsigned char buf[120000];
    try {
      EventListize+=boost::iostreams::read(zdat, (char*)buf, 100000);
    } catch(...) {
      IkWarningSend("Invalid Compressed data from station %d", _CurrentStation->Id);
      _CurrentStation->Error = IoSdEvent::eBadCompress;
      return _CurrentStation->Error;
    }
#endif
    sP=(short*)buf;
    if (gSuperVerbose) {
      stringstream s; s << "New size:" << EventListize;
      InternalLog(s.str().c_str(),IKDEBUG);
    }

  // Note: EventListize for UUB is 41000
  // Calibration histograms are 7284 bytes long
  // extra info add 34x4=136 extra bytes
  // radio traces are 2048x4 long (8192)
  // in the future we will use an internal version
  if(4<EventListize){
    int buff_version;
    buff_version=ntohl(*(uint32_t*)buf);
    //printf("T3 data - version: %d\n",buff_version);
    if(buff_version==262){
      return(T3_uub_v262(_CurrentStation,buf,EventListize));
    } else if(263==buff_version){
      return(T3_uub_v263(_CurrentStation,buf,EventListize));
    } else if( 264==buff_version ||
               (264 < buff_version && buff_version<300) ){
      // The second condition is to include some additional version
      return(T3_uub_v264(_CurrentStation,buf,EventListize));
    }
  }
  if (EventListize==41000 || EventListize==(41000+7284) || EventListize==(41000+7284+136) || EventListize==(41000+7284+136+8192)) {
    // IkInfoSend("Got event T3 data from %d, looking like UUB", _CurrentStation->Id); // No more needed, works now
    // Got a UUB
    // Right now data is scarce:
//struct shwr_gps_info
//{
//  uint32_t second;
//  uint32_t ticks;
//  uint32_t ticks_prev_pps;
//};
//
//struct shwr_evt_raw
//{
//  uint32_t id;                                                        <==== used as a version now
//  uint32_t Evt_type_1; /* primary trigger event type (SB, TOT, ...).
//                          which kind of trigger the event appeared.*/
//  uint32_t Evt_type_2; /* secunday trigger event type (SB, ToT, ...),
//                          Which list of trigger it might be considered */
//  int32_t trace_start; /* The fadc_raw is just a copy the memory block,
//                          but the element ...[0] is not when the ADC 
//                          trace really starts. This parameter
//                          will contain that information.
//                        */
//  struct shwr_gps_info ev_gps_info;
//  int32_t micro_off;
//  int32_t nsamples;
//  uint32_t fadc_raw[SHWR_RAW_NCH_MAX][SHWR_NSAMPLES];
//
//  uint32_t buffer_ev_status;
//};
//
// Then the histograms
// Then:
//  uint32_t next 120MHz clock
//  signed char sawtooth x 4, prev prev, prev, current and next
//
//  unsigned int extra[32]

    _CurrentStation->IsUUB = 1;
    lP = (longWord*)sP;
    //  debug for histograms if needed
    //if ( EventListize==(41000+7284)) {
      //int ftmp=open("/home/cdas/buffer.dat",O_RDWR|O_APPEND);
      //write(ftmp,(void *)lP,41000+7284);
      //close(ftmp);
    //}
    // First word is internal id, used for versioning since version 260
    unsigned int InternalId=ntohl(*lP++);
    _CurrentStation->Calib = new IoSdCalib();
    _CurrentStation->Calib->Version=256; // 256+ are UUB versions
    if (EventListize==(41000+7284)) {
      // calib version 257 with simple histograms
      _CurrentStation->Calib->Version=257;
    }
    if (EventListize==(41000+7284+136)) {
      // calib version 258 with simple histograms and GPS timing
      // _CurrentStation->Calib->Version=258;
      // calib version 259 with all 4 histograms, GPS timing, and offset bug fixed
      _CurrentStation->Calib->Version=259;
    }
    // Since version 260, version is included in InternalId
    // 260 corresponds to code V128R0B0P5, with ToT, and many information in Extra fields
    // 261 corresponds to code V128R0B0P6, with radio data
    if (InternalId>256) _CurrentStation->Calib->Version=InternalId;
    // Then 2 trigger types
    unsigned int type1=ntohl(*lP++);
    unsigned int type2=ntohl(*lP++);
    // Then trace start
    unsigned int tracestart=ntohl(*lP++);
    // GPS basic data
    unsigned int gpssec=ntohl(*lP++);
    unsigned int ticks=ntohl(*lP++);
    unsigned int prevtics=ntohl(*lP++);
    // offset and samples
    unsigned int offset=ntohl(*lP++);
    unsigned int samples=ntohl(*lP++);
    // filling data
    _CurrentStation->Trigger.Type = 0; // For UUB, Type is 0 and Type is in Type1 and 2
    _CurrentStation->Trigger.Type1 = type1;
    _CurrentStation->Trigger.Type2 = type2;
    // UB stuff or unused yet
    _CurrentStation->Fadc = NULL;
    // UUB FADC
    // the trace. It is version 3, mixed with GPS
    _CurrentStation->UFadc = new IoUSdFadc();
    _CurrentStation->UFadc->CodingVersion=3; // only current one
    _CurrentStation->UFadc->TraceStart = tracestart;
    // Time stamp GPS : second, ticks, prevticks (which is 120MHz of prev sec)
    _CurrentStation->Gps = new IoSdGps();
    _CurrentStation->Gps->IsUUB = 1;
    _CurrentStation->Gps->Second = gpssec;
    _CurrentStation->Gps->Tick = ticks;
    _CurrentStation->Gps->TickFall = prevtics;
    // basic GPS done
    // micro offset from Patrick, to the bin for UUB
    // FADC buffer
    _CurrentStation->UFadc->NSample = samples;
    // check samples to 2048, else we could die making a wrong vector
    if (samples!=2048) {
      IkWarningSend("UUB data from station %d with wrong NSample (%d, should be %d)", _CurrentStation->Id,samples,2048);
      _CurrentStation->Error |= IoSdEvent::eBadCompress;
      return _CurrentStation->Error;
    }
    // now dump the new array
    int nbval=_CurrentStation->UFadc->NSample*10; // 5 channels, high and low
    _CurrentStation->UFadc->Traces.reserve(nbval);
    sP=(short*)lP;
    _CurrentStation->UFadc->Traces.assign(sP, sP + nbval);
    sP=sP+nbval;
    // BUG: there was one missing long. I do it by 2 sP++, can be replaced by reading the lP...
    sP++;
    sP++;
    // calibration histograms
    if (EventListize==(41000+7284) || EventListize==(41000+7284+136) || EventListize==(41000+7284+136+8192)) {
      _CurrentStation->Histo = new IoSdHisto();
// This is what we get:
//
// struct muon_histo {
//   unsigned short Offset[4];
//   unsigned short Base[4][20];
//   unsigned short Peak[4][150];  // note: this is bitshifted by 2 (ie x4)
//   unsigned short Charge[4][600]; // note: this is bitshifter by 3 (ie x8)
//   int Shape[4][NBBIN]; // note: baseline substracted (can be negative)
//   unsigned int StartSecond;
//   unsigned int EndSecond;
//   unsigned int NEntries;
// };
//
// with NBBIN=69
//
      unsigned short Offset[4];
      for (int i=0;i<4;i++) Offset[i]=ntohs(*sP++);
      // got the baselines used for all 4 channels: PMT 1-3 and SSD
      for (int i=0;i<3;i++) {
        _CurrentStation->Histo->Offset[i]=Offset[i]; // baselines
        _CurrentStation->Histo->Offset[i+3]=Offset[i]; // peak
        _CurrentStation->Histo->Offset[i+6]=69*Offset[i]; // charge, 69 bins
      }
      _CurrentStation->Histo->Offset[9]=69*Offset[3]; // charge SSD histogram, old, actually is incorrect since SSD charge isn't computed on 69 bins
      // setting SSD histograms offset values;
      _CurrentStation->Histo->Offset3[0]=Offset[3];
      _CurrentStation->Histo->Offset3[1]=Offset[3];
      _CurrentStation->Histo->Offset3[2]=Offset[3]*20;
      for (unsigned int i=0;i<3;i++)  for (unsigned int j=0;j<20;j++) _CurrentStation->Histo->Base[i][j] = ntohs(*sP++);
      for (unsigned int j=0;j<20;j++) _CurrentStation->Histo->Base3[j] = ntohs(*sP++); // special treatment of SSD baseline histogram
      for (unsigned int i=0;i<3;i++) for (unsigned int j=0;j<150;j++) _CurrentStation->Histo->Peak[i][j] = ntohs(*sP++);
      for (unsigned int j=0;j<150;j++) _CurrentStation->Histo->Peak3[j] = ntohs(*sP++); // special treatment of SSD peak histogram
      for (unsigned int i=0;i<4;i++) for (unsigned int j=0;j<600;j++) _CurrentStation->Histo->Charge[i][j] = ntohs(*sP++);
      lP=(longWord *)sP;
      for (unsigned int i=0;i<4;i++) for (unsigned int j=0;j<69;j++) _CurrentStation->Histo->UShape[i][j] = ntohl(*lP++);
      _CurrentStation->Calib->StartSecond=ntohl(*lP++);
      _CurrentStation->Calib->EndSecond=ntohl(*lP++);
      _CurrentStation->Calib->NEntries=ntohl(*lP++);
    } else _CurrentStation->Histo = NULL;
    if (EventListize==(41000+7284+136) || EventListize==(41000+7284+136+8192)) {
      // more complete GPS data and extra values
      // for simplicity, the old GPS structure is used
      // the Next100 hosts the Next120
      // and the 2 ST are used as before
      _CurrentStation->Gps->Next100=ntohl(*lP++);
      signed char * sc=(signed char *)lP;
      // keeping prev prev ST in 40MHz temporarily
      _CurrentStation->Gps->Current40=(*sc++);
      _CurrentStation->Gps->PreviousST=(*sc++);
      _CurrentStation->Gps->CurrentST=(*sc++);
      _CurrentStation->Gps->NextST=(*sc++);
      lP=(longWord *)sc;
      // 32 extra unsigned ints
      for (int i=0;i<32;i++) {
        _CurrentStation->Extra[i]=ntohl(*lP++);
      }
    }
    // radio extra
    if (_CurrentStation->Calib->Version>260) {
      if (_CurrentStation->Extra[31] && EventListize==(41000+7284+136+8192)) {
        // one extra trace for radio data
	int nbval=_CurrentStation->UFadc->NSample*12; // 5 channels, high and low + 2 radio channels
	_CurrentStation->UFadc->Traces.reserve(nbval);
        sP=(short *)lP;
        for (int i=0;i<2048*2;i++)
          _CurrentStation->UFadc->Traces.push_back(*sP++);
      }
    }
 // Log debug info
#if 1
  stringstream s;
  s << "#UUB id " << _CurrentStation->Id << " Extra";
  for (int i=0;i<32;i++) s << " " << _CurrentStation->Extra[i];
  //s << "#UUB id " << InternalId << endl << "#UUB Wrote types " << _CurrentStation->Trigger.Type1 << " " << _CurrentStation->Trigger.Type2 << endl;
  //s << "#UUB TraceStart " << _CurrentStation->UFadc->TraceStart << endl;
  //s << "#UUB Wrote GPS " << _CurrentStation->Gps->Second << " " << _CurrentStation->Gps->Tick << " " << _CurrentStation->Gps->TickFall << endl;
  //s << "#UUB Got Offset " << offset << " NSample " << _CurrentStation->UFadc->NSample << " ie nbval " << nbval << endl;
  //s << "#UUB Vector size: " << _CurrentStation->UFadc->Traces.size() << " and first 10 values:";
  //for (int i=0;i<10;i++) s << " " << _CurrentStation->UFadc->Traces[i];
  //s << endl;
  InternalLog(s.str().c_str(),IKDEBUG);
#endif
 // end Log debug
    return ret;
  }
  // Note: EventListize for PP tanks -> March 2004 is 14442
  if (EventListize>14442) {
    IkWarningSend("Huge Compressed data from station %d (%d bytes, bigger than 14442)", _CurrentStation->Id,EventListize);
    _CurrentStation->Error = IoSdEvent::eBadCompress;
    return _CurrentStation->Error;
  }
  // We have a UB Station
  _CurrentStation->IsUUB = 0;
  _CurrentStation->UFadc = NULL;
  // End of UB only code
  _CurrentStation->Error = _CurrentStation->Error & ERRORMASK;
  _CurrentStation->Error = _CurrentStation->Error & ERRORMASK;
  // instances
  _CurrentStation->Gps = new IoSdGps();
  _CurrentStation->Histo = new IoSdHisto();
  _CurrentStation->Calib = new IoSdCalib();
  _CurrentStation->Fadc = new IoSdFadc();

  _CurrentStation->Gps->IsUUB = 0;

  lP = (longWord*)sP;
  /* Time stamp GPS : second */
  _CurrentStation->Gps->Second = ntohl(*lP++);
  /* Time stamp GPS : nano */
  _CurrentStation->Gps->Tick = ntohl(*lP++);
//  if (_CurrentStation->Id>=71 && EventListize!=14414) {// berk!!! (pire qu'en dessous)
  if (_CurrentStation->Id>=11 && EventListize!=14414) {// berk!!! (pire qu'en dessous) - to include the AERA stations (2012/Dec/11)
    lP++; // second, I don't care
    _CurrentStation->Gps->TickFall = ntohl(*lP++);
  }
  /* Time correction from 100Mhz->us convertion in LS */
  short TimeCorrect = 0;
//  if (_CurrentStation->Id>=71) // berk!!!
  if (_CurrentStation->Id>=11) // berk!!! - to include the AERA stations (2012/Dec/11)
    TimeCorrect = (short) ntohl(*lP++);
  sP = (short*)lP;
  /* Trigger status */
  _CurrentStation->Trigger.Type = ntohs(*sP++);	
  switch(_CurrentStation->Trigger.Type&0x1f) {
  case 1: _CurrentStation->Trigger.Name = "Threshold"; break;
  case 2: _CurrentStation->Trigger.Name = "Time Over Threshold"; break;
  case 16: _CurrentStation->Trigger.Name = "External Trigger"; break;
  case 25: _CurrentStation->Trigger.Name = "Muon Trigger"; break; // For EA muon EventList only
  default: _CurrentStation->Trigger.Name = "Unknown Trigger (?)"; break;
  }
  /* number of following samples */
  _CurrentStation->Fadc->NSample = ntohs(*sP++); 
  lP = (longWord*)sP;
  /* FADC samples */
  // WARNING !!!!!! Station is sending 2x EBMAXSAMPLE... That's why we have
  // to make some acrobatics.
  /* FADC samples */
  for (unsigned int i=0;i<EBMAXSAMPLE;i++) { // FIXME should be Samples but #$P#$& LS soft does not give it right yet.  
//    if (i==768 && _CurrentStation->Id>=71) break;
    if (i==768 && _CurrentStation->Id>=11) break; //to include AERA stations (2012/Dec/11)
    decode = ntohl(*lP++);
    if (i<kIoSd::MAXSAMPLE) {
      _CurrentStation->Fadc->Trace[0][IoSdEvent::eHigh][i] = decode & BITMASK;
      _CurrentStation->Fadc->Trace[1][IoSdEvent::eHigh][i] = decode >> 10 & BITMASK;
      _CurrentStation->Fadc->Trace[2][IoSdEvent::eHigh][i] = decode >> 20 & BITMASK;
    }
//    if (_CurrentStation->Id<71) {
    if (_CurrentStation->Id<11) { // to include the AERA stations (2012/Dec/11)
      decode = ntohl(*lP++);
      if (i<kIoSd::MAXSAMPLE) {
        _CurrentStation->Fadc->Trace[0][IoSdEvent::eLow][i] = decode & BITMASK;
        _CurrentStation->Fadc->Trace[1][IoSdEvent::eLow][i] = decode >> 10 & BITMASK;
        _CurrentStation->Fadc->Trace[2][IoSdEvent::eLow][i] = decode >> 20 & BITMASK;
      }
    }
  }
//  if (_CurrentStation->Id>=71) {
  if (_CurrentStation->Id>=11) {// to include the AERA stations (2012/Dec/11) 
    for (unsigned int i=0;i<EBMAXSAMPLE;i++) { // FIXME should be Samples but #$P#$& LS soft does not give it right yet. 
      decode = ntohl(*lP++);
      if (i<kIoSd::MAXSAMPLE) {
        _CurrentStation->Fadc->Trace[0][IoSdEvent::eLow][i] = decode & BITMASK;
        _CurrentStation->Fadc->Trace[1][IoSdEvent::eLow][i] = decode >> 10 & BITMASK;
        _CurrentStation->Fadc->Trace[2][IoSdEvent::eLow][i] = decode >> 20 & BITMASK;
      }
    }
    lP+=EBMAXSAMPLE-768;
  }

  if (version==1) {
    int calsize=ntohl(*lP++);
    if (gSuperVerbose) {
      stringstream s; s << "Cal size:" << calsize;
      InternalLog(s.str().c_str(),IKDEBUG);
    }
    //    if (calsize!=1208 && calsize != 132 && calsize != 108) {
    // rustine...
//    if (_CurrentStation->Id>=71 && EventListize!=14414 && calsize==0) calsize=ntohl(*lP++);
    if (_CurrentStation->Id>=11 && EventListize!=14414 && calsize==0) calsize=ntohl(*lP++); //to include AERA stations (2012/Dec/11)
    if (calsize!=84 && calsize!=104) { // Padded to 4!!!!
      //      IkWarningSend ("Invalid Calib data from station %d",LsId);
      // we have an unknown data block. Don't try to be clever, forget
      // everything that comes after
      if (calsize!=0) {
        _CurrentStation->Error=IoSdEvent::eBadCompress;
        return IoSdEvent::eBadCompress;
      }
    } else {
      sP = (short *)lP;
      _CurrentStation->Calib->Version = ntohs(*sP++);
      _CurrentStation->Calib->TubeMask = ntohs(*sP++);
      lP = (longWord *)sP;
      _CurrentStation->Calib->StartSecond = ntohl(*lP++);
      _CurrentStation->Calib->EndSecond = ntohl(*lP++);
      sP = (short *)lP;
      _CurrentStation->Calib->NbT1 = ntohs(*sP++);
      _CurrentStation->Calib->NbT2 = ntohs(*sP++);
      for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->Evolution[j] = ntohs(*sP++);
      for (unsigned int j=0; j<2*kIoSd::NPMT; j++) _CurrentStation->Calib->Base[j] = ntohs(*sP++)/100.;
      for (unsigned int j=0; j<2*kIoSd::NPMT; j++) _CurrentStation->Calib->SigmaBase[j] = ntohs(*sP++)/100.;
      for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->VemPeak[j] = ntohs(*sP++)/10.;
      for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->Rate[j] = ntohs(*sP++)/100.;
      for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->NbTDA[j] = ntohs(*sP++);
      for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->DA[j] = ntohs(*sP++)/100.;
      for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->SigmaDA[j] = ntohs(*sP++)/100.;
      for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->VemCharge[j] = ntohs(*sP++)/10.;
      _CurrentStation->Calib->NbTOT = ntohs(*sP++); 
      if (calsize==104) {
        for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->DADt[j] = ntohs(*sP++)/100.;
        for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->SigmaDADt[j] = ntohs(*sP++)/100.;
        for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->DAChi2[j] = ntohs(*sP++)/100.;
	sP++; // (we have 9 shorts. To int-pad it, we need a 10th short.
      } else {
        for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->DADt[j] = 0;
        for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->SigmaDADt[j] = 0;
        for (unsigned int j=0; j<kIoSd::NPMT; j++) _CurrentStation->Calib->DAChi2[j] = 0;
      }
    }

    // TO BE CHECKED !!!  => Checked, 2 seg faults, now OK :)
    lP = (longWord *)sP;

    int monsize=ntohl(*lP++);
    if (gSuperVerbose) {
      stringstream s; s << "Mon size:" << monsize;
      InternalLog(s.str().c_str(),IKDEBUG);
    }
    if (monsize!=6080) {
      if (monsize!=0) {
        _CurrentStation->Error=IoSdEvent::eBadCompress;
        return IoSdEvent::eBadCompress;
      }
    } else {
      sP=(short *)lP;
      for (unsigned int i=0;i<kIoSd::NB_HISTO_CALIB;i++) _CurrentStation->Histo->Offset[i] = ntohs(*sP++);
      for (unsigned int i=0;i<kIoSd::NPMT;i++)  for (unsigned int j=0;j<20;j++) _CurrentStation->Histo->Base[i][j] = ntohs(*sP++);
      for (unsigned int i=0;i<kIoSd::NPMT;i++) for (unsigned int j=0;j<150;j++) _CurrentStation->Histo->Peak[i][j] = ntohs(*sP++);
      for (unsigned int i=0;i<kIoSd::NPMT+1;i++) for (unsigned int j=0;j<600;j++) _CurrentStation->Histo->Charge[i][j] = ntohs(*sP++);
      lP=(longWord *)sP;
      for (unsigned int i=0;i<kIoSd::NPMT;i++) for (unsigned int j=0;j<kIoSd::SINGLE_MUON_SIZE;j++) _CurrentStation->Histo->Shape[i][j] = ntohl(*lP++);
    }
    int gpssize=ntohl(*lP++);
    if (gSuperVerbose) {
      stringstream s; s << "GPS size:" << gpssize;
      InternalLog(s.str().c_str(),IKDEBUG);      
    }
    if (gpssize!=28 && gpssize!=30) {
      if (gDebug) {
	stringstream s; s << "Invalid GPS data";
	InternalLog(s.str().c_str(),IKDEBUG);
      }
      //	  IkWarningSend("Invalid GPS data from station %d",LsId);
    } else {
      // Read here GPS information.
      _CurrentStation->Gps->Current100=ntohl(*lP++);
      _CurrentStation->Gps->Next100=ntohl(*lP++);
      _CurrentStation->Gps->Current40=ntohl(*lP++);
      _CurrentStation->Gps->Next40=ntohl(*lP++);
      _CurrentStation->Gps->PreviousST=ntohl(*lP++);
      _CurrentStation->Gps->CurrentST=ntohl(*lP++);
      _CurrentStation->Gps->NextST=ntohl(*lP++);
//      if (_CurrentStation->Id>=71) // berk!!!
      if(_CurrentStation->Id>=11) // berk!!! - to include AERA stations (2012/Dec/11)
	{
	  sP=(short*)lP;
	  short test=ntohs(*sP);
	  _CurrentStation->Gps->Offset=((unsigned short)ntohs(*sP++)) + (TimeCorrect<<16);
	  if (gSuperVerbose) {
	    stringstream s; s << "time correct "<<TimeCorrect << " " << test
	   		    << "Ok?: " << (short)(_CurrentStation->Gps->Offset>>16) << " " 
	  		    << (short)(_CurrentStation->Gps->Offset & 0xffff);
	    InternalLog(s.str().c_str(),IKDEBUG);
	  }	
	}
    }
  } else {
    stringstream s;
    s << "ERROR: SD Internal version is not compatible. It is expect to be 1 and received: "<<version;
    InternalLog(s.str().c_str(),IKDEBUG);
  }
  
  if (gSuperVerbose) {
    stringstream s; s << "Data OK from # " << _CurrentStation->Id << " Time: " 
		      << _CurrentStation->Gps->Second << "." << _CurrentStation->Gps->NanoSecond;
    InternalLog(s.str().c_str(),IKDEBUG);
  }
  
  return ret;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
int EbManager::IsEventOrphanOfTrigger (IkT3 *ikm)
/*-----------------------------------------------------------------------*/ 
{
  list<EbEvent>::iterator it;
  if ( EventList.size () ) {
    it = find_if (EventList.begin(), EventList.end(), EbEvent_nb(ikm->id));
    if ( it != EventList.end () ) {
    //  if (gDBNbStations<(int)ikm->addresses.size()) {
    //	gDBStations = DbCArrayInit();
    //	gDBNbStations = DbCArrayGetNbStat(gDBStations);
    //  }
      
      for(unsigned int i=0;i<ikm->addresses.size();i++) {
        int ii=0;
        for(ii=0;ii<gDBNbStations;ii++) {
          if(ikm->addresses[i]==gDBStations[ii].Id) break;
        }
        if(ii==gDBNbStations) { // station not found => reload
          gDBStations = DbCArrayInit();
          gDBNbStations = DbCArrayGetNbStat(gDBStations);
          break; // useless to continue
        }
      }

      it->Id = gCurrentEventNumber;
      it->Trigger.Id = ikm->id;
      it->Trigger.Sender = gT3Sender;
      it->Trigger.NumberOfStation = ikm->addresses.size();
      it->Trigger.Second = ikm->refSecond;
      it->Trigger.MicroSecond = ikm->refuSecond;
      it->Trigger.SDPAngle = ikm->SDPAngle;
      it->Trigger.Algo = ikm->algo;
      unsigned int sizei=it->Trigger.NumberOfStation;
      for (unsigned int i=0; i<sizei; i++) {
	int flag=1; // les données sont-elles déjà arrivées pour cette ls ?
	for (unsigned int j=0; j<it->Stations.size(); j++) {
	  if ((unsigned int)ikm->addresses[i]==it->Stations[j].Id) {
	    for (int k=0; k<gDBNbStations; k++) if (it->Stations[j].Id == (unsigned int)gDBStations[k].Id) {
	      it->Stations[j].Easting = gDBStations[k].Easting;
	      it->Stations[j].Northing = gDBStations[k].Northing;
	      it->Stations[j].Altitude = gDBStations[k].Altitude;
	      it->Stations[j].Name = gDBStations[k].Name;
	      break;
	    }
	    it->Stations[j].Trigger.Offset = ikm->offsets[i];
	    it->Stations[j].Trigger.Window = ikm->window[i];	  
	    flag=0; break;
	  }
	}
	if (flag) {
	  IoSdStation stat;
	  stat.Id = ikm->addresses[i];
	  stat.Error = IoSdEvent::eDataLost;
	  stat.Trigger.Offset = ikm->offsets[i];
	  stat.Trigger.Window = ikm->window[i];
	  for (int k=0; k<gDBNbStations; k++) if (stat.Id == (unsigned int)gDBStations[k].Id) {
	    stat.Easting = gDBStations[k].Easting;
	    stat.Northing = gDBStations[k].Northing;
	    stat.Altitude = gDBStations[k].Altitude;
	    stat.Name = gDBStations[k].Name;
	    break;
	  }
	  it->Stations.push_back(stat);
	}
      }
      
      return 1;
    } // fin if(it)
  }

  return 0;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* This function pushes each new data at its place in the main list.     */
void EbManager::PushDataInEventsList() 
/*-----------------------------------------------------------------------*/ 
{
  EbEvent *event = NULL;

  /* check the id of data */
  event = Find_EbEvent_ByTriggerId ();
  if ( !(event) ) {
    EbEvent e(_CurrentStation, _T3IdInCurrentStation);
    e.NumberOfErrorZeroStation=0;
    e.UNumberOfErrorZeroStation=0;
    if ( !_CurrentStation->Error  )
      e.NumberOfErrorZeroStation++;
    else if( (_CurrentStation->Error & ERRORMASK) == 0){
      e.UNumberOfErrorZeroStation++;
    }
    EventList.push_back(e);
    if (gDebug || gVerbose) {
      stringstream s; s<< "Pushing data of LsId = " << _CurrentStation->Id 
		       << " for event not yet labeled, t3Id # " <<_T3IdInCurrentStation;
      InternalLog(s.str().c_str(),IKDEBUG);
    }
    return;
  }

  /* good data , push in list */
  if ( PushStationInEvent (event) == -1 ) return;
  if (gSuperVerbose) {
    stringstream s; s<< "Pushing data of LsId = " << _CurrentStation->Id 
		     << " for event " << event->Id;
    InternalLog(s.str().c_str(),IKDEBUG);
  }
  if (_CurrentStation->IsUUB) {
    stringstream s; s<< "Pushing data of UUB " << _CurrentStation->Id 
		     << " with extra[0] " << _CurrentStation->Extra[0] ;
    InternalLog(s.str().c_str(),IKDEBUG);
  }
  if ( !(_CurrentStation->Error ))
    event->NumberOfErrorZeroStation++;
  else if( (_CurrentStation->Error & ERRORMASK) == 0){
    event->UNumberOfErrorZeroStation++;
  }
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
EbEvent *EbManager::Find_EbEvent_ByTriggerId ()
/*-----------------------------------------------------------------------*/ 
{
  list<EbEvent>::iterator it;

  if ( EventList.size () ) {
    it = find_if (EventList.begin(), EventList.end(), EbEvent_nb(_T3IdInCurrentStation));
    if ( it != EventList.end () ) return &(*it);
    else return NULL;
  }
  
  /*IkWarningSend ("Data from Pm without trigger from Ct, ls %d, t3Id %d.", 
		 _CurrentStation->Id, _T3IdInCurrentStation);*/
  if (gDebug || gVerbose) {
    stringstream s; s<< "Data from Pm without trigger from Ct, ls " << _CurrentStation->Id 
		     << " t3Id " << _T3IdInCurrentStation;
    InternalLog(s.str().c_str(),IKDEBUG);
  }

  return NULL;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
int EbManager::PushStationInEvent (EbEvent* event)
/*
 * We must take care to the case of the same station appearing twice 
 * in the same event, cf bugzilla #82
 */
/*-----------------------------------------------------------------------*/ 
{
  int flag = 1; // trigger not yet received
  unsigned int size = event->Stations.size();
  for (unsigned int i=0; i<size; i++) if (event->Stations[i].Id == _CurrentStation->Id
		  		      &&  event->Stations[i].Fadc==NULL) {//cf bug #82
    if (event->Stations[i].Error == IoSdEvent::eDataLost) { // cf bugzilla #49
      _CurrentStation->Easting = event->Stations[i].Easting;
      _CurrentStation->Northing = event->Stations[i].Northing;
      _CurrentStation->Altitude = event->Stations[i].Altitude;
      _CurrentStation->Name = event->Stations[i].Name;
      _CurrentStation->Trigger.Offset = event->Stations[i].Trigger.Offset;
      _CurrentStation->Trigger.Window = event->Stations[i].Trigger.Window;
      event->Stations[i] = *_CurrentStation;
      if (_T3PreviousId != 0) {
	event->Trigger.PreviousId = _T3PreviousId;
	_T3PreviousId = 0;
      }
    }
    flag = 0;
  }

  if (flag) {
    event->Stations.push_back(*_CurrentStation);
  } 

  return 0;
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* This function searches finished EventList in the main list, fills them   */
/* in a root file and deletes them from the list.                        */
void EbManager::SearchEventsFinished()
/*-----------------------------------------------------------------------*/ 
{
  if ( !(EventList.size ()) ) return;

  list<EbEvent>::iterator it,stop;
  it = EventList.begin(); stop = EventList.end();

  while ( it != stop ) { 
    int complete = 1;
    unsigned int size = it->Stations.size();
    for (unsigned int i=0; i<size; i++) if (it->Stations[i].Error == IoSdEvent::eDataLost) {
      complete = 0;
    }
    if (it->Id == 0) complete = 0;
    if ( complete ) { /* complete data */
      WriteInRootFile (*it);
      EventList.erase (it++);
    } else {    /* incomplete data */
      time_t t; time(&t);
      if ( (t - it->ConstructionTime) > ((it->Request+1) * TIMETOGETT3) ) { /* request again data to missing ls */
	CheckNewT3Request (it);
	it = EventList.begin (); 
	stop = EventList.end ();
      } else it++; /* wait again */
    } 
    stop = EventList.end ();
  }

}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
void EbManager::CheckNewT3Request (list<EbEvent>::iterator it)
/*-----------------------------------------------------------------------*/ 
{
  if ( !(EventList.size ()) ) return;

  /* no more request data */
  if ( (it->Request) > (MaxNumberOfTries-1) ) { /* trigger too old */
    if (it->Id != 0) WriteInRootFile (*it);
    EventList.erase (it); 
  } else {/* build request with missing ls */
    int nb=0;
    IoSdT3Trigger tmp = it->Trigger;
    vector<unsigned int> localStations;
    vector<unsigned int> windows;
    vector<unsigned int> offsets;
    for (unsigned int i=0; i<it->Stations.size(); i++) {
      if (it->Stations[i].Error == IoSdEvent::eDataLost && it->Stations[i].Trigger.Window==0) {
	localStations.push_back(it->Stations[i].Id);
	windows.push_back(it->Stations[i].Trigger.Window);
	offsets.push_back(it->Stations[i].Trigger.Offset);
        nb++;
      }
    }
    it -> Request++;
    tmp.Id += 0xc000;
    if (nb) SendT3(tmp, localStations, windows, offsets);
  }

}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* It requests a T3 if data aren't come.                                 */
void EbManager::SendT3(const IoSdT3Trigger& t3, const vector<unsigned int>& ids, 
		       const vector<unsigned int>& windows, const vector<unsigned int>& offsets) 
/*-----------------------------------------------------------------------*/ 
{
  IkT3 ikm;
  unsigned int i;

  ikm.id = t3.Id;
  for (i=0;i<ids.size();i++) 
    ikm.addresses.push_back(ids[i]);
  
  for (i=0;i<offsets.size();i++) 
    ikm.offsets.push_back(offsets[i]);
  
  for (i=0;i<windows.size();i++) 
    ikm.window.push_back(windows[i]);
  
  ikm.refSecond = t3.Second;
  ikm.refuSecond = t3.MicroSecond;
  ikm.mode = "LIST";
  ikm.algo = t3.Algo;
  ikm.send(gPmName.c_str());
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/
/* It checks if the root file is opened since more than 12 hours.      */
bool EbManager::CloseFileTest() 
/*-----------------------------------------------------------------------*/
{
  time_t current_time, time_limit;

  time(&current_time);
  time_limit = _OpenFileTime + MaxTimeFileOpen ; // 12 h for official Eb
 
  return (current_time>time_limit);
}
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* It checks if the root file is opened since more than 12 hours.      */
void EbManager::ResetIo(char *opt) 
/*-----------------------------------------------------------------------*/
{
  gHeader.EndOfRun = (unsigned int)( time ((time_t *)NULL) );
  _Io->WFile.File->cd();
  gHeader.Write();
  _Io->Close();
  rename(_RootFileName, _RootFileNameForRename);
  delete _Io;
  if (strcmp(opt, "terminate") == 0) return;

  BuildRootFileName();
  mkdirs(_RootFileName);
  gHeader.StartOfRun = (unsigned int)( time ((time_t *)NULL) );
  if ( !(_Io = new IoSd(_RootFileName, "w")) ) {
    IkSevereSend("Can't open output...");
  }
}
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/ 
/* This function fills data in a root file.                              */
void EbManager::WriteInRootFile(EbEvent& event) 
/*-----------------------------------------------------------------------*/ 
{
stringstream s; s << "Writing event: " << event.Id;
  InternalLog(s.str().c_str(),IKDEBUG);
  if (!_Io) return;

  _Io->Write(event);
  // debug
  for (int i=0;i<event.Stations.size(); i++) {
    if ( event.Stations[i].IsUUB) {
      if (event.Stations[i].Error==256 && event.Stations[i].Histo) {
    stringstream s; s<< "Pushing data of UUB " << event.Stations[i].Id
                     << " with extra[0] " << event.Stations[i].Extra[0] ;
    InternalLog(s.str().c_str(),IKDEBUG);
      }
    }
  }
  // Writing streamer information. Might not be necessary to write each time...
  _Io->WFile.File->WriteStreamerInfo();
  
  // because we know theRootInterface()...
  double NbBytes = _Io->WFile.File->GetBytesWritten();
  if ( NbBytes > MAXROOTFILESIZE || (CloseFileTest() == TRUE)) {
    ResetIo();
    gDBStations = DbCArrayInit(); // Add by Fernando Contreras (Xavier Bertou) for reload the array everytime at a new data file
    gDBNbStations = DbCArrayGetNbStat(gDBStations);
  }
}
/*-----------------------------------------------------------------------*/ 

/*-----------------------------------------------------------------------*/ 
/* This function answers to the ping sender.                             */
void EbManager::IkPong() 
/*-----------------------------------------------------------------------*/ 
{
  if (IkPingValue) {
    IkSuPong pong;
    pong.PongValue=IkPingValue;
    pong.send(SU);
    //     pong.send(PingSender);
    IkPingValue = 0;
  }
}
/*-----------------------------------------------------------------------*/ 
