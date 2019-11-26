/*
* This class is used in the algorithms.h methods
* It uses the locks in each firstfit, worstfit and bestfit algorithms
* so that when a thread runs each algorithm, it will read through a list,
* apply the read lock and once ready to write, apply the write lock which,
* then waits for all the remaining read locks to complete its search before
* locking out further reads and applying the writing and then resuming reading.
* This implementation should be Deadlock-free, no race conditions and good concurrency.
*/

#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
 
class Locker
{
	pthread_mutex_t mutex;
	
	pthread_cond_t readCond;
	pthread_cond_t writeCond;
	
	int readerCount;
	int writerCount;

public:
	Locker();
	~Locker();
	
	void lockRead();
	void lockWrite();
	
	void unlockRead();
	void unlockWrite();
	
	//void unlock();
};

#endif //LOCKER_H