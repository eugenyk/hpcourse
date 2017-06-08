srcdir = src
bindir = bin
objdir = obj
incdir = src/include

CXX = g++
CXXFLAGS = -Wall -Wpedantic -std=c++11

all: $(bindir) $(objdir) main

sources = main.cpp
objects = $(sources:.cpp=.o)

debug: CXXFLAGS += -DDEBUG
debug: all

$(bindir):
	mkdir -p $(bindir)

$(objdir):
	mkdir -p $(objdir)

main: $(objects)
	$(CXX) $(CXXFLAGS) -pthread $(addprefix $(objdir)/, $(objects)) -o $(bindir)/main

%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $(objdir)/$@ -I./src/include


.PHONY: clean
clean:
	rm -rf $(bindir)
	rm -rf $(objdir)
