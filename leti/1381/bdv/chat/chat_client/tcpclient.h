#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class TcpClient
{
private:
    int socket_descr;
    int port;
    struct sockaddr_in server_addr;
    struct hostent* server;

public:
    TcpClient();
    bool connect_(int portnum);
    void send(std::string msg);
    std::string receive();
    void close_();


};

#endif // TCPCLIENT_H
