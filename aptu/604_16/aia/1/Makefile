CC = g++
FLAGS = -std=c++11 -Wall -Werror

all: ./bin/main 
	
./bin/main: ./src/main.cpp ./src/random_image.cpp ./src/random_image.h ./src/input_parser.h
	$(CC) $(FLAGS) $< ./src/random_image.cpp  -o $@

.PHONY: all clean

clean:
	rm -f ./bin/*