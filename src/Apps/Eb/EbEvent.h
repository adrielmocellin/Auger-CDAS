#ifndef EBEVENT_H
#define EBEVENT_H

#include <time.h>
#include <list>
#include <vector>

#include <IoSdData.h>

#include "TROOT.h"

#include "IkC.h"
#include "IkT3.hxx"
#include "IkSuPing.hxx"
#include "IkSuPong.hxx"

class EbEvent : public IoSdEvent 
{
 public:
  EbEvent();
  EbEvent(IkT3 *);
  EbEvent(IoSdStation *, int);

  time_t ConstructionTime;
  short Request;
};


#include <algorithm>
#include <functional>

class EbEvent_nb : public unary_function<EbEvent,bool> 
{
 private:
  unsigned int n;

 public:
  explicit EbEvent_nb(const unsigned int& nb):n(nb){}
  bool operator()(const EbEvent& c){return c.Trigger.Id == n;}
};

#endif
