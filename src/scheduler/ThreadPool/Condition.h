#ifndef _CONDITION_H
#define _CONDITION_H

#include "head.h"
#include "Mutex.h"
#include <iostream>
using namespace std;
/*Condition class*/
class Condition{
private:
    /*protected the Condition*/
	CMutex &_mutex;
    pthread_cond_t _pcond;
public:
	Condition(CMutex &m);
	~Condition();
    /*wait Condition*/
    void wait();
    /*awake thread*/
    void notifyWake();
    /*awake all the threads*/
    void notifyWakeAll();
};

#endif
