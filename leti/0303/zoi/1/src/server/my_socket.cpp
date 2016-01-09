#include <QtCore/QMetaMethod>

#include "command_thread.h"
#include "socket.h"
#include "server.h"

#include "my_socket.h"

IOSocketDecorator::IOSocketDecorator(qintptr descriptor, QObject* parent)
    : QObject(parent), descriptor_(descriptor), socket_(nullptr)
{
}

IOSocketDecorator::~IOSocketDecorator()
{
    socket_ = nullptr;
}

void IOSocketDecorator::writeAsync(const server::proto::Message& msg)
{
    static QByteArray normalizedSignature = QMetaObject::normalizedSignature("send(const server::proto::Message&)");
    int methodIndex = metaObject()->indexOfMethod(normalizedSignature);
    if(methodIndex != -1)
    {
        if(!metaObject()->method(methodIndex).invoke(this, Qt::QueuedConnection, Q_ARG(server::proto::Message, msg)))
            qWarning() << Q_FUNC_INFO << false;
    }
    else
        qWarning() << Q_FUNC_INFO << -1;
}

void IOSocketDecorator::initAsync()
{
    static QByteArray normalizedSignature = QMetaObject::normalizedSignature("init()");
    int methodIndex = metaObject()->indexOfSlot(normalizedSignature);
    if(methodIndex != -1)
    {
        if(!metaObject()->method(methodIndex).invoke(this, Qt::QueuedConnection))
            qWarning() << Q_FUNC_INFO << normalizedSignature << false;
    }
    else
        qWarning() << Q_FUNC_INFO << normalizedSignature << -1;
}

void IOSocketDecorator::init()
{
    if(socket_)
    {
        socket_->deleteLater();
        socket_ = nullptr;
    }

    QTcpSocket* socket = new QTcpSocket(this);
    if(!socket->setSocketDescriptor(descriptor_))
    {
        qWarning() << Q_FUNC_INFO << socket->peerAddress() << socket->errorString();
        delete socket;

        emit disconnected();
    }
    else
    {
        socket_ = new IOSocket(socket, this);
        connect(socket_, &IOSocket::read, this, &IOSocketDecorator::receive);
        connect(socket_, &IOSocket::disconnected, this, &IOSocketDecorator::disconnected);
        connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error()));
    }
}

void IOSocketDecorator::receive(const server::proto::Message& message)
{
    if(message.text().substr(0, 3).compare("/c ") == 0)
    {
        server::proto::Message command(message);
        std::string* text = command.mutable_text();
        *text = text->substr(3); // skip '/c '
        ExecutionThread::instance()->enqueueCommand(this, command);
    }
    else
        Server::instance()->broadcast(this, message);
}

void IOSocketDecorator::send(const server::proto::Message& message)
{
    if(!socket_)
        return;

    if ((socket_->state() == QAbstractSocket::ConnectedState) && (!socket_->write(message)))
            error();
}

void IOSocketDecorator::error()
{
    if(socket_->error() != QAbstractSocket::UnknownSocketError && socket_->error() != QAbstractSocket::RemoteHostClosedError)
        qWarning() << socket_->peerAddress() << socket_->errorString();
}
