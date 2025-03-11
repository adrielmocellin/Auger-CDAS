#ifndef EBMANAGER_H
#define EBMANAGER_H

#include <sys/time.h>
#include "Eb.h"
#include <Pm.h>
#include <Gnc.h>
#include <IkSDSummary.hxx>
#include <IkSDUpdate.hxx>
#include <DbC.h>
#include <DbCArray.h>
#include <IoSd.h>

int EbPmInit(CDASSERVER* );

/* #defines of T3 re-requested */
// 8 minutes. Because of new histograms
#define TIMETOGETT3 480  // FIXME
// Turning back on retries... XB 17/11/2009
// #define MAXNUMBEROFTRIES 1

class EbManager 
{
 public:
  int IkPingValue;
  list<EbEvent> EventList; /* used to storage the events */
 
 public:
  static EbManager* Instance ();
  void Init ();
  cdasErrors ConnectToPm (int, char*);
  cdasErrors CheckNewT3FromPm ();
  void CheckNewT3FromCt ();
  int IsEventOrphanOfTrigger (IkT3 *);
  void PushDataInEventsList ();
  void SearchEventsFinished ();
  void AddIkT3 (IkT3 *);
  void IkPong ();
  void BuildRootFileName();
  void ResetIo(char *opt="");
  char *DataFilePath; // changed from a #define previously, XB 13/01/2016
  int MaxNumberOfTries; // changed from a #define previously, XB 13/01/2016
  int UseRealIDs; // changed from always previously, XB 13/01/2016
  int MaxTimeFileOpen; // changed from a #define previously, XB 13/01/2016

 protected : 
  EbManager();
  
 private:
  void ReadId();
  EbEvent* Find_EbEvent_ByTriggerId ();
  int PushStationInEvent (EbEvent*);
  void CheckNewT3Request (list<EbEvent>::iterator);
  void SaveEventNumber ();
  void WriteInRootFile(EbEvent&);
  void SendT3(const IoSdT3Trigger&, const vector<unsigned int>&, const vector<unsigned int>&, const vector<unsigned int>&);
  void ClearNewT3Trigger();
  void SetNewT3Trigger(IkT3 *);
  unsigned int DecodeDataType(char* );
  void ReconnectToPm ();
  int FillCurrentStation(char *, unsigned int);
  bool CloseFileTest();

 private:
  static EbManager *_instance;
  int _SocketFd, _Listen, _Port;
  char _Addr [128];
  fd_set _Global_Fd;
  //  std::string PingSender;
  IkT3 *_CurrentTrigger;
  IoSdStation *_CurrentStation;
  CDASSERVER* _PmServer;
  int _PmSocket;
  IoSd *_Io;
  int _T3IdInCurrentStation, _T3PreviousId;
  char _RootFileName[MAXFILENAMESIZE];
  char _RootFileNameForRename[MAXFILENAMESIZE];
  time_t _OpenFileTime, _StartOfRun, _EndOfRun;
};

EbManager* TheEbManager();


#endif
