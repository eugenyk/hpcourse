#include "ChatServer.h"
#include "CommandExecutor.h"

int ChatServer::main(const std::vector<std::string>& args)
{
    unsigned short port = 10000;
    unsigned short threads = 4;
    
    Poco::Net::ServerSocket svs(port);
    Poco::Net::SocketReactor reactor;
    
    Poco::Net::ParallelSocketAcceptor<ChatServerHandler, Poco::Net::SocketReactor> acceptors(svs, reactor, threads);
    
    Poco::Thread reactorThread, commandThread;
    reactorThread.start(reactor);
    
    commandThread.start(commandExecutor);
    waitForTerminationRequest();
    queue.wakeUpAll();
    reactor.stop();
    reactorThread.join();
    commandThread.join();
    
    return Application::EXIT_OK;
}
