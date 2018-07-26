/*************************************************************************
	> File Name: main.cpp
	> Author: Darkfaker1024@163.com 
	> Mail: 
	> Created Time: Wed 25 Jul 2018 01:39:22 PM CST
 ************************************************************************/

#include <iostream>
#include "head.h"
#include "Threadpool.h"
#include "Thread.h"
using namespace std;
//class Threadpool;

void func(){
    cout << "thread 1  run" << endl;
}

void func2(){
    cout << "thread 2 run" << endl;
}

int main(){

    CThreadpool th(string("thread1"));
    th.setTaskQueSize(4);
    th.initPool(2);
    //th.setInitCallBack(func);
    th.runThread(func2);
    th.runThread(func);
    sleep(10); 
    //th.runThread(func);
    return 0;
}

