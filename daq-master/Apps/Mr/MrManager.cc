#include <signal.h>

#include <IkC.h>
#include <IkInfo.hxx>
#include <IkWarning.hxx>
#include <IkSevere.hxx>
#include <IkFatal.hxx>
#include <IkLsCalReq.hxx>

#include <central_local.h>

#include <cstdlib>
#include <cstdio>

#include "TROOT.h"
#include "TTree.h"

//#include "TSDCalibration.h"
//#include "TSDMonCal.h"

#include "Mr_ClassDef.h"

#include "MrManager.h"
#include "MrUUBCalibMonit.h"
#include "MrRoot.h"
#include "calmon_pack.h"
#include "central_local.h"

extern TTree*     gSDMonCalTree;
extern TSDMonCal* gSDMonCal;

const int kMaxRootFileSize = 620000000;
const int kDATALTH = 10;
const int kBufSize = 100000;

static const int kTimeBetweenRequestCalib = 6*60; // 6 minutes between 2 calib request
//static const int kTimeBetweenRequestCalib = 6*60000000; // debugging mode...

static const int kMinTimeBetweenMonitBlocks = 5*60; // We wait at least 5 minutes before considering a new monitoring entry (modif because some stations send one block every 10 seconds)

UInt_t gLastMonitBookingTime[kLS];

TMrManager::TMrManager()
{
  //
  // Initializes some members
  //
  fPmServer = NULL;

  fLastRequestCalibSecond = 0;
  
   fMonitData = new TMoRawData[kLS];
   fCalibData = new TCbRawData[kLS];
  for (int i=0; i<kLS; i++) {
    fMonitData[i].Reset();
    fCalibData[i].Reset();
    gLastMonitBookingTime[i]=0;
  }
}


int MrPmInit(CDASSERVER *server)
{
  //
  // Connection to Pm
  //

  longWord metaData[100];
  int nbytes;
  int rtn;

  /* first we must create the data */
  metaData[0] = htonl(1);                  /* service version */
  metaData[1] = htonl(2);                  /* # services  */
  metaData[2] = htonl(PmCALIB);
  metaData[3] = htonl(0);                  /* #params  */
  metaData[4] = htonl(PmMoSdRaw);
  metaData[5] = htonl(0);                  /* #params  */
  rtn = GncWrite(server, metaData, 24, PmCBID,&nbytes);

  if (rtn == CDAS_SUCCESS) {
    int rtn; 
    longWord fromPmData[kDATALTH];
    GsStdHeader *header;
    if ((rtn = GncRead(server, (void *)fromPmData,
		       kDATALTH*sizeof(longWord),&nbytes)) == CDAS_SUCCESS) {
      header = GncGetHeaderLastRead();
      IkInfoSend ("Connected to Pm.");
    }
    else IkWarningSend( "Problem with reading IAmCb reply");
  }
  else IkWarningSend("Problem with sending IAmCb");
  return rtn;
}

cdasErrors TMrManager::ConnectToPm(int port, char *addr)
{
  //
  // Connection to Pm
  //

  if ( GncInit("Pm", PmMOID, addr, port, (int (*)(CDASSERVER *srvr))MrPmInit,
	       NULL, &fPmServer) != CDAS_SUCCESS)
    return CDAS_CONNECT_ERROR;
  
  fPmSocket = fPmServer->fd;
  return CDAS_SUCCESS;
}

cdasErrors TMrManager::ListenOnPort()
{
  //
  // Listen if Pm has sent us something
  //

  struct timeval timeout;
  
  timeout.tv_sec = 0; timeout.tv_usec = 5000;
  FD_ZERO (&fGlobalFd);
  FD_SET (fPmSocket,&fGlobalFd);
  
  fListen = select(fPmSocket+1, &fGlobalFd, NULL, NULL, &timeout);

  if ( (fListen > 0) && FD_ISSET(fPmSocket,&fGlobalFd) )
    return CDAS_SUCCESS;
  
  return CDAS_SELECT_ERROR;
}

