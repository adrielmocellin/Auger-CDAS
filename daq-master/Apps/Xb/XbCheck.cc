/** \mainpage
 * Welcome to the documentation of the new CDAS Central Trigger. Its name, "Xb" stands
 * for "Xtremely Beautiful", as it is extremely small and we all know small is beautiful.
 * It was designed as an alternative to the official Ct, aiming at simplicity and efficiency.
 * The complete code is less than 1000 lines, and the algorithm is the fastest we thought of
 * in the CDAS group.
 * \section main Main algorithm
 * The idea of the algorithm is to work on a time histogram: one goes through all the t2 of
 * one second, and builds an histogram of number of T2 in bins of 12us (or more if there are
 * not a lot of T2, for speed optimization). Then one only needs to go through this histogram
 * and treat bins of time with 3 (4) or more triggers in a sliding window of 5 bins. When
 * such a bin is found, one checks for the clustering, which is the slow part of the algorithm.
 * This treatments allows to pass only once on the list of T2, hence its speed. It was found
 * to be 40% faster than one using a list of T2 instead of an histogram (as the ordering of the
 * triggers in a list-based algorithm takes time).
 * \section xbcheck XbCheck
 * A program is available to check the Xb algorithm on simulated data. You don't need to have
 * the complete CDAS installation. To compile it without CMT, just go to the src directory and run:
 * \code
 * g++ XbCheck.cc XbArray.cc XbArraySimu.cc XbAlgo.cc -o XbCheck.exe
 * \endcode
 * XbCheck.exe will read a file available named "simu.dat", add 32000 random triggers, and try to find the
 * patterns. Look at the simu.dat file in the src directory to get more information.
 *
 */


/*! \file
    \brief Xb Check allows to check the Xb algorithm on some input patterns
*/

#include "XbArray.h"
#include "XbAlgo.h"
#include "XbCheck.h"
#include <cstdio>
#include <cstdlib>

int newt2config=1,verbose=0,debug=0,trivial_algo=180,carmen_miranda=300;
unsigned int second_;
struct t2 input[NT2MAX];

int constructT2List(unsigned int sec) {
int nt2 = 0; int ntot;
unsigned int id,time,utime,energy;
char buf[256];
char first[256];
  nt2=NBT2;
  ntot=0;
  for (int i=0;i<nt2;i++) {
    input[i].time=0+(int) (1000000.0*rand()/(RAND_MAX+1.0));
    input[i].ID=200+(int) (1600.0*rand()/(RAND_MAX+1.0));
    if ((20.0*rand()/(RAND_MAX+1.0))<1) {// 5% of TOT
      input[i].energy=TOTVALUE;
      ntot++;
    } else input[i].energy=0;
  }
  FILE *f=fopen("simu.dat","r");
  if (f) {
    while (fgets(buf,256,f)) {
      sscanf(buf,"%s",first);
      if (strstr(buf,"#")) continue;
      sscanf(buf,"%u %u %u %u",&id,&time,&utime,&energy);
      if (time==sec) {
        input[nt2].time=utime;
        input[nt2].ID=id;
        input[nt2].energy=energy;
        nt2++;
      }
    }
    fclose(f);
  }
  cout << nt2 << " triggers (" << ntot << " TOT)" << endl;
  return nt2;
}

int main(int argc, char* argv[]) {
  second_=STARTSECOND;
  srandom(RANDOMSEED);
  if (argc>1) {
    if (strstr(argv[1],"-h")) {
      cout << "Welcome to XbCheck" << endl;
      cout << "Run <XbCheck array> to get the array coordinates" << endl;
      cout << "Run <XbCheck> with no arguments to run the program" << endl;
      cout << "When running, XbCheck will generate 32000 random triggers and add the triggers of the simu.dat file" << endl;
      cout << "Format of the simu.dat file is <Id Time(sec) Time(usec) energy(9==TOT)>" << endl;
      return 0;
    }
    XbArrayInit(1);
    return 0;
  }

  XbArrayInit(0);
  XbAlgoT2Init();
  for (second_=STARTSECOND;second_<STARTSECOND+NBSECOND;second_++) {
  int nt2 = constructT2List(second_); // construct a T2 list pf nt2 triggers stored in global array input[NT2MAX]
  vector<t2list> out=XbAlgoT2Process(input,nt2);
  if (out.size()) {
    cout << out.size() << " events found while processing second " << second_ << endl;
    for (unsigned int i=0;i<out.size();i++) {
      t3 myt3=XbAlgoT3Build(out[i]);
      cout << myt3.algo << " at second " << myt3.refSecond << " and microsecond " << myt3.refuSecond << endl;
      for (unsigned j=0;j<myt3.point.size();j++)
        if (myt3.point[j].window==0) cout << myt3.point[j].ID << " " << myt3.point[j].offset << endl;
        // else cout << myt3.point[j].ID << " " << myt3.point[j].offset << " 30" << endl; // uncomment to see 6 crowns of T3...
      }
    }
  }
  return 0;
}
