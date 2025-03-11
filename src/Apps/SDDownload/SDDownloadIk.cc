#include "IkC.h"
#include "IkSuPing.hxx"
#include "IkSuPong.hxx"
#include "UiMessage.h"
#include "SDDownload.h"
#include "SDDownloadPm.h"
#include "IkLsDownloadAck.hxx"
#include "IkLsDownloadCheck.hxx"
#include "IkDownloadSummary.hxx"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static int SDDIkPingValue;
static int SDDIkCheck=-1;
static vector<int> Missing;
static vector<int> Resend;

static void SendCheck(int slice) {
  IkLsDownloadCheck ikm;
  if (broadcast_init || Participating.size()>1)  // FUCKING BSUS!!!!
    ikm.mode="BROADCAST";
  else {
    ikm.mode="LIST";
    for (unsigned int i=0;i<Participating.size();i++)
      ikm.addresses.push_back(Participating[i]);
  }
  ikm.CheckId=++SDDIkCheck;
  ikm.DownloadId=DownloadId;
  if (slice!=HEADSLICE) ikm.SliceNumbers=slice;
  else ikm.SliceNumbers=0;
  ikm.send(PmName);
}

static int NumberOfChecks=0;

void SDDIkCheckPackets(int slice) {
  fprintf(stderr," %d\n",slice);
  sleep(BSUFUCKDELAY);
  SendCheck(slice);
  Resend.clear();
  if(NumberOfChecks==0) {
    Missing.clear();
    for (unsigned int i=0;i<Participating.size();i++)
      Missing.push_back(Participating[i]);
  }
  // NETWORKDELAY seconds for download
  for (int j=0;j<NETWORKDELAY;j++) {
    sleep(1);
    IkMessageCheck();
    if (broadcast_init==0 && Missing.size()==0) {
      NumberOfChecks=0;
      return;
    }
  }
  NumberOfChecks++;
  if(NumberOfChecks>=NUMBEROFACK) {
    // Missing should be empty. Removing stations that didn't answer
    for (unsigned int i=0;i<Missing.size();i++)
      for (unsigned int j=0;j<Participating.size();j++)
        if (Missing[i]==Participating[j]) {
          IkWarningSend("Removing Station %d from download sequence",Participating[j]);
          Participating.erase(Participating.begin()+j);
        }
    if (Participating.size()==0) {
      if (broadcast_init) IkFatalSend("No station answered my M_DOWNLOAD_CHECK :(");
      else IkFatalSend("No station could survive the entire download :(");
      exit(1);
    }
    NumberOfChecks=0;
    return;
  } else {
    for(unsigned int i=0;i<Resend.size();i++)
      SDDPmSendSlice(Resend[i]);
    SDDIkCheckPackets(slice);
  }
  NumberOfChecks=0;
  broadcast_init=0;
  return;
}

void SDDIkPrepareResend(IkLsDownloadAck* ikm) {
  if (ikm->CheckId!=SDDIkCheck) {
    IkWarningSend("Station %d answered with wrong CheckId, ignoring",ikm->StationId);
    return;
  }
  // Removing station from missing list if nothing is missing any more
  if (ikm->MissingSlices.size()==0)
    for (unsigned int i=0;i<Missing.size();i++)
      if (Missing[i]==ikm->StationId) Missing.erase(Missing.begin()+i);
  // Removing station "trop à la masse" from Participating list
  if (ikm->MissingSlices.size()>TOOMANYSLICESMISSING)
    for (unsigned int i=0;i<Participating.size();i++)
      if (Participating[i]==ikm->StationId) {
        IkWarningSend("Station %d is missing too many slices (%d), dropping it",ikm->StationId,ikm->MissingSlices.size());
        Participating.erase(Participating.begin()+i);
        return;
      }
  // Adding missing slices if any to the list of slices to be resent
  for (unsigned int i=0;i<ikm->MissingSlices.size();i++) {
    int ok=0;
    for (unsigned int j=0;j<Resend.size();j++)
      if (Resend[j]==ikm->MissingSlices[i]) ok=1;
    if (!ok) Resend.push_back(ikm->MissingSlices[i]);
  }
}


void SDDIk(IkMessage *ikm) {
  switch (ikm->get_type()) {
  case IKSUPING:
    SDDIkPingValue=((IkSuPing*)ikm)->PingValue;
    break;
  case IKLSDOWNLOADACK:
    if (broadcast_init) {
      int ok=0;
      for (unsigned int j=0;j<Missing.size();j++)
        if (Missing[j]==((IkLsDownloadAck*)ikm)->StationId) ok=1;
      if (!ok) Missing.push_back(((IkLsDownloadAck*)ikm)->StationId);
      ok=0;
      for (unsigned int j=0;j<Participating.size();j++)
        if (Participating[j]==((IkLsDownloadAck*)ikm)->StationId) ok=1;
      if (!ok) Participating.push_back(((IkLsDownloadAck*)ikm)->StationId);
    }
    SDDIkPrepareResend((IkLsDownloadAck*)ikm);
    break;
  default:
    IkWarningSend("Received Unknown IkMessage:%s",ikm->to_text().c_str());
    break;
  }
}

void SDDIkPong() {
  if (SDDIkPingValue) {
    IkSuPong pong;
    pong.PongValue=SDDIkPingValue;
    pong.send(SU);
    SDDIkPingValue=0;
  }
}

void SDDIkSummary() {
  IkDownloadSummary ikm;
  for (unsigned int i=0;i<Participating.size();i++)
    ikm.StationList.push_back(Participating[i]);
  ikm.send(LOG);
}
