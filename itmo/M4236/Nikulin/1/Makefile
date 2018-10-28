.PHONY: all test clean

CXX = g++
CXXFLAGS += -Wall -Werror -pedantic -pthread -std=c++03

PWD = $(shell pwd)
ROOT = $(PWD)
BIN_DIR = $(ROOT)/bin

target = $(BIN_DIR)/main.exe

all: $(target)

$(target):  template.cpp test.sh Makefile
	$(CXX) $(CXXFLAGS) template.cpp -o $(target)

test: $(target)
	bash test.sh $(target)

clean:
	rm $(target)

