#include "Threadpool.h"
#include "Thread.h"
#include "Mutex.h"
#include "Condition.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*construct*/
CThreadpool::CThreadpool(const string &n)
: _mutex()/*init mutex*/
, _notFull(_mutex)/*init conditionn*/
, _notEmpty(_mutex)/*init condotion*/
, _poolName(n)/*init Threadpool name*/
, maxTaskQueSize(0)/*set task queu size*/
, isRuning(false)/*init false*/
{}
/*init Threadpool*/
void CThreadpool::initPool(int numOfThread){
	/*the number of Threads > 0*/
	if (numOfThread <= 0) throw exception();
	/*is runing*/
	isRuning = true;
	/*reserve space*/
	_threads.reserve(numOfThread);
	/*take the Threads to the vector*/
	for (int i = 0; i < numOfThread; ++i){
		char id[32];
		snprintf(id, sizeof(id), "%d", i + 1);
		_threads.push_back(
			new Thread(bind(&CThreadpool::threadGetTask, this)
			, _poolName + id));
		_threads[i]->startThread();
	}
	if (numOfThread == 0 && _initCallBack){
		_initCallBack();
	}
}
/*run one thread,push one task*/
void CThreadpool::runThread(Construct_Task &&task){
    /*if have only one thread*/	
    if (_threads.empty()){
        task();/*run the task*/
	}
	else{
        /*locked*/
		CMutexLock lock(_mutex);
        /*if task queue is full*/
		while (isFull()){
            /*wait task queue not full*/
			_notFull.wait();
		}
        assert(!isFull());
        /*push one task*/
		taskQue.push_back(move(task));
        /*awake a thread*/
		_notEmpty.notifyWake();
	}
}
/*stop thread pool*/
void CThreadpool::stopPool(){
	CMutexLock lock(_mutex);
	isRuning = false;
    /*awake all the threads*/
	_notEmpty.notifyWakeAll();
    /*join all the threads*/
	for_each(_threads.begin(), _threads.end()
          , bind(&Thread::joinThread, placeholders::_1));
}

void CThreadpool::setTaskQueSize(int s){
	maxTaskQueSize = s;
}

void CThreadpool::setInitCallBack(const Construct_Task &c){
	_initCallBack = c;
}

CThreadpool::~CThreadpool(){
    if(isRuning){
        stopPool();
    }
}



