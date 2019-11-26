#ifndef MEMORY_H
#define MEMORY_H

#include <pthread.h>

struct memory {
    void * address;
	size_t size;
	pthread_mutex_t mutex;
    memory(void * _address, size_t _size) : address(_address), size(_size) {pthread_mutex_init(&mutex, NULL);}
};

#endif //MEMORY_H