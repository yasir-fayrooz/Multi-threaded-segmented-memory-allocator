#include <string>
#include <unistd.h>
#include <list>
#include "memory.h"
#include "Locker.h"
#include "StratEnum.h"

Locker* locker = new Locker();

StratEnum strategy;

std::list<memory*> listAllocated;
std::list<memory*> listFreed;

void * firstFit(size_t chunk_size)
{
	void * address = nullptr;
	memory* mem;
	
	std::list<memory*>::iterator it;
	
	locker->lockRead();
	for(it = listFreed.begin(); it != listFreed.end(); it++)
	{
		if(chunk_size < (*it)->size && pthread_mutex_trylock(&(*it)->mutex) == 0)
		{
			locker->unlockRead();
			locker->lockWrite();
			
			mem = (*it);
			address = (*it)->address;
			(*it)->address = (void *)((char*)address + chunk_size);
			(*it)->size -= chunk_size;
			
			pthread_mutex_unlock(&(mem)->mutex);
			locker->unlockWrite();
			return address;
		}
		else if(chunk_size == (*it)->size && pthread_mutex_trylock(&(*it)->mutex) == 0)
		{
			locker->unlockRead();
			locker->lockWrite();
			
			address = (*it)->address;
			it = listFreed.erase(it);
			
			locker->unlockWrite();
			return address;
		}
	}
	
	locker->unlockRead();
	
	return address;
}

void * bestFit(size_t chunk_size)
{
	void * bestFitAddress = nullptr;
	memory * bestMemory = nullptr;
	
	std::list<memory*>::iterator it;
	
	locker->lockRead();
	for(it = listFreed.begin(); it != listFreed.end(); it++)
	{
		if(chunk_size < (*it)->size)
		{
			if((bestMemory == nullptr || bestMemory->size > (*it)->size) && pthread_mutex_trylock(&(*it)->mutex) == 0)
			{
				if(bestMemory != nullptr) //un;ocks old best fit before setting new memory lock to variable.
				{
					pthread_mutex_unlock(&(bestMemory)->mutex);
				}
				bestMemory = *it;
			}
		}
		else if(chunk_size == (*it)->size && pthread_mutex_trylock(&(*it)->mutex) == 0)
		{
			locker->unlockRead();
			locker->lockWrite();
			
			bestFitAddress = (*it)->address;
			it = listFreed.erase(it);
			
			locker->unlockWrite();
			return bestFitAddress;
		}
	}
	locker->unlockRead();
	
	if(bestMemory != nullptr && pthread_mutex_trylock(&(bestMemory)->mutex) == 0)
	{		
		locker->lockWrite();
		
		bestFitAddress = bestMemory->address;
		bestMemory->address = (void *)((char*)bestFitAddress + chunk_size);
		bestMemory->size -= chunk_size;
		
		pthread_mutex_unlock(&(bestMemory)->mutex);
		locker->unlockWrite();
		return bestFitAddress;
	}
	
	return bestFitAddress;
}

void * worstFit(size_t chunk_size)
{
	
	void * worstFitAddress;
	memory * worstMemory = nullptr;
	
	std::list<memory*>::iterator it;
	
	locker->lockRead();
	for(it = listFreed.begin(); it != listFreed.end(); it++)
	{
		if(chunk_size < (*it)->size)
		{
			if(worstMemory == nullptr)
			{
				worstMemory = *it;
			}
			else if(worstMemory->size < (*it)->size)
			{
				worstMemory = *it;
			}
		}
	}
	locker->unlockRead();
	
	if(worstMemory != nullptr && pthread_mutex_trylock(&(worstMemory)->mutex) == 0)
	{
		locker->lockWrite();
		
		worstFitAddress = worstMemory->address;
		worstMemory->address = (void *)((char*)worstFitAddress + chunk_size);
		worstMemory->size -= chunk_size;
		
		pthread_mutex_unlock(&(worstMemory)->mutex);
		locker->unlockWrite();
		
		return worstFitAddress;	
	}
	
	return nullptr;
}

int bytesLeft()
{
	std::list<memory*>::iterator it;
	int bytes = 0;
	for(it = listFreed.begin(); it != listFreed.end(); it++)
	{
		bytes += (*it)->size;
	}
	return bytes;
}