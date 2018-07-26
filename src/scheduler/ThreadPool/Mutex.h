#ifndef _MUTEX_H
#define _MUTEX_H

#include "head.h"
#include "Thread.h"
#include <iostream>
#include <assert.h>
using namespace std;

/*pthread mutex class*/
class CMutex{
private:
    /*create mutex*/
    pthread_mutex_t _mutex;
    /*the tid of thread*/
    pid_t _holder;
    /*judge the mutex*/
    bool islocked;
public:
    /*create mutex class*/
    CMutex();
    ~CMutex();
    /*lock the mutex*/
    void lock();
    /*unlock the mutex*/
    void unlock();
    /*trylock*/
    void tryLock();
    //void assertLock();
    /*return mutex*/
    pthread_mutex_t *getThreadLock(){
        return &_mutex;
    }
};
/*baled the mutex*/
class CMutexLock{
private:
    CMutex _mutex;
public:
    explicit CMutexLock(CMutex &mutex)
    : _mutex(mutex)
    {
        _mutex.lock();
    }
    ~CMutexLock(){
        _mutex.unlock();
    }
};

#endif
