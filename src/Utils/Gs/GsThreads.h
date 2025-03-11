/* ****************************************************************** */
/* GsSynchro.h :                                                      */
/*       defines the data structures used to synchronize              */
/*       activities in Gs                                             */
/*                                                                    */
/* ****************************************************************** */

/*! 
 This file declare the data structures needed to synchronize the 
 GsClientManagers (\ref GsClientManager) and GsLoop.
 Dedicated types are defined (together with the associated pointers) to hide 
 the underlaying thread library : 
       - GsLocker for mutexes (i.e. to guarranty non concurent access to shared memory, 
       - GsSignal for conditions (i.e. to allow a interthread communication)
*/

#ifndef __GSTHREADS__
#define __GSTHREADS__

#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef GSMAIN
#define GSTHDECLARE 
#else
#define GSTHDECLARE extern
#endif

typedef pthread_t GsThread, * GsThreadP;

static inline GsThread GsThreadWAI(void)
{
  return (GsThread)(pthread_self());
}

typedef pthread_mutex_t GsLocker, * GsLockerP;
typedef pthread_cond_t GsSignal, * GsSignalP;


#define GSLOCKERINITIALIZER PTHREAD_MUTEX_INITIALIZER
#define GSSIGNALINITIALIZER PTHREAD_COND_INITIALIZER



/*! GsGlobalLocker : Mutex used to block access to global ressources
           AND server data when/if not thread awared
    GsGlobalSignal : to watched for GsGlobalLocker associated variables
*/
GSTHDECLARE GsLocker GsGlobalLocker;
GSTHDECLARE GsSignal GsGlobalSignal;
GSTHDECLARE GsLocker GsIkLocker;


/***********************************************************************/
/* Access to these global variables */
/* some simple wrappers to (partially) hide the thread library */

static inline void GsLockerInit(GsLockerP lockerp)
{
  // TO BE CHANGED : set attr by hand
  pthread_mutex_init(lockerp,NULL);
}

// WARNING : to avoid deadlock, one will introduce a "cleanup" function
// every time a mutex is locked. But the present implementation impose
// to have cleanup_push and cleanup_pop AT THE SAME level of block nesting!
// So we MUST use macros instead of functions to manipulate GsLockers,
// and they must be used only at the same level of block nesting. Sorry!

#ifdef SEPARABLECLEANUPFUNCTIONS

static inline int GsGetAccessTo(GsLockerP lockerp)
{
  // cleanup push
 return pthread_mutex_lock((pthread_mutex_t)lockerp);
}
static inline int  GsTryAccessTo(GsLockerP lockerp)
{
  // cleanup push
 pthread_mutex_trylock((pthread_mutex_t)lockerp);
}
static inline int GsReleaseAccessTo(GsLockerP lockerp)
{
  int code;
 code = pthread_mutex_unlock((pthread_mutex_t)lockerp);
 // cleanup pop
 return code;
}
#else // SEPARABLECLEANUPFUNCTIONS

