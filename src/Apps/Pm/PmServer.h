#ifndef __PmServer__
#define __PmServer__

#include "PmClientConnection.h"

class PmServer 
{

 private:
  /* instanciation */
  static PmServer *_instance;
  PmServer ();
  PmServer (const PmServer&);
  ~PmServer ();

  /* server tcp/ip variables */
  int _port, _Number_Fd, _socket_fd, _max_fd, _num_data_client;
  fd_set _Global_Fd,_Temp_Fd;
  struct sockaddr_in _serv_addr;

  /* Pm protocol functions */
  PmStdConnection* _CheckProtocolOfNewConnection (int);
  cdasErrors _SendAck (int,int);
  int _FindConnectionBySocket (int);
  int _FindSocketByConnection (PmMessageType);
  int _FindSocketByBSId (longWord);
  int _FindBSUBySocket (int);
  int _FindBSUByBsId (int);
  int _FindCDASBySocket (int);
  int _EraseSocketLinkedWithBySocket (int);
  void _OrderBsuByLsx();

  /* Listes des connexions multiples. */
  PmBSUConnection *_pmBSUConnection [__MAXBSUONLINE__];
  PmCDASConnection *_pmCDASConnection [__MAXCDAS__];

  /* Connexions uniques. */
  PmDOWNLOADConnection *_pmDOWNLOADConnectionP;

  /* Pour connaitre le service d'un client. */
  class PmSocketLinkedWithService
    {
    public:
      int socket;
      int nameOfService;
      int bsu;
      int bsId;
      int lsxAddr;
      PmSocketLinkedWithService () {}
      PmSocketLinkedWithService (longWord type,const int& sock,int id,int bs_id, char *addr)
	: socket(sock), nameOfService(type), bsu(id), bsId(bs_id) 
	{
	  int a, b, c, d;
	  sscanf(addr, "%d.%d.%d.%d", &a, &b, &c, &d);
	  lsxAddr = c*1000+d;
	}      
      PmSocketLinkedWithService (longWord type,const int& sock,int id,int bs_id)
	: socket(sock), nameOfService(type), bsu(id), bsId(bs_id) 
	{
	  lsxAddr=-1; // cdas client
	}
      ~PmSocketLinkedWithService () {}
    };
  vector<PmSocketLinkedWithService> _pmSocketLinkedWithService;

  /* To manage timing of internal processing */
  time_t _currentTime, _lastTime;

  /* To manage outputs because of uLsx */
  vector<int> _listOfBsu;

  /* Pm connections functions */
  cdasErrors _OpenConnection (PmStdConnection&,int,char*);
  cdasErrors _ManageClient (int);
  PmStdHeader* _ReadPmStdHeaderFromSocket (int);
  cdasErrors _CheckTrailerFromSocket (int, PmTrailer&);

 public:

  /* instanciation */
  static PmServer *Instance ();

  /* Pm protocol functions */
  int _FindBSIdBySocket (int);
  void _RemoveClient (int);
  void _CloseConnection (int);
  void InitPort (int);
  void ProcessInterrupts ();
  void ListenPort ();
  void CheckNewConnection ();
  void ListenRegisteredClients ();
  void OutputToBSU ();
  void OutputToClients ();
  void IkPong ();
  void TreatIkPmMessage (IkMessage*);
  vector<int> GetSocketsOpened ();

  /* ping/pong variable with su */
  int IkPingValue;

};

PmServer* thePmServer ();

#endif
