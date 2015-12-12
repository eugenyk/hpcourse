CC = g++

CFLAGS = -Wall -Ofast -mtune=native --std=c++11 -static

LFLAGS = -L./boost/install_out/lib/ -L./protobuf/install_out/lib/
LIBS =  -lboost_system -lboost_thread -lprotobuf -lpthread

INCLUDES = -I"./protobuf/install_out/include" -I"./boost/install_out/include"

MAIN = chat_server

.PHONY: clean

all:    $(MAIN)
	@echo $(MAIN) compiled!

$(MAIN): main.o message.o
	$(CC) $(CFLAGS) $(LFLAGS) -o $(MAIN) main.o message.o $(LIBS)

main.o: src/main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c src/main.cpp -o main.o

message.o: src/message.pb.cc
	$(CC) $(CFLAGS) $(INCLUDES) -c src/message.pb.cc -o message.o

clean:
	$(RM) *.o *~ $(MAIN)
