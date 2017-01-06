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
    TcpServer::start(32165);
    TcpServer::wait_clients(2);
    TcpServer::close_();
    */
    TcpServer srv;
    srv.start(32165);
    srv.wait_clients(2);
    srv.close_();
    return 0;
}
