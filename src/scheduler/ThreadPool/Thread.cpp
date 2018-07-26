/*************************************************************************
> File Name: Thread.cpp
> Author: Darkfaker1024@163.com
> Mail: Darkfaker
> Created Time: Sat 21 Jul 2018 12:39:55 PM CST
************************************************************************/

#include "Thread.h"
#include "Mutex.h"
#include "Condition.h"
#include <iostream>
using namespace std;

//Thread cade
////////////////////////////////////////////

Thread::Thread(ThreadFunc&& func, const string &n)
: _isRuning(false)
, _isJoined(false)
, _pthreadId(0)
, _tid(new pid_t(0))
, _func(move(func))
, _name(n)
{}

/*start a thread*/
void* runThread(void *msg){
	ThreadData *data = static_cast<ThreadData*>(msg);
	/*call the thread func*/
    data->runThreadFunc();
	delete data;
	return NULL;
}
/*create a thread*/
void Thread::startThread(){
	if (_isRuning) throw "start errror: is runging\n";
	_isRuning = true;
    /*baled the thread data*/
	ThreadData *data = new ThreadData(_func, _name, _tid);
	if (pthread_create(&_pthreadId, NULL, &runThread, (void*)data)){
		_isRuning = false;
		delete data;
	}
}
/*merge the threads*/
int Thread::joinThread(){
	if (!_isRuning) throw "join error: is not runing\n";
	if (_isJoined) throw "join error: is joined\n";
	_isJoined = true;
	return pthread_join(_pthreadId, NULL);
}

Thread::~Thread(){
	if (_isRuning && !_isJoined)
		pthread_detach(_pthreadId);
}
////////////////////////////////////////////////









