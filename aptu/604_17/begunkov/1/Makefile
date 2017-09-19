PATH_COMMON := ../common
PATH_COMMON_INC := $(PATH_COMMON)/include

CXXFLAGS += -Wall -Werror -pthread
CXXFLAGS += -I$(PATH_COMMON_INC)

all: task1
	@

task1: main.cpp $(PATH_COMMON_INC)/state.h
	$(CXX) $(CXXFLAGS) main.cpp -o ./bin/main.out

run: task1
	./bin/main.out

clean:
	rm -r ./bin/*.out

.PHONY: all task1 run clean
