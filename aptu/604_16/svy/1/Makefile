FLAGS = --std=c++11 -ltbb
CXX = g++
EXE = main
SRCMAIN = main.cpp

all: $(EXE)

$(EXE): $(SRCMAIN)
	$(CXX) $(SRCMAIN) -o $(EXE) $(FLAGS)

clean:
	rm main

.PHONY: clean all
