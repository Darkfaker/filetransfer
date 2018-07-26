#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <iostream>
#include "head.h"
#include "Mutex.h"
#include "Condition.h"
#include "Thread.h"
using namespace std;
/*threadpool class*/
class CThreadpool{
public:
	typedef function<void()> Construct_Task;
    typedef vector<Thread*> Vec_Thread;
    typedef deque<Construct_Task> Task_Que;
	typedef unsigned int size_t;
private:
    /*protected the Task_Que*/
	CMutex _mutex;
    /*notice the threads*/
	Condition _notFull;
	Condition _notEmpty;
	string _poolName;
    /*task callback func*/
	Construct_Task _initCallBack;
    /*theads ptr*/
	Vec_Thread _threads;
    /*task queue*/
	Task_Que taskQue;
	size_t maxTaskQueSize;
	bool isRuning;
    bool isFull(){
       // _mutex.assertLock();
        return maxTaskQueSize > 0 && taskQue.size() >= maxTaskQueSize;
    }
    /*get task func*/
    Construct_Task getTask(){
        CMutexLock lock(_mutex);
        while(taskQue.empty() && isRuning){
            _notEmpty.wait();
        }
        Construct_Task task;
        if(!taskQue.empty()){
            task = taskQue.front();
            taskQue.pop_front();
            if(maxTaskQueSize > 0){
                _notFull.notifyWake();
            }
        }
        return task;
    }
	
    /*take the task func from task queue*/
    void threadGetTask(){
        try{
            if(_initCallBack){
                cout << "threadGetTask()" << endl;
                _initCallBack();
            }
            /*runing task*/
            while(isRuning){
                Construct_Task task(getTask());
                if(task){
                    task();
                }
            }
            /*try catch*/
        }catch(const string &str){
            fprintf(stderr,"callbackfunc error\n",_poolName.c_str());
            abort();
        }catch(const exception& ex){
            fprintf(stderr,"callbackfunc error\n",_poolName.c_str());
            cout << ex.what() << endl;
            abort();
        }catch(...){
            fprintf(stderr,"unknow exception\n",_poolName.c_str());
            throw;
        }
    }
	/*take*/

public:
	CThreadpool(const string &n);
    /*start threadpool*/
    void initPool(int numOfThread);
	/*stop threadpool*/
    void stopPool();
	/*run one thread*/
    void runThread(Construct_Task &&task);
	void setTaskQueSize(int s);
    /*init callback*/
	void setInitCallBack(const Construct_Task &c);
	~CThreadpool();
};

#endif


