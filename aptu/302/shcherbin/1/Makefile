CXX ?= g++
CXXFLAGS ?= -O2 -Wall -Wextra -Wshadow -Werror
LDFLAGS ?= -lpthread

all: producer_consumer.cpp
		$(CXX) producer_consumer.cpp $(CXXFLAGS) $(LDFLAGS) -o producer_consumer

clean:
		rm -f producer_consumer

.PHONY: clean all
