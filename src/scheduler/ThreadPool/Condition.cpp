#include "Condition.h"

/*craete Condition*/
Condition::Condition(CMutex &m)
:_mutex(m)
{
    pthread_cond_init(&_pcond, NULL);
}
/*destroy Condition*/
Condition::~Condition(){
    pthread_cond_destroy(&_pcond);
}
/*wait Condition*/
void Condition::wait(){
    pthread_cond_wait(&_pcond, _mutex.getThreadLock());
}
/*awake threads*/
void Condition::notifyWake(){
    pthread_cond_signal(&_pcond);
}
/*awake all the threads*/
void Condition::notifyWakeAll(){
    pthread_cond_broadcast(&_pcond);
}
