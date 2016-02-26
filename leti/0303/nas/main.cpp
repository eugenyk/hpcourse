#include "ChatServer.h"

Poco::NotificationQueue queue;
CommandExecutor commandExecutor(queue);
std::vector<Poco::Net::StreamSocket> clients;

int main(int argc, char** argv)
{
    ChatServer app;
    return app.run(argc, argv);
}