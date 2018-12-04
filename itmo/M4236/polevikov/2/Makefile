ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
LD_LIBRARY_PATH = $(ROOT_DIR)/lib/libcds-release/bin

CXXFLAGS = -I$(ROOT_DIR)/lib/libcds -pthread
#CXXFLAGS += -fPIC -fsanitize=thread
#CXXFLAGS += -fPIC -fsanitize=address,undefined
#CXXFLAGS += -g -fno-omit-frame-pointer -fno-optimize-sibling-calls
LDFLAGS = -lboost_thread -L$(LD_LIBRARY_PATH) -lcds
CC = g++

all:
	$(CC) test/test.cpp -o bin/run_test $(CXXFLAGS) $(LDFLAGS)

clean:
	rm bin/run_test