// TO BE CHANGED : this version implies nesting lock/unlock
// Which may be not the right way
// WARNING : it was too restrictive to provide memorization of previous state 
// So the threads are assumed to be always in DEFERRED mode !!!!
#ifdef ASYNCHRONOUSTHREADMODE
#define GsGetAccessTo(lockerp)  \
         { /* closing will be in releaseAccess !!!! */ \
          int lockerp##oldtype; \
         pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &lockerp##oldtype); \
         pthread_cleanup_push((void (*) (void  *))pthread_mutex_unlock, \
                              (void *) (lockerp)); \
         pthread_mutex_lock((pthread_mutex_t*)(lockerp));

#define GsReleaseAccessTo(lockerp)  \
         pthread_cleanup_pop(1); /* execute the cleanup and pop it */ \
         pthread_setcanceltype(lockerp##oldtype,NULL); \
         } /* close the brace opened in GetAccess !!! */

#else // ASYNCHRONOUSTHREADMODE
#define GsGetAccessTo(lockerp)   \
         pthread_cleanup_push((void (*) (void  *))pthread_mutex_unlock, \
                              (void *) (lockerp)); \
         pthread_mutex_lock((pthread_mutex_t*)(lockerp));

#define GsReleaseAccessTo(lockerp)  \
         pthread_cleanup_pop(1); /* execute the cleanup and pop it */ 

#endif // ASYNCHRONOUSTHREADMODE
#endif // SEPARABLECLEANUPFUNCTIONS

static inline int GsSendSignal(GsSignalP condp)
{
 return pthread_cond_signal((pthread_cond_t *)condp);
} 

static inline int  GsWaitFor(GsSignalP condp, GsLockerP lockerp)
{
 return pthread_cond_wait((pthread_cond_t *)condp,(pthread_mutex_t*)lockerp);
} 

// Access to server data space. If server routines are "threads compatible"
// there is no needs to do that (parrallel processing)

#ifdef SERVERISTHREADCOMPATIBLE
static inline void GsGetAccessToServer(void)
{
}
static inline void GsReleaseAccessToServer(void)
{
}
#else // SERVERISTHREADCOMPATIBLE
// WARNING : as we use GsGetAccessTo, same problems here !
#define GsGetAccessToServer() \
                      GsGetAccessTo(&GsGlobalLocker);
#define GsReleaseAccessToServer() \
                      GsReleaseAccessTo(&GsGlobalLocker);
#endif // SERVERISTHREADCOMPATIBLE

/*************************************************************************/
// WARNING : to allow proper thread termination (e.g. proper services cleaning
// when a client manager exits) we will introduce a "cleanup" function every 
// time we have something more to clean.
// But the present implementation impose to have cleanup_push and 
// cleanup_pop AT THE SAME level of block nesting!
// So we MUST use macros instead of functions and they must be used only at 
// the same level of block nesting. Sorry!
#ifdef SEPARABLECLEANUPFUNCTIONS
// TO BE DONE once it will be possible
#else // SEPARABLECLEANUPFUNCTIONS
#define GsThreadAddExitHandler(function,arg) \
                     pthread_cleanup_push(function,arg);

#define GsThreadRemoveExitHandler(function,arg) \
                     pthread_cleanup_pop(0);
#endif  //SEPARABLECLEANUPFUNCTIONS


/************************************************************************/
/* signal handling */
/* GsHandledSignals makes available for all threads the list of signals */
/* that must be blocked, i.e. a list of signals associated to a sigaction */
/* in the "main thread" BEFORE the "child thread" is launched */
/* for ease of maintenance routines that manipulate sigsets will be */
/* grouped here */

GSTHDECLARE sigset_t GsHandledSignals;

#ifdef GSMAIN
static inline void GsSetSignalHandling(void (*exitFunction)(int)) 
// to be called by the "main thread" only
{
  // handlers (terminate the process, ignore the signal 
  struct sigaction endHandlerParams = { {exitFunction},{{0}},0,NULL} ;
  struct sigaction ignHandlerParams = { {SIG_IGN},{{0}},0,NULL} ;

  // init list of "managed" signals
  sigemptyset(&GsHandledSignals);
  // exit properly on :
  sigaction(SIGTERM,&endHandlerParams,NULL);
  sigaddset(&GsHandledSignals,SIGTERM);
  sigaction(SIGQUIT,&endHandlerParams,NULL);
  sigaddset(&GsHandledSignals,SIGQUIT);
  sigaction(SIGABRT,&endHandlerParams,NULL);
  sigaddset(&GsHandledSignals,SIGABRT);
  // TRY to stay alive in case a client died during a transfer 
  sigaction(SIGPIPE,&ignHandlerParams,NULL);
  sigaddset(&GsHandledSignals,SIGPIPE);
   
}
#endif // GSMAIN


/***************************************************************************/
/* routines to be used in threads */

static inline void GsThreadInit()
{
  // place here what every thread should do when launched 
  // e.g. mask system signals, etc..
  pthread_sigmask(SIG_BLOCK,&GsHandledSignals,NULL);
}

static inline void GsThreadExit(int code)
{
  long int retval=code;
  pthread_exit((void *)retval);
}

// GsThreadContinue is a wrapper for pthread_test_cancel
// it MUST be used before and after system calls to avoid
// possible dead-locks
static inline void GsThreadContinue(void)
{
  pthread_testcancel();
}

/***************************************************************************/
/* threads management */

static inline int GsThreadLaunch(GsThreadP thread,
				 void * (*start_routine)(void *),
				 void * arg)
{
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(
			      &attr, 
			      PTHREAD_CREATE_DETACHED
			      );
  return (pthread_create(
			 (pthread_t *) thread,
			 &attr,
			 start_routine,
			 arg
			 )
	  );
  
}

static inline int GsThreadDestroy(GsThread thread)
{
  return(pthread_cancel((pthread_t)thread));
}



#undef GSTHDECLARE
#endif
