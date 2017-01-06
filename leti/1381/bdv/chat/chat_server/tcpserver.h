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

class TcpServer
{
    struct NameId
    {
        std::string name;
        int id;
    };

private:
    static int socket_descr;
    static int new_socket_descr[10];
    static pthread_t threads[10];
    static int clients_number;
    static int port;
    static struct sockaddr_in server_addr;
    static struct sockaddr_in client_addr;
    static std::vector<NameId> name_id;

    TcpServer();
public:
    static void start(int portnum);
    static void *wait_client(void *id);
    static void wait_clients(int max_number);
    static void send(std::string msg, int receiver_id);
    static std::string receive(int sender_id);
    static void close_();

    static bool getnameandmsg(std::string rec_data, std::string& name, std::string& msg);
    static void setnameandmsg(std::string& snd_data, std::string name, std::string msg);
    static std::string findNameById(int id);
    static int findIdByName(std::string name);
};

#endif // TCPSERVER_H
