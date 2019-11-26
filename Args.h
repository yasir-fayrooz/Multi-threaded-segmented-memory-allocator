/*
* This struct takes the arguments for each thread. Each
* thread needs to know the details about the command line
* args and the amount of threads instantiated in order to
* split the work accordingly for each thread. i.e. if there
* are 4 threads and 1000 allocations, each thread will allocate
* 250 each to maximise efficiency and effectiveness.
*/

#ifndef ARGS_H
#define ARGS_H

struct Args {
    int max;
	int min;
	int size;
	int threadCount;
    Args(int _max, int _min, int _size, int _threadCount) : max(_max), min(_min), size(_size), threadCount(_threadCount) {}
};

#endif //ARGS_H