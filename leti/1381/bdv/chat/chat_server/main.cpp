#include <QCoreApplication>

#include "tcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int max_threads;
    if(argc < 2)
    {
        std::cout << "Usage:\ntcp_server 5\n5 - max threads.\n";
        return -1;
    }
    else
        max_threads = std::stoi(argv[1]);
    TcpServer srv(max_threads);
    srv.start(32165);

    return a.exec();
}
