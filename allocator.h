#include <unistd.h>
#include <list>
#include "algorithms.h"

int sbrkUsed = 0;

void * alloc(size_t chunk_size)
{
	void * newAddress;
	
	if(listFreed.size() != 0)
	{
		if(strategy == FIRST_FIT)
		{
			newAddress = firstFit(chunk_size);
		}
		else if(strategy == WORST_FIT)
		{
			newAddress = worstFit(chunk_size);
		}
		else if(strategy == BEST_FIT)
		{
			newAddress = bestFit(chunk_size);
		}
	}
	
	if(newAddress == nullptr)
	{
		newAddress = sbrk(chunk_size);
		if(listFreed.size() != 0)
		{
			sbrkUsed++;
		}
	}
	
	return newAddress;
	
}

void dealloc(void * chunk)
{
	std::list<memory*>::iterator it;
	
	for(it = listAllocated.begin(); it != listAllocated.end(); it++)
	{
		if((*it)->address == chunk)
		{
			memory* freedPtr = *it;
			it = listAllocated.erase(it);
			listFreed.push_back(freedPtr);
		}
	}
}