#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

class TcpServer;

struct wait_client_args
{
    int socket_descr;
    TcpServer* srv;
};

void *wait_client(void *ar);

class TcpServer
{
    struct NameSD
    {
        std::string name;
        int socket_descriptor;
    };

private:
    int socket_descr;
    //int new_socket_descr[10];
    pthread_t threads[10];
    int clients_number;
    int port;
    struct sockaddr_in server_addr;
    //struct sockaddr_in client_addr;
    std::vector<struct NameSD> name_id;
    wait_client_args* wcargs;

public:
    TcpServer();
    ~TcpServer();
    void start(int portnum);
    void wait_clients(int max_number);
    void send(std::string msg, int socket_descriptor);
    std::string receive(int socket_descriptor);
    void close_();
    static bool getnameandmsg(std::string rec_data, std::string& name, std::string& msg);
    static void setnameandmsg(std::string& snd_data, std::string name, std::string msg);
    std::string findNameBySD(int socket_descriptor);
    int findSDByName(std::string name);
    void add_client(std::string name, int socket_descriptor);
};

#endif // TCPSERVER_H
