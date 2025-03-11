
#include "Pm.h"
#include "PmServer.h"
#include "PmLog.h"

void info ()
{
  PmLogPrint(IKINFO,"   ******** Starting of Post Master for Auger experiment. ********");
  PmLogPrint(IKINFO,"   \"Plus cela va, plus je vais vers la simplicité. J'utilise les");
  PmLogPrint(IKINFO,"   métaphores les plus usées. Au fond, c'est cela qui est éternel:");
  PmLogPrint(IKINFO,"   les étoiles ressemblent à des yeux, par exemple, ou la mort est");
  PmLogPrint(IKINFO,"   comme le sommeil.\"  (Borges)");
}

void usage (char* proc)
{
  PmLogPrint(IKFATAL,"Bad usage : %s  <start|stop>, or %s  <IkSuffix>",proc,proc);
  exit (1);
}

void PmIkHandler (IkMessage* ikm)
{
  switch (ikm->get_type())
    {
    // all these Ik types have for parent class IkPmMessage
    case IKLSWAKEUP:
    case IKLSSTART:
    case IKLSSTOP:
    case IKLSOS9:
    case IKLSCALREQ:
    case IKLSCHANGESTATE:
    case IKLSCONFIG:
    case IKLSGPSSET:
    case IKLSMONREQ:
    case IKLSSAVECONF:
    case IKLSCONT:
    case IKLSLOADCONF:
    case IKLSPARAM:
    case IKLSPAUSE:
    case IKLSREBOOT:
    case IKLSLOGREQ: // upload requests from LS
    case IKT3:
    case IKLSDOWNLOADCHECK:
      thePmServer () -> TreatIkPmMessage (ikm);
      break;
    case IKLSRECORD:
      theLTRASHServices () -> GetInput (ikm);
      break;
    case IKSUPING:
      thePmServer () -> IkPingValue = ((IkSuPing*)ikm)->PingValue;
      break;
    default:
      IkWarningSend ("Unknown Ik message type received %d",ikm->get_type());
      PmLogPrint (IKERROR,"Unknown Ik message type received %d ",ikm->get_type());
      break;
    }

}

string RcName;

int main (int argc, char** argv)
{	
  if ( UiParseCommandLine (argc, argv) != CDAS_SUCCESS )
    usage (argv[0]);

  string ServerName;

  ServerName = PM;
  RcName = RC;
  if ( TaskIkSuffix ) 
    {
      ServerName += TaskIkSuffix;
      RcName += TaskIkSuffix;
    }

  CDASInit (ServerName.c_str());
  IkMonitor (PmIkHandler, "destination is \"Pm\"");

  info ();

  thePmServer () -> InitPort (PmPort);

  PmLogPrint (IKINFO,"Connexion tcp/ip ouverte sur le port %d", PmPort);

  while (1)
    {
      thePmServer () -> ListenPort ();

      thePmServer () -> CheckNewConnection ();

      thePmServer () -> ListenRegisteredClients ();

      thePmServer () -> ProcessInterrupts ();

      thePmServer () -> OutputToClients ();

      thePmServer () -> IkPong ();
    }

  return 0;
}
