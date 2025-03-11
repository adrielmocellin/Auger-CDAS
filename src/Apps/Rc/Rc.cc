#include <Rc.h>
#include <time.h>
#include <math.h>
#include <iomanip>
#include <sstream>

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
{stringstream s;
	s << "\t#    Run Controller for AUGER experiment started       #";
        InternalLog(s.str().c_str(), IKINFO);
}

void RunControl::Init(string name)
{
	CDASInit(name.c_str());
	Info();
	IkMonitor( &RunControl::RcIkHandler,
			"destination is \"%s\" or type is %d",
			name.c_str(),IKLSGENMESS);
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

int main()
{string name="Rc";
	theRunControl()->Init(name);
	while (1) {
        sleep(1);
        IkMessageCheck();
        }
}
