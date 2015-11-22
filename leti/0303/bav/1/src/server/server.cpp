#include <QtCore/QCoreApplication>
#include <QtNetwork/QTcpServer>

#include "socketdecorator.h"
#include "threadpool.h"
#include "server.h"

Server* Server::instance()
{
    static Server instance_;
    return &instance_;
}

Server::Server(QObject* parent) : QTcpServer(parent), threadPool_(new ThreadPool)
{
    // for using metaobject system
    qRegisterMetaType<server::proto::Message>("sever::proto::Message");

    setMaxPendingConnections(100); // default is 30
    connect(this, &QTcpServer::acceptError, this, &Server::printError);
}

Server::~Server()
{
    stop();
    delete threadPool_;
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

void Server::incomingConnection(qintptr handle)
{
    QPointer<IOSocketDecorator> socket = new IOSocketDecorator(handle);
    connect(socket.data(), &IOSocketDecorator::disconnected, this, &Server::socketDisconnected);

    QThread* thread = threadPool_->acquireThread();
    socket->moveToThread(thread);

    rwmutex_.lockForWrite();
    clients_.insert(handle, socket);
    rwmutex_.unlock();

    socket->initAsync();
}

void Server::socketDisconnected()
{
    IOSocketDecorator* socket = qobject_cast<IOSocketDecorator*>(sender());
    Q_ASSERT(socket);
    if(Q_LIKELY(socket))
    {
        QThread* thread = socket->thread();
        Q_ASSERT(thread);

        rwmutex_.lockForWrite();
        clients_.remove(socket->descriptor_);
        rwmutex_.unlock();

        socket->deleteLater();

        if(Q_LIKELY(thread))
            threadPool_->releaseThread(thread);
    }
}

void Server::printError(QAbstractSocket::SocketError error)
{
    if(error != QAbstractSocket::UnknownSocketError && error != QAbstractSocket::RemoteHostClosedError)
        qWarning() << errorString();
}

void Server::broadcast(const QPointer<IOSocketDecorator>& from, const server::proto::Message& msg)
{
    QReadLocker lock(&rwmutex_);
    auto it = clients_.cbegin();
    while(it != clients_.cend())
    {
        auto client = it.value();
        if(client)
        {
            if(client != from)
                client->writeAsync(msg);
        }

        it++;
    }
}

void Server::setThreadPoolSize(quint32 size)
{
    threadPool_->setMaxThreadCount(size);
}

quint32 Server::threadPoolSize() const
{
    return threadPool_->maxThreadCount();
}
