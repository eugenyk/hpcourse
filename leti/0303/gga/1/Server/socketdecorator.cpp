#include <QtCore/QMetaMethod>

#include "waitingthread.h"
#include "socket_handler.h"
#include "server.h"

#include "socketdecorator.h"


SocketDecorator::SocketDecorator(qintptr descriptor, QObject* parent)
    : QObject(parent), descriptor_(descriptor), socket_handler(nullptr)
{
    qDebug() << "run constructor:"<< QThread::currentThread();
}

SocketDecorator::~SocketDecorator()
{
    socket_handler = nullptr;
}


void SocketDecorator::run()
{
    wait_loop=new QEventLoop();

    qDebug() << "run started"<< QThread::currentThread();
    if(socket_handler)
    {
        socket_handler->deleteLater();
        socket_handler = nullptr;
    }

    socket = new QTcpSocket();
    if(!socket->setSocketDescriptor(descriptor_))
    {
        qWarning() << Q_FUNC_INFO << socket->peerAddress() << socket->errorString();
        delete socket;

        emit disconnected();
    }
    else
    {
        socket_handler = new Socket_Handler(socket);

        connect(socket_handler, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error()));
        connect(socket_handler, &Socket_Handler::read, this, &SocketDecorator::receive, Qt::QueuedConnection);
        connect(socket_handler, &Socket_Handler::disconnected, this, &SocketDecorator::disconnected, Qt::QueuedConnection);
        connect(this,SIGNAL(m_writeAsync(server::proto::Message)),this,SLOT(writeAsync(server::proto::Message)), Qt::QueuedConnection);

        while (socket->waitForReadyRead(-1)){}
        wait_loop->exec();      //wait for full unlock and after destroy this object

    }


}

void SocketDecorator::writeAsync(const server::proto::Message& msg)
{
    socket_handler->write(msg);
}



void SocketDecorator::receive(const server::proto::Message& message)
{
    if(message.text().substr(0, 3).compare("/c ") == 0)
    {
        server::proto::Message command(message);
        std::string* text = command.mutable_text();
        *text = text->substr(3); // skip '/c '
        WaitingThread::instance()->enqueueCommand(this, command);
    }
    else
        Server::instance()->broadcast(this, message);
}

void SocketDecorator::send(const server::proto::Message& message)
{
    if(!socket_handler)
        return;

    if(socket_handler->state() == QAbstractSocket::ConnectedState)
        if(!socket_handler->write(message))
            error();
}

void SocketDecorator::error()
{
    if(socket_handler->error() != QAbstractSocket::UnknownSocketError && socket_handler->error() != QAbstractSocket::RemoteHostClosedError)
        qWarning() << socket_handler->peerAddress() << socket_handler->errorString();
}
