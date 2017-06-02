srcdir = src
bindir = bin
objdir = obj

CXX = g++
CXXFLAGS = -Wall -Wpedantic -std=c++11

all: $(bindir) $(objdir) main

sources = main.cpp utils.cpp image.cpp
objects = $(sources:.cpp=.o)

debug: CXXFLAGS += -DDEBUG
debug: all

$(bindir):
	mkdir -p $(bindir)

$(objdir):
	mkdir -p $(objdir)

main: $(objects)
	$(CXX) $(addprefix $(objdir)/, $(objects)) $(CXXFLAGS) -ltbb -Wl,-rpath,$(TBB_LIBRARY_RELEASE) -L$(TBB_LIBRARY_RELEASE) -I$(TBB_INCLUDE) -o $(bindir)/main

%.o: $(srcdir)/%.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $(objdir)/$@ -I$(TBB_INCLUDE)


.PHONY: clean
clean:
	rm -rf $(bindir)
	rm -rf $(objdir)
