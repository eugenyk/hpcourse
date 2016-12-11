CC = g++
FLAGS = -std=c++11 -Wall -g
LIBS = -ltbb

all: ./bin/main 
	
./bin/main: ./src/main.cpp ./src/random_image.cpp ./src/random_image.h ./src/input_parser.h
	$(CC) $(FLAGS) $< ./src/random_image.cpp  -o $@ $(LIBS)

.PHONY: all clean

clean:
	rm -f ./bin/*