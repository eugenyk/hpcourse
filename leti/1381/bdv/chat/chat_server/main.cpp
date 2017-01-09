#include <QCoreApplication>

#include "tcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int max_clients;
    TcpServer srv;
    srv.start(32165);

    return a.exec();
}
