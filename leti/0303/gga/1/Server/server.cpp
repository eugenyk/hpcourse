#include <QtCore/QCoreApplication>
#include <QtNetwork/QTcpServer>

#include "socketdecorator.h"
#include "server.h"



Server* Server::instance()
{
    static Server instance_;
    return &instance_;
}

Server::Server(QObject* parent) : QTcpServer(parent)
{
    connect(this, &QTcpServer::acceptError, this, &Server::printError);

    m_threadPool=new QThreadPool(this);
}

Server::~Server()
{
    stop();
    delete m_threadPool;
}

bool Server::start(quint16 port)
{
    if(isListening())
        return false;

    if(!listen(QHostAddress::AnyIPv4, port == 0 ? 10000 : port))
    {
        qWarning() << errorString();
        return false;
    }
    else
    {
        qWarning() << "Start listening port" << serverPort();
        return true;
    }
}

void Server::stop()
{
    if(isListening())
        close();

    QWriteLocker lock(&rwmutex_);
    auto it = clients_.begin();
    while(it != clients_.end())
    {
        if(it.value())
            it.value()->deleteLater();
        it++;
    }
    clients_.clear();
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QPointer<SocketDecorator> socket = new SocketDecorator(socketDescriptor);
    socket->setAutoDelete(true);

    connect(socket.data(), &SocketDecorator::disconnected, this, &Server::socketDisconnected);

    rwmutex_.lockForWrite();
    clients_.insert(socketDescriptor, socket);
    rwmutex_.unlock();

    m_threadPool->start(socket);
}

void Server::socketDisconnected()
{
    SocketDecorator* socket = qobject_cast<SocketDecorator*>(sender());
    Q_ASSERT(socket);
    if(Q_LIKELY(socket))
    {
        rwmutex_.lockForWrite();
        clients_.remove(socket->descriptor_);
        rwmutex_.unlock();

        socket->wait_loop->quit();
    }
 }

void Server::printError(QAbstractSocket::SocketError error)
{
    if(error != QAbstractSocket::UnknownSocketError && error != QAbstractSocket::RemoteHostClosedError)
        qWarning() << errorString();
}

void Server::broadcast(const QPointer<SocketDecorator>& from, const server::proto::Message& msg)
{
    QReadLocker lock(&rwmutex_);
    auto it = clients_.cbegin();
    while(it != clients_.cend())
    {
        auto client = it.value();
        if(client)
        {
            if(client != from)
                client->m_writeAsync(msg);
        }

        it++;
    }
}

void Server::setThreadPoolSize(quint32 size)
{
    m_threadPool->setMaxThreadCount(size);
}

quint32 Server::threadPoolSize() const
{
    return m_threadPool->maxThreadCount();
}
