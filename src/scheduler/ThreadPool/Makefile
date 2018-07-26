main: main.o Threadpool.o Thread.o Condition.o Mutex.o head.h 
	g++ -o main main.o Threadpool.o Thread.o Condition.o Mutex.o head.h -lpthread -std=c++11
main.o: main.cpp Threadpool.cpp Threadpool.h Thread.cpp Thread.h Condition.cpp Condition.h Mutex.cpp Mutex.h head.h
	g++ -c main.cpp Threadpool.cpp Threadpool.h Thread.cpp Thread.h Condition.cpp Condition.h Mutex.cpp Mutex.h head.h -lpthread -std=c++11
Threadpool.o: Threadpool.cpp Threadpool.h Thread.cpp Thread.h Condition.cpp Condition.h Mutex.cpp Mutex.h head.h
	g++ -c Threadpool.cpp Threadpool.h Thread.cpp Thread.h Condition.cpp Condition.h Mutex.cpp Mutex.h head.h -lpthread -std=c++11
Thread.o: Thread.cpp Thread.h head.h
	g++ -c Thread.cpp Thread.h head.h -std=c++11
Mutex.o: Mutex.cpp Mutex.h head.h
	g++ -c Mutex.cpp Mutex.h head.h -std=c++11
Condition.o: Condition.cpp Mutex.cpp Condition.h Mutex.h head.h 
	g++ -c Condition.cpp Condition.h Mutex.cpp Mutex.h head.h -std=c++11

clean:
	rm -rf *.o 
	rm -rf *.h.gch
