#include "ChatServer.h"

Poco::NotificationQueue queue;
CommandExecutor commandExecutor(queue);
std::vector<Poco::Net::StreamSocket> clients;
std::mutex mymutex;

int main(int argc, char** argv)
{
    ChatServer app;
    return app.run(argc, argv);
}