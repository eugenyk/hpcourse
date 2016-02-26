#ifndef ChatServer_h
#define ChatServer_h

#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/ParallelSocketAcceptor.h>
#include <Poco/Thread.h>

#include "ChatServerHandler.h"

extern Poco::NotificationQueue queue;
extern CommandExecutor commandExecutor;
extern std::vector<Poco::Net::StreamSocket> clients;

class ChatServer: public Poco::Util::ServerApplication
{
public:
    ChatServer(){}
    ~ChatServer(){}
    
protected:
    
    int main(const std::vector<std::string>& args);
};

#endif /* ChatServer_h */
