all: main

CCFLAGS= -O2 -Wall -Wextra -Wshadow -Wpedantic -lpthread

main.o: main.cpp
	g++ $(CCFLAGS) -c main.cpp -o main.o

main: main.o
	g++ $(CCFLAGS) main.o -o main

clean:
	rm -f *.o main
