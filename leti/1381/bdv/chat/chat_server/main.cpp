#include <QCoreApplication>

#include "tcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int max_threads;
    if(argc < 2)
        max_threads = 5;
    else
        max_threads = std::stoi(argv[1]);
    TcpServer srv(max_threads);
    srv.start(32165);

    return a.exec();
}
