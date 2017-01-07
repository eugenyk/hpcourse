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
    int max_clients;
    if(argc < 2)
    {
        std::cout << "Usage:\nchat_server max_clients\nExample:\nchat_server 10" << std::endl;
        return -1;
    }
    else
        max_clients = std::stoi(argv[1]);
    if(max_clients < 1)
        return -1;
    TcpServer srv;
    srv.start(32165);
    srv.wait_clients(max_clients);
    srv.close_();
    return 0;
}