cdasErrors TMrManager::ListenPm()
{
  //
  // If a message has been sent, process it.
  // The storage on disk is made in the following way :
  // we store the monitoring and calibration data till
  // a pair is complete for a given station then we store on disk.
  // If we receive consecutively 2 calib. or monit. blocks from
  // the same station we store one block on disk.
  //

  cdasErrors error;
  int nb_bytes;

  if ( (error = ListenOnPort()) != CDAS_SUCCESS )
    return error;
  
  union{int ibuf[kBufSize]; char cbuf[kBufSize*sizeof(int)];} data;
  
  if ( GncRead (fPmServer, data.ibuf, kBufSize, &nb_bytes) != CDAS_SUCCESS ) {
    // error during the read, close the connection
    IkFatalSend("Error during the read of data. Bye");
    GncClose(fPmServer);
    CloseRootFile();
    exit(0);
  }

  PmLsRawMessage *rawP = (PmLsRawMessage *)(data.cbuf+sizeof(longWord));
  PmLsMessage *messP =  &(rawP->mess);
  int lsId = ntohl(rawP->lsId);
  int rcvcode = ntohl(messP->type);
  int errcode=0;

  if (rcvcode == M_CALIB_SEND || rcvcode == M_CALIB_REQ_ACK) {
    longWord *lpt=(longWord *)&(messP->data);
    longWord BufferOk;
    lpt++;

    BufferOk = ntohl(*lpt++);
    if(BufferOk == CALMON_ALL){ // defined in Utils/CL/central_local.h
      MrUUBCalibMonit(fMonitData[lsId],fCalibData[lsId],data.cbuf);
      FillTree(lsId);
    } else {
      if (fCalibData[lsId].fIsFilled == 0) {
	errcode = fCalibData[lsId].SetData(data.cbuf);
	if (errcode == 0 && fMonitData[lsId].fIsFilled == 1) FillTree(lsId); 
      } else {
	//IkInfoSend("We received 2 calibration blocks without monitoring blocks between");
	cout << endl << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	cout << " We received 2 calibration blocks without monitoring blocks between : Station " << lsId << endl;
	cout << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	FillTree(lsId);
	errcode = fCalibData[lsId].SetData(data.cbuf);
      }
    }
  }
  else if (rcvcode == M_MONIT_SEND || rcvcode == M_MONIT_REQ_ACK) {

    UInt_t actual_time = (UInt_t)time((time_t *)NULL);

    if (fMonitData[lsId].fIsFilled == 0 && actual_time >= (kMinTimeBetweenMonitBlocks + gLastMonitBookingTime[lsId]) ) {
      errcode = fMonitData[lsId].SetData(data.cbuf);
      if (errcode == 0 && fCalibData[lsId].fIsFilled == 1) FillTree(lsId); 
    } else if (actual_time  >= (kMinTimeBetweenMonitBlocks + gLastMonitBookingTime[lsId]) ) {
      //IkInfoSend("We received 2 monitoring blocks without calibration blocks between");
      cout << endl << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
      cout << " We received 2 monitoring blocks without calibration blocks between : Station" << lsId << endl;
      cout << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
      FillTree(lsId);
      errcode = fMonitData[lsId].SetData(data.cbuf);
    }
  } else IkWarningSend("Unknown type message from Pm");
  return CDAS_SUCCESS;
}


void TMrManager::FillTree(int lsid)
{
  //
  // We fill the TSDMrMonCal class (to be stored on disk)
  // We reinitialize the tables for the lsid index
  //

  if (CloseFileTest() == TRUE) {
    cout << " Changement date reached." << endl;
    cout << " Close the current one and open a new one... " << endl;
    gSDMonCalTree->AutoSave();
    gSDMonCalTree->Print();
    CloseRootFile();
    if (OpenRootFile()) BookTree();
  }

  TSDMrMonCal MrMonCal(lsid,&fMonitData[lsid],&fCalibData[lsid]);
  cout << "LS " <<  MrMonCal.fLsId;
  cout << "   MRMONCALTIME " <<  MrMonCal.fTime;
  gSDMonCal = (TSDMonCal*)&MrMonCal;
  cout << "   MONCALTIME " <<  gSDMonCal->fTime << endl;
  FillSDMonCal();

  fMonitData[lsid].Reset();
  fCalibData[lsid].Reset();

  if (BookingTreeTest() == TRUE && FileOk()) {
    cout << "LS " <<  MrMonCal.fLsId;
    cout << "   BOOKING MRMONCALTIME " <<  MrMonCal.fTime;
    cout << "   MONCALTIME " <<  gSDMonCal->fTime << endl;
    gSDMonCalTree->AutoSave();
    gSDMonCalTree->Print();
    
    if (FileSize() > kMaxRootFileSize || CloseFileTest() == TRUE) {
      cout << " Too high size file or changement date reached." << endl;
      cout << " Close the current one and open a new one... " << endl;
      CloseRootFile();
      if (OpenRootFile()) BookTree();
    }
  }
#ifdef VERBOSE
  MrMonCal.Dump();
#endif
}

void TMrManager::RequestCalibFromSD()
{
  //
  // Sends an Ik message to request calibration from the LS of the EA
  // no more used
  //
  return;

  time_t currenttime;
  currenttime=time((time_t *)NULL);

  if (abs(currenttime-fLastRequestCalibSecond) > kTimeBetweenRequestCalib) {
    fLastRequestCalibSecond = currenttime;
    // Send an Ik Message to request calib !!!!

    IkLsCalReq ikm;
    IkPmMessage *msg =(IkPmMessage*)&ikm;
    msg->mode = "LIST";
    msg->addresses.push_back((int)14);
    msg->addresses.push_back((int)16);
    msg->addresses.push_back((int)18);
    msg->addresses.push_back((int)21);
    msg->addresses.push_back((int)26);
    msg->addresses.push_back((int)31);
    msg->addresses.push_back((int)33);
    msg->addresses.push_back((int)34);
    msg->addresses.push_back((int)35);
    msg->addresses.push_back((int)36);
    msg->addresses.push_back((int)38);
    msg->addresses.push_back((int)40);
    msg->addresses.push_back((int)43);
    msg->addresses.push_back((int)44);
    msg->addresses.push_back((int)45);
    msg->addresses.push_back((int)46);
    msg->addresses.push_back((int)48);
    msg->addresses.push_back((int)49);
    msg->addresses.push_back((int)51);
    msg->addresses.push_back((int)52);
    msg->addresses.push_back((int)53);
    msg->addresses.push_back((int)55);
    msg->addresses.push_back((int)57);
    msg->addresses.push_back((int)58);
    msg->addresses.push_back((int)61);
    msg->addresses.push_back((int)63);
    msg->addresses.push_back((int)64);
    msg->addresses.push_back((int)65);
    msg->addresses.push_back((int)66);
    msg->addresses.push_back((int)67);
    msg->addresses.push_back((int)68);
    msg->addresses.push_back((int)70);
    msg->send("Pm");
  }
}
