#ifndef __PmServices__
#define __PmServices__

#include "PmDefs.h"

static int __num_mess__ = 0;	// RR - pas tres propre tout ca! à dégager dès le prochain protocole DOWNLOAD !!!!!!!!!!

//
// Operator for object managment in a list iterator
//
class sup_priority:public unary_function < PmMessReference, bool >
{
  unsigned char priority, num, slice;
public:
  explicit sup_priority (const unsigned char &a):priority (a) {}
  
  // modified on 2013/Jul/31 - it would fix the order which the messages are sent
  //     from LIFO to FIFO for packages with the same priority.
  //    R. Sato.
  bool operator  () (const PmMessReference & mess) { return mess.priority > priority; }
};

class sup_slice_number:public unary_function < LsMessReference, bool >
{
  int n;
public:
  explicit sup_slice_number (const int &i):n (i) {}
  bool operator  () (const LsMessReference & mess) { return mess.slice > n; }
};


class equal_slice_number:public unary_function < LsMessReference, bool >
{
  int n;
public:
  explicit equal_slice_number (const int &i):n (i) {}
  bool operator  () (const LsMessReference & mess) { return mess.slice == n; }
};

class equal_mess_number:public unary_function < LsMessReference, bool >
{
  int n;
public:
  explicit equal_mess_number (const int &i):n (i) {}
  bool operator  () (const LsMessReference & mess) { return mess.messNumber == n; }
};




//
// Pm services object
//
class PmServices
{
protected:
  /* Is there any client to get output ? */
  short _CountClients;

public:
  PmServices () {
    Output = NULL;
    OutputData = NULL;
    lengthOutput = _CountClients = 0;
  }
  virtual ~ PmServices () {}

  virtual void IAmClientForOutput ();
  virtual void ForgetMe ();

  virtual void GetInput (const LsMessReference &);
  virtual void BuildOutput (const LsMessReference &);
  virtual void ResetOutput ();
		
  char *Output;
  int lengthOutput;
  PmServicesOutput *OutputData;
  vector < PmServicesOutput > Outputs;
  int TypeOfService;
};

//
// Services required by BSU client
//
// LSBLOCK: LS -> CS 
class PmLSBLOCKSServices:public PmServices
{
private:
  list < LsMessReference > _partial_message[__MAXLSID__][__MAXSTREAM__][__MAXINDEX__];

  void _ProcessMess (const LsMessReference &);
  void _ProcessGenMess (const LsMessReference &);
  void _ProcessError (const LsMessReference &);
  void _ProcessNotDefined (const LsMessReference &);
  void _JustNotifyRC (const LsMessReference &);
  int _FindStream (const int &);
  LsMessReference *_AddPartialMessage (LsMessReference &);
  int _ResetSliceNumber (int, int, int, int);
  void _DiscardPartialMessage (int, int, int);
  int _SearchIncompleteMessage (int, int, int);
  int _FindIndexByNumberMessAndDiscardIncompleteMess (int, int, int);
  LsMessReference *_InsertRawMessAndBuildCompleteMess (int, int, LsMessReference &);
  LsMessReference *_BuildMessageWithCompleteRawMessage (int, int, int);
  void _EraseCompleteMessage (int, int, int);
  void _FreeRetSaucisson (LsMessReference *);

public:
  PmLSBLOCKSServices () { TypeOfService = PmLSBLOCKS; }
  ~PmLSBLOCKSServices () {}
  void GetInput (const LsMessReference &) {}
  void GetLsFrame (const LsFrame &, int);
};

// BROADCAST: CS -> LS
class PmBROADCASTServices:public PmServices
{
protected:
  list < PmMessReference > _messToSend;

  void _FillCsFrameHeader (CsFrame &, int, char *, const vector < int >&);
  void _FillLsRawMess (LsRawMessage &, PmMessReference &);
  void _FillLsRawMessInCsFrame (LsRawMessage &, CsFrame &, char *);
  void _FillCsFrameInOutputData (CsFrame &);
  int _CountLengthOfCsFrame (const PmMessReference &);
  void _ReallocOutputDataAndSetVersion ();
  void _AddOutputInList ();

  unsigned char _messNumber;

public:
  PmBROADCASTServices () {
    TypeOfService = PmBROADCAST;
    OutputData = NULL;
    _messNumber = 0;
  }
  virtual ~ PmBROADCASTServices () {}

  char *OutputData_;
  longWord lengthOutputData_;
  vector < PmServicesOutput > listOfOutput_;

  void GetInput (const LsMessReference &) {}
  void GetInput (const PmUPLOADMessage &);
  void GetInput (IkMessage *, char *, int);
  void BuildOutput ();
  void ResetOutput ();

};

// BSCONFIG: CS -> BSU
class PmBSCONFIGServices//: public PmServices
{
};

// BSLOST: BSU -> Pm
class PmBSLOSTServices//: public PmServices
{
};


//
// Services required by client Ct
//
// T2: LS -> Ct
class PmT2Services:public PmServices
{
private:
  PmT2Services ();
  ~PmT2Services () {}
  static PmT2Services *_instance;

