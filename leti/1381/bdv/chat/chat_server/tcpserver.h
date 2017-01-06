#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <string>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

class TcpServer
{
private:
    static int socket_descr;
    static int new_socket_descr[10];
    static pthread_t threads[10];
    static int clients_number;
    static int port;
    //char buffer[256];
    static struct sockaddr_in server_addr;
    static struct sockaddr_in client_addr;

    TcpServer();
public:
    static void start(int portnum);
    static void *wait_client(void *id);
    static void wait_clients(int max_number);
    static void send(std::string msg, int receiver_id);
    static std::string receive(int sender_id);
    static void close_();
};

#endif // TCPSERVER_H
