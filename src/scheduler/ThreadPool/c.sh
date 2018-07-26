#!/bin/bash
rm *.o
rm *.h.gch
#g++ -o main main.cpp Threadpool.cpp  Thread.cpp  Condition.cpp  Mutex.cpp  -std=c++11 -lpthread
g++ -o main main.cpp Mutex.cpp Condition.cpp Thread.cpp Threadpool.cpp -lpthread -std=c++11