  unsigned int _lastTime; /* Pour se situer par rapport à la seconde du T2 courant. */
  int _firstFiveSec; /* Pour qu'il s'écoule 5 sec la première fois. */
  unsigned int _verboseCounter; /* verbose messages */

  void _AddT2InListIndexedBySecond (const LsMessReference &, int);
  void _BuildOutput (int);
  void _CleanMemory (int);

public:
  vector < T2LsBuffer > _t2Buffer[6]; /* To store T2 of each local station. */
  unsigned int _t2Second[6]; 
  static PmT2Services *Instance ();
  void GetInput (const LsMessReference &);
};
PmT2Services *theT2Services ();

/* XB XB XB */
class PmT2FastServices:public PmServices
{
private:
  PmT2FastServices ();
  ~PmT2FastServices () {}
  static PmT2FastServices *_instance;

public:
  void _BuildOutput (int);
  static PmT2FastServices *Instance ();
};
PmT2FastServices *theT2FastServices ();
/* XB XB XB */
//
// Services required by client Eb
//
// EVENT: LS -> Eb
class PmEVENTServices:public PmServices
{
private:
  PmEVENTServices () { TypeOfService = PmEVENT; }
  ~PmEVENTServices () {}
  static PmEVENTServices *_instance;

public:
  static PmEVENTServices *Instance ();

};
PmEVENTServices *theEVENTServices ();

// CALIB: LS -> Eb
class PmCALIBServices:public PmServices
{
private:
  PmCALIBServices () { TypeOfService = PmCALIB; }
  ~PmCALIBServices () {}
  static PmCALIBServices *_instance;

public:
  static PmCALIBServices *Instance ();

};
PmCALIBServices *theCALIBServices ();

//
// Services required by client MoSd
//
// MON: LS -> Mo
class PmMoSdRawServices:public PmServices
{
private:
  PmMoSdRawServices () { TypeOfService = PmMoSdRaw; }
  ~PmMoSdRawServices () {}
  static PmMoSdRawServices *_instance;

public:
  static PmMoSdRawServices *Instance ();
};
PmMoSdRawServices *theMoSdRawServices ();


//
// Services required by client sPMT
//
// MON: LS -> sPMT
class PmSpmtSdServices:public PmServices
{
private:
  PmSpmtSdServices () { TypeOfService = PmSpmt; }
  ~PmSpmtSdServices () {}
  static PmSpmtSdServices *_instance;

public:
  static PmSpmtSdServices *Instance ();
};

PmSpmtSdServices *theSpmtSdServices();

//
// Services required by Generic client
//
// DOWNLOAD
class PmDOWNLOADServices:public PmBROADCASTServices
{
private:
  EnumPmBoolean _firstMess;
  word _slice;

  void _BuildOutput (PmDownloadMessage &);

public:
  PmDOWNLOADServices () {
    _firstMess = PmYES;
    __num_mess__++;
    _slice = 0;
  }
  ~PmDOWNLOADServices () {}

  void GetInput (const LsMessReference &) {}
  void GetInput (char *);
  EnumPmBoolean GetStatusMess () { return _firstMess; }
};

// UPLOAD
class PmUPLOADServices:public PmServices
{
private:
  PmUPLOADServices () {}
  ~PmUPLOADServices () {}
  void _BuildFilesName (int, IkMessage *);
  int _CheckErrorCode (const LsMessReference &);

  static PmUPLOADServices *_instance;
  PmUPLOADMessage _uploadParticipating[__MAXBSUONLINE__];	// RR - macro
  char _uploadFileName[__MAXLSID__][__MAXBSUONLINE__];	// RR - macro
  FILE *_upLoadFile;

public:
  static PmUPLOADServices *Instance ();

  void GetInput (const LsMessReference &);
  void WakeUp (IkMessage *, int, int);
  PmUPLOADMessage *CheckMessage (int);
  EnumPmBoolean State (int);
};
PmUPLOADServices *theUPLOADServices ();

// LTRASH
class PmLTRASHServices:public PmServices
{
private:
  PmLTRASHServices () { TypeOfService = PmLTRASH; }
  ~PmLTRASHServices () {}

  static PmLTRASHServices *_instance;
  FILE *_trashFile;
  char _trashFileName[__MAXFILENAME__];

public:
  static PmLTRASHServices *Instance ();
  void GetInput (const LsMessReference &, int) {}
  void GetInput (IkMessage *);
  void GetInput (LsFrameP, int &);
};
PmLTRASHServices *theLTRASHServices ();
// RADIOTRASH
class PmRADIOTRASHServices:public PmServices
{
private:
  FILE *_trashFile;
  char _trashFileName[__MAXFILENAME__];

public:
  PmRADIOTRASHServices () {}
  ~PmRADIOTRASHServices () {}

  void GetInput (const LsMessReference &, int) {}
  void GetInput (char *, int &, int &);
};

// BTRASH
class PmBTRASHServices//:public PmServices
{
};

#endif
