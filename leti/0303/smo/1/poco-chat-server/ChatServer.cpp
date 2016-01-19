#include "ChatServer.h"
#include "CommandExecutor.h"

int ChatServer::main(const std::vector<std::string>& args)
{
    unsigned short port = 10000;
    Poco::Net::ServerSocket svs(port);
    Poco::Net::SocketReactor reactor;
    
    Poco::Net::ParallelSocketAcceptor<ChatServerHandler, Poco::Net::SocketReactor> acceptors(svs, reactor, 4);
    
    Poco::Thread reactorThread, commandThread;
    reactorThread.start(reactor);
    
//    Poco::NotificationQueue queue;
//    CommandExecutor commandExecutor(queue);


    commandThread.start(commandExecutor);
    waitForTerminationRequest();
    queue.wakeUpAll();
    reactor.stop();
    reactorThread.join();
    commandThread.join();
    
    return Application::EXIT_OK;
}
