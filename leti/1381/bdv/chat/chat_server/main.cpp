#include "mainwindow.h"
#include <QApplication>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tcpserver.h"

int main(int argc, char *argv[])
{

    /*
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
    */

    TcpServer::start(32165);
    TcpServer::wait_clients(2);
    TcpServer::close_();
    /*std::cout << "From client: " << tcp_srv.receive() << std::endl;
    tcp_srv.send("Got your msg");
    tcp_srv.close_();
    */
   return 0;
}
