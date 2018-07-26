#include "Mutex.h"


//CMutex code
/********************************/
CMutex::CMutex()
:_holder(0)
{
    pthread_mutex_init(&_mutex, NULL);
}

CMutex::~CMutex(){
    pthread_mutex_destroy(&_mutex);
}
/*lock the mutex*/
void CMutex::lock(){
    pthread_mutex_lock(&_mutex);
}
/*unlock the mutex*/
void CMutex::unlock(){
    pthread_mutex_unlock(&_mutex);
}
/*trylock*/
void CMutex::tryLock(){
   pthread_mutex_trylock(&_mutex); 
}

//void CMutex::assertLock(){
  //  assert(_holder == syscall(SYS_gettid));
//}

/********************************/











