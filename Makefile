HEADERS=main.h Locker.h Args.h memory.h
SOURCES=main.cpp Locker.cpp algorithms.h allocator.h
OBJECTS=main.o Locker.o

CC=g++

CFLAGS= -Wall -pedantic -pthread -std=c++17
LDFLAGS=

output: assignment2

assignment2: $(OBJECTS)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)
	
%.o: %.cpp $(HEADERS)
	$(CC) $(CXXFLAGS) -c $<