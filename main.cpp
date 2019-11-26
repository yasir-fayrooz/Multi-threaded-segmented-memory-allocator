#include "main.h"
#include "allocator.h"
#include <pthread.h>
#include "StratEnum.h"

void show_usage()
{
    std::cerr
              << "How to use:\n"
              << "\tThis program takes args:\n"
			  << "\t/[program name] [-s] [sample size] [minimum memory size] [maximum memory size] [-a] [strategy] [-t] [threadCount] -r [runCount]\n"
			  << "\t--------------------------------------------------------------\n"
			  << "\t[sample size] dictates the amount of memory to generate\n"
			  << "\t[minimum memory size] dictates the minimum size of each memory in bytes to generate\n"
			  << "\t[maximum memory size] dictates the maximum size of each memory in bytes to generate\n"
			  << "\t[strategy] strategies to choose from: 'firstfit', 'bestfit' or 'worstfit'\n"
			  << "\t[threadCount] the amount of threads to launch\n"
		      << "\t[runCount] the amount of times the experiment should run\n"
			  << "\t--------------------------------------------------------------\n"
              << "\t i.e: ./program -s 100 1 4096 -a firstfit -t 4 -r 10\n"
              << std::endl;
}


/*
* This is the thread used to allocate memory
* This thread is spawned x amount of times according
* to the command line arguments presented. It then
* takes an Args.h struct as its parameter to pass in
* multiple arguments and information needed for each thread.
* It pushes back memory to the list which then runs either firstfit,
* worstfit or bestfit methods in algorithms.h and the locking is done
* in each method to prevent deadlocks/race conditions.
*/
void* memoryAllocThread(void* arg)
{
	
	Args* args = (Args*)arg;
	
	for(int i = 0; i < args->size / args->threadCount; i++)
	{
		int mem_size = rand() % args->max + args->min;
		listAllocated.push_back(new memory(alloc(mem_size), mem_size));
	}
	
	return nullptr;
}

int main(int argc, char* argv[]) {
	
	// ARGS CODE BEGIN
	
	int size;
	int min;
	int max;
	int threadCount;
	int runCount;
	
	if(argc != 11)
	{
		show_usage();
		return 1;
	}
	else
	{
		std::string s = argv[1];
		std::string s_size = argv[2];
		std::string s_min = argv[3];
		std::string s_max = argv[4];
		std::string a = argv[5];
		std::string strat = argv[6];
		std::string t = argv[7];
		std::string thread_count = argv[8];
		std::string r = argv[9];
		std::string run_count = argv[10];
		
		try
		{
			size = std::stoi(s_size);
			min = std::stoi(s_min);
			max = std::stoi(s_max);
			threadCount = std::stoi(thread_count);
			runCount = std::stoi(run_count);
			
			if(size <= 0 || min >= max || min <= 0 || threadCount <= 0)
			{
				show_usage();
				return 1;
			}
		}
		catch(std::exception& e)
		{
			show_usage();
			return 1;
		}
		
		if(s == "-s" && 
		  (strat == "firstfit" ||
		   strat == "worstfit" ||
		   strat == "bestfit") &&
		   a == "-a" && 
		   t == "-t" &&
		   r == "-r")
	    {
			if(strat == "firstfit")
				strategy = FIRST_FIT;
			else if(strat == "worstfit")
				strategy = WORST_FIT;
			else if(strat == "bestfit")
				strategy = BEST_FIT;
			else
				strategy = INVALID;
	    }
		else
		{
			show_usage();
			return 1;
		}
	}
	
	// ARGS CODE END
	
	
	//ALLOC CODE BEGIN
	srand ( time(NULL) );
	for(int i = 0; i < runCount; i++)
	{
		runPreExperimentAllocation(max, min, size);
	
		runExperiment(threadCount, max, min, size, i);

		freeLists();
	}
	std::cout << "-------------------------------------------------------" << std::endl;
	std::cout << "|                Final Average Report                 |" << std::endl;
	std::cout << "-------------------------------------------------------" << std::endl;
	std::cout << "Average duration (in microseconds): " << durationTotal / runCount << std::endl;
	std::cout << "Average sbrk used: " << sbrkUsedTotal / runCount << std::endl;
	std::cout << "Average bytes left in free list: " << bytesLeftTotal / runCount << std::endl;
	//ALLOC CODE END
	
	return EXIT_SUCCESS;
}

void runPreExperimentAllocation(int max, int min, int size)
{
	std::cout << "-------------------------------------------------------" << std::endl;
	std::cout << "Allocating pre-experiment list of size " << size << "..." << std::endl;
	for(int i = 0; i < size; i++)
	{
		int mem_size = rand() % max + min;
		listAllocated.push_back(new memory(alloc(mem_size), mem_size));
	}
	
	std::cout << "Deallocating the pre-experiment list..." << std::endl;
	while(listAllocated.size() > 0)
	{
		dealloc(listAllocated.front()->address);
	}
}

void runExperiment(int threadCount, int max, int min, int size, int runCount)
{
	//array of thread ids
	pthread_t *thread_id = new pthread_t[threadCount];
	
	std::cout << "Allocating experiment list of size " << size << "..." << std::endl;
	auto startTime = std::chrono::high_resolution_clock::now();
	
	//launch the threads
	for (int count = 0; count < threadCount; ++count) 
	{
		int result = pthread_create(&thread_id[count], NULL, memoryAllocThread, new Args(max,min,size,threadCount));
		if (result != 0) 
		{
			std::cout << "Error creating threads" << std::endl;
			exit(1);
		}
	}
	//join the threads
	for (int count = 0; count < threadCount; ++count)
	{
		int result = pthread_join(thread_id[count], NULL);
		if (result != 0)
		{
			std::cout << "Error joining threads" << std::endl;
			exit(1);
		}
	}
	
	auto endTime = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>( endTime - startTime ).count();
	std::cout << "-------------------------------------------------------" << std::endl;
	std::cout << "|                    Report " << runCount + 1 << "                         |" << std::endl;
	std::cout << "-------------------------------------------------------" << std::endl;
	std::cout << "Sample size: " << size << " Bytes from: " << min << " - "  << max << std::endl;
	std::cout << "Thread count: " << threadCount << std::endl;
	std::cout << "Duration of re-allocation (in microseconds): " << duration << std::endl;
	std::cout << "Amount of times sbrk used when free list is not empty: " << sbrkUsed << std::endl;
	int bytesRemaining = bytesLeft();
	std::cout << "Bytes left in free list: " << bytesRemaining << std::endl;
	
	durationTotal += duration;
	sbrkUsedTotal += sbrkUsed;
	bytesLeftTotal += bytesRemaining;
	
	delete[] thread_id;
}

void freeLists()
{
	listAllocated.clear();
	listFreed.clear();
	sbrkUsed = 0;
}


