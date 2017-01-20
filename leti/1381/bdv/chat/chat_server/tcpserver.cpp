#include "tcpserver.h"

TcpServer::TcpServer(int max_threads, QObject *parent) : QTcpServer(parent)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    pool = new QThreadPool(0);
    pool->setMaxThreadCount(max_threads);
}

TcpServer::~TcpServer()
{
    for(int i = 0; i < sockets.size(); i++)
    {
        QTcpSocket* s = std::get<1>(sockets[i]);
        QMutex* m = std::get<2>(sockets[i]);
        delete s;
        delete m;
    }
    std::cout << "Sever finished" << std::endl;
    delete[] pool;
    google::protobuf::ShutdownProtobufLibrary();
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket();
    QMutex* mutex = new QMutex();
    std::tuple<std::string, QTcpSocket*, QMutex*> elem;
    std::get<1>(elem) = socket;
    std::get<2>(elem) = mutex;
    c_mutex.lock();
    sockets.push_back(elem);
    c_mutex.unlock();
    std::cout << "Incoming connection " << std::endl;
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, SIGNAL(readyRead()), this, SLOT(ready_read()), Qt::DirectConnection);
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
    ReadAndHandle* mytask = new ReadAndHandle(socket, &sockets, &c_mutex);
    mytask->setAutoDelete(true);

    connect(mytask, SIGNAL(need_move_to_thread(QTcpSocket*,QThread*)),
            this, SLOT(moveToThread(QTcpSocket*,QThread*)), Qt::BlockingQueuedConnection);
    pool->start(mytask);
}

void TcpServer::moveToThread(QTcpSocket* socket, QThread* thread)
{
    socket->moveToThread(thread);
}
