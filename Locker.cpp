/*
* This class is used in the algorithms.h methods
* It uses the locks in each firstfit, worstfit and bestfit algorithms
* so that when a thread runs each algorithm, it will read through a list,
* apply the read lock and once ready to write, apply the write lock which,
* then waits for all the remaining read locks to complete its search before
* locking out further reads and applying the writing and then resuming reading.
* This implementation should be Deadlock-free, no race conditions and good concurrency.
*/

#include "Locker.h"

Locker::Locker()
{
    pthread_mutex_init(&mutex, NULL);   
    pthread_cond_init(&readCond, NULL);
    pthread_cond_init(&writeCond, NULL);
	
	readerCount = 0;
    writerCount = 0;
}

void Locker::lockRead()
{
    pthread_mutex_lock(&mutex);
	
	while(writerCount > 0 || readerCount == -1) //if theres a writer, block reading until signal from writer.
    {
        pthread_cond_wait(&writeCond, &mutex);
    }
	
    readerCount++;
	pthread_mutex_unlock(&mutex);     
}

void Locker::lockWrite()
{
    pthread_mutex_lock(&mutex);
	
    while(readerCount != 0) //if there are still readers
    {
        writerCount++; //inc writer count for readers to wait after read
        pthread_cond_wait(&readCond, &mutex); //wait for readers to release
        writerCount--;
    }
	
    readerCount = -1;
    pthread_mutex_unlock(&mutex);
}

void Locker::unlockRead()
{
	pthread_mutex_lock(&mutex);
	
    if(readerCount > 0) //one of the readers have unlocked, then decrement reader.
    {
        readerCount--;
		
		if(readerCount == 0 && writerCount > 0) //readers finished and writer is waiting to write
		{
			pthread_cond_signal(&readCond); //broadcast to begin writing
		}
    }
	
	pthread_mutex_unlock(&mutex);
}

void Locker::unlockWrite()
{
	pthread_mutex_lock(&mutex);
	
	if(readerCount == -1) //one of the writers unlocked
    {
		readerCount = 0;
        if(writerCount > 0) //if another writer waiting, then signal it.
        {
            pthread_cond_signal(&readCond);
        }
        else //signal the readers to begin reading
        {
            pthread_cond_broadcast(&writeCond);
        }
    }
	
	pthread_mutex_unlock(&mutex);
}

Locker::~Locker()
{
    pthread_cond_destroy(&readCond);
    pthread_cond_destroy(&writeCond);
	pthread_mutex_destroy(&mutex);
}