#include <Rc.h>
#include <time.h>
#include <math.h>
#include <iomanip>
#include <sstream>
#include <IkLsReady.hxx>
#include <IkLsGenMess.hxx>
#include <IkLsStartAck.hxx>
#include <IkLsOS9.hxx>
#include <IkLsSaveConf.hxx>
#include <IkLsReboot.hxx>
#include <cstdlib>
#include <cstdio>

// This code is duplicated from Rc instead of making use of it.
// Ugly, but not going to fix it now (ie: won't be fixed ever)

// Instanciate the singleton  --------------------------------
RunControl *RunControl::_instance = new RunControl();

RunControl *RunControl::Instance() {return _instance;}

RunControl* theRunControl(){return RunControl::Instance();}
// -----------------------------------------------------------

// RuncControl constructor
RunControl::RunControl() 
{
	memset(_Db.Survey, 0, sizeof(SdTank)*(MAXTANK+1));
	_Db.nSurvey = 0; _Db.lastLoad  = 0;
}

void RunControl::Info()
{
	cout << endl;
	cout << "\t########################################################" << endl;
	cout << "\t#    Manual Station configurator                       #" << endl;
	cout << "\t########################################################" << endl<<endl;
}

void RunControl::Init(string name)
{
	CDASInit(name.c_str());
	Info();
	IkMonitor( &RcIkHandler,
			"destination is \"%s\" or type is %d or type is %d",
			name.c_str(),IKLSREADY,IKLSGENMESS);
        DbLoad();
}

const char *RunControl::BuildConfig(SdTank *T, int newId, int cpuId, int uub)
{
	static std::string tmp;
	stringstream toto;
	int gps=MySqlGetGPSOffset(cpuId-2048,uub);
	toto << "buildconfig" << setprecision(12) << " north="<< lrint(T->Northing*100.0);
	toto << " east=" << lrint(T->Easting*100.0);
	toto << " height=" << lrint(T->Altitude*100.0) << " hold=1";
	if (newId) toto << " id=" << newId;
	if (gps) toto << " off=" << gps;
	toto << '\0';
	tmp = toto.str();
	return tmp.c_str(); 
}

int main(int argc, char **argv)
{string name="RcManual"; int cpuId = -1; int surveyId = -1; int surveyIndex, cdasIndex,uub;
  if (argc != 4) {
  std::cout << "Usage: " << argv[0] << " CpuId  DbId UUb" << std::endl;
  std::cout << "  will force assignment of DbId to the tank that came online with BootId" << std::endl;
  std::cout << "!!! BootId must be larger than 2047 and DbId must exist in Db !!! "<< std::endl;
  std::cout << "!!! UUb must be 0 or 1, 0 for a UB, 1 for a UUB !!! "<< std::endl;
  exit(1);
  }
  theRunControl()->Init(name);
  cpuId = atoi(argv[1]);
  surveyId = atoi(argv[2]);
  uub = atoi(argv[3]);
  if (cpuId < 2048) {cout << "!!! BootId must be larger than 2047 !!! " << endl; exit(1);}
  theRunControl()->DbLoad();   // Load Db if it was'nt done recently
  if (! (surveyIndex = theRunControl()->SurveyIndex(surveyId))) 
    {cout << "!!! DbId must exist in Db !!! " << endl; exit(1);};
  {IkLsOS9 bldConf; IkLsReboot reboot;
   bldConf.mode = reboot.mode = "LIST";
  SdTank* surveyTank = &(theRunControl()->SurveyDb()[surveyIndex]);
  cout << "Building config for Station "<< cpuId << ". ";
  cout << "It will become \""<< surveyTank->Name << "\"";
  cout << " with new Id " << surveyTank->Id;
  cout << endl;
  bldConf.command = theRunControl()->BuildConfig(surveyTank, surveyTank->Id, cpuId, uub);
  bldConf.addresses.push_back(cpuId);
  bldConf.send("Pm");
  sleep(8);
  reboot.addresses.push_back(cpuId);
  reboot.send("Pm");
  cout << "Config sent..."<<endl; 
  }
}
