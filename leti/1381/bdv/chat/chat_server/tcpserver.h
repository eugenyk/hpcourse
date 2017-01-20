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
#include <QtConcurrent/QtConcurrent>
#include "readandhandle.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT

private:
    QThreadPool* pool;
    int port;
    int max_threads;
    std::vector<std::tuple<std::string, QTcpSocket*, QMutex*> > sockets;
    QMutex c_mutex;

protected:
    void incomingConnection(qintptr socketDescriptor);

public:
    explicit TcpServer(int max_threads, QObject *parent = 0);
    ~TcpServer();
    void start(int portnum);

public slots:
    void moveToThread(QTcpSocket* socket, QThread* thread);
    void ready_read();
};

#endif // TCPSERVER_H
