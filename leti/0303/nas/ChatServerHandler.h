#ifndef ChatServerHandler_h
#define ChatServerHandler_h

#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/SocketReactor.h>
#include <Poco/Net/SocketNotification.h>
#include <Poco/NObserver.h>
#include <Poco/NotificationQueue.h>

#include "CommandExecutor.h"

extern Poco::NotificationQueue queue;
extern CommandExecutor commandExecutor;
extern std::vector<Poco::Net::StreamSocket> clients;
extern std::mutex mymutex;

class ChatServerHandler
{
public:
    ChatServerHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor);
    ~ChatServerHandler();
    
    void onReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
    void onShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
    
    void sendBroadcastMesage(const std::string sender, const std::string message);
    
private:
    enum
    {
        BUFFER_SIZE = 1024
    };
    
    Poco::Net::StreamSocket   _socket;
    Poco::Net::SocketReactor& _reactor;
    
};

#endif /* ChatServerHandler_h */
