/*************************************************************************
	> File Name: Thread.h
	> Author: Darkfaker1024@163.com
	> Mail: Darkfaker1024
	> Created Time: Fri 20 Jul 2018 05:15:41 PM CST
 ************************************************************************/

#ifndef _THREAD_H
#define _THREAD_H
#include "head.h"
#include <iostream>
#include "Condition.h"
#include "Mutex.h"
#include <stdlib.h>
#include <stdio.h>
using namespace std;

/*id cache*/
class CacheTid{
public:
    pid_t _tid;
    CacheTid(pid_t i):_tid(i){}
    pid_t tid(){return static_cast<pid_t>(::syscall(SYS_gettid));}
};
/*theads class*/
class Thread{
public:
    /*callback func*/
    typedef function<void()> ThreadFunc;
protected:
    bool _isRuning;
    bool _isJoined;
    /*thread id*/
    pthread_t _pthreadId;
    /*smart ptr*/
    shared_ptr<pid_t> _tid;
    /*callback func*/
    ThreadFunc _func;
    /*threads name*/
    string _name;
public:
    explicit Thread(ThreadFunc&& func, const string &name = string());
    ~Thread();
    /*create a thread*/
    void startThread();
    /*merge the thread*/
    int joinThread();
    /*update tid*/
    pid_t updateTid()const{return syscall(SYS_gettid);}
    /*get tid*/
    pid_t getTid()const {return *_tid;}
    /*is runing*/
    bool isStart()const {return _isRuning;}
    /*get name*/
    const string &getName()const {return _name;}
    friend class ThreadData;

};

/*baled the thread data*/
class ThreadData{
public:
    typedef Thread::ThreadFunc funcType;
    typedef shared_ptr<pid_t> _Ptr_Type;
    /*thread func*/
    funcType _func;
    /*threads name*/
    string _name;
    weak_ptr<pid_t> _wkTid;
    ThreadData(const funcType &func
               , const string &name
               , const shared_ptr<pid_t> &tid)
    : _func(func)
    , _name(name)
    , _wkTid(tid)
    {}
    /*use the smart ptr start thread func*/
    void runThreadFunc(){
        pthread_t tid = pthread_self();
        _Ptr_Type ptid = _wkTid.lock();
        if(ptid){
            *ptid = tid;
            ptid.reset();
        }/*try catch*/
        try{
            _func();
        }catch(const string &str){
            fprintf(stderr,"shared_ptr to wake_ptr error\n",_name.c_str());
            cout << str << endl;
            abort();
        }catch(const exception& ex){
            fprintf(stderr,"shared_ptr to wake_ptr error\n",_name.c_str());
            cout << ex.what() << endl;
            abort();
        }catch(...){
            fprintf(stderr,"unkown exception\n",_name.c_str());
            abort();
            throw;
        }
    }
};
#endif
