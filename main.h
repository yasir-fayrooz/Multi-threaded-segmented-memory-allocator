#include <stdlib.h>
#include <exception>
#include <chrono>
#include <time.h>
#include <iostream>

#include "memory.h"
#include "Args.h"

void show_usage();
void* memoryAllocThread(void* arg);
void runPreExperimentAllocation(int max, int min, int size);
void runExperiment(int threadCount, int max, int min, int size, int runCount);
void freeLists();

auto durationTotal = 0;
int sbrkUsedTotal = 0;
int bytesLeftTotal = 0; 