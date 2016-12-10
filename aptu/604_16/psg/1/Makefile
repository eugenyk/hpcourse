run_cmd=
PROG=flowgraph
ARGS=

# icpc by default
ifneq (,$(shell which icc 2>/dev/null))
    CXX=icpc
endif

CXX0XFLAGS ?= -std=c++0x

ifeq ($(shell uname), Linux)
LIBS+= -lrt -lpthread
else ifeq ($(shell uname), Darwin)
override CXXFLAGS += -Wl,-rpath,$(TBBROOT)/lib
endif

all: release test

release:
	$(CXX) -O2 -DNDEBUG *.cpp $(CXXFLAGS) -o $(PROG) -ltbb_debug $(LIBS) $(CXX0XFLAGS)

debug:
	$(CXX) -O0 -g -DTBB_USE_DEBUG *.cpp $(CXXFLAGS) -o $(PROG) -ltbb_debug $(LIBS)  $(CXX0XFLAGS)

clean:
	$(RM) $(PROG) *.o *.d

test:
	$(run_cmd) ./$(PROG) $(ARGS)

perf_build: release

perf_run:
	$(run_cmd) ./$(PROG)
