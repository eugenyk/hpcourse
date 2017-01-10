#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThreadPool>
#include <QMutex>
#include <vector>
//#include <tbb/concurrent_vector.h>
#include <iostream>

#include "readandhandle.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT

private:
    QThreadPool* pool;
    int port;
    int max_threads;
    std::vector<std::tuple<std::string, QTcpSocket*, QMutex*> > sockets;

protected:
    void incomingConnection(qintptr socketDescriptor);

public:
    explicit TcpServer(int max_threads, QObject *parent = 0);
    ~TcpServer();
    void start(int portnum);

    void delete_socket_from_store(QTcpSocket *sock, QMutex** mut);

    static bool getnameandmsg(std::string rec_data, std::string& name, std::string& msg);
    static void setnameandmsg(std::string& snd_data, std::string name, std::string msg);

    std::string find_name_by_sock(QTcpSocket* sock);
    QTcpSocket* find_sock_by_name(std::string name);

public slots:
    void ready_read();
    void close_socket(QTcpSocket* socket);
    void set_username(QTcpSocket* socket, char* name, int size);

};

#endif // TCPSERVER_H
