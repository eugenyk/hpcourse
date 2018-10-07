all: a.out

a.out: main.o
	g++ -pthread main.o

main.o: main.cpp
	g++ -c main.cpp -std=c++11

