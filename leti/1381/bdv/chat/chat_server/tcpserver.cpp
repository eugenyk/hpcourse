#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
    pool = new QThreadPool(0);
    pool->setMaxThreadCount(5);
    //QThreadPool::globalInstance()->setMaxThreadCount(5);
}

TcpServer::~TcpServer()
{
    for(int i = 0; i < sockets.size(); i++)
        delete sockets[i].second;
    std::cout << "Sever finished" << std::endl;
    delete[] pool;
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket();
    std::pair<std::string, QTcpSocket*> elem;
    elem.second = socket;
    sockets.push_back(elem);
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, SIGNAL(readyRead()), this, SLOT(ready_read()));
}

void TcpServer::start(int portnum)
{
    port = portnum;
    if(listen(QHostAddress::Any, port))
    {
        std::cout << "Server started." << std::endl;
    }
    else
    {
        std::cout << "Starting server error." << std::endl;
    }
}

void TcpServer::ready_read()
{
    QTcpSocket* socket = (QTcpSocket*)QObject::sender();
    ReadAndHandle* mytask = new ReadAndHandle(socket, sockets);
    mytask->setAutoDelete(true);

    connect(mytask, SIGNAL(setting_username(QTcpSocket*,char*,int)), this, SLOT(set_username(QTcpSocket*,char*,int)));
    connect(mytask, SIGNAL(deleting_socket_from_store(QTcpSocket*)), this, SLOT(close_socket(QTcpSocket*)));

    pool->start(mytask);
}


void TcpServer::close_socket(QTcpSocket* socket)
{
    socket->close();
    delete_socket_from_store(socket);
    delete socket;
}

void TcpServer::delete_socket_from_store(QTcpSocket* sock)
{
    for(auto it = sockets.begin(); it != sockets.end(); it++)
    {
        std::pair<std::string, QTcpSocket*> elem = *it;
        if(elem.second == sock)
        {
            sockets.erase(it);
            break;
        }
    }
}

void TcpServer::set_username(QTcpSocket* socket, char* name, int size)
{
    std::string s(name, size);
    delete[] name;

    for(auto it = sockets.begin(); it != sockets.end(); it++)
    {
        if((*it).second == socket)
        {
            (*it).first = s;
            break;
        }
    }
}
