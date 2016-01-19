LDFLAGS=-lPocoUtil -lPocoFoundation -lPocoNet -lprotobuf
#HEADERS=-I/Users/dimak/poco-1.6.1/Util/include -I/Users/dimak/poco-1.6.1/Foundation/include -I/Users/dimak/poco-1.6.1/Net/include -I/Users/dimak/protobuf/src
#LIBS=-L/Users/dimak/poco-1.6.1/lib/Darwin/x86_64 -L/Users/dimak/protobuf/src/.libs

all:
	g++ $(LDFLAGS) $(HEADERS) $(LIBS) Message.pb.cc ChatServer.cpp ChatServerHandler.cpp CommandExecutor.cpp main.cpp -o server
