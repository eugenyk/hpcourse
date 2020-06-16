CXXFLAGS += -Wall -Wextra -std=gnu++17
LFLAGS += -ltbb

main: main.o
	$(CXX) $(CXXFLAGS) main.o $(LFLAGS) -o main

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o


clean:
	rm main main.o *.out || true
