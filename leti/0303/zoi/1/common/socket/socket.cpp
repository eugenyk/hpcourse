#include <QtNetwork/QHostAddress>
#include <QtEndian>

#include "Message.pb.h"
#include "socket.h"

template<typename T>
static inline T castWithEndian(T src)
{
    if(IOSocket::endian() == QSysInfo::BigEndian)
        return qToBigEndian(src);
    else
        return qToLittleEndian(src);
}

static QSysInfo::Endian endian = QSysInfo::ByteOrder;

QSysInfo::Endian IOSocket::endian()
{
    return ::endian;
}

void IOSocket::setEndian(QSysInfo::Endian endian)
{
    ::endian = endian;
}

IOSocket::IOSocket(QTcpSocket* socket, QObject* parent) : QObject(parent), socket_(socket), lastPacketTimestamp_(QDateTime::currentDateTime()), size_(0)
{
    connect(socket_.data(), SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError)));
    connect(socket_.data(), &QTcpSocket::disconnected, this, &IOSocket::disconnected);
    connect(socket_.data(), &QTcpSocket::connected, this, &IOSocket::connected);
    connect(socket_.data(), &QTcpSocket::readyRead, this, &IOSocket::readyRead);
}

IOSocket::~IOSocket()
{
}

QAbstractSocket::SocketState IOSocket::state() const
{
    return socket_ ? socket_->state() : QAbstractSocket::UnconnectedState;
}

QAbstractSocket::SocketError IOSocket::error() const
{
    return socket_ ? socket_->error() : QAbstractSocket::UnknownSocketError;
}

bool IOSocket::write(const server::proto::Message& message)
{
    if(!socket_)
        return false;

    qint32 size = castWithEndian((qint32)message.ByteSize());

    QByteArray buffer;
    buffer.reserve(message.ByteSize() + sizeof(qint32));
    buffer.append((char*)&size, sizeof(qint32));
    buffer.append(message.SerializeAsString().c_str(), message.ByteSize());

    return (socket_->write(buffer.constData(), buffer.size()) > 0);
}

void IOSocket::readyRead()
{
    QDateTime timestamp = QDateTime::currentDateTime();
    if(lastPacketTimestamp_.msecsTo(timestamp) > 1000)
    {
        buffer_.clear();
        size_ = 0;
    }

    lastPacketTimestamp_.swap(timestamp);

    QByteArray chunk = socket_->readAll();

    // try to read the size of the message
    // if we still don't have one
    if(size_ == 0)
    {
        size_ = castWithEndian(*((qint32*)chunk.left(sizeof(qint32)).constData()));
        if(size_ < 0)
            size_ = 0;

        // remove the first 4 bytes containing the size
        if(size_)
            chunk.remove(0, sizeof(qint32));
    }

    // appen current chunk to the buffer
    qint32 needBytes = size_ - buffer_.size();
    if(needBytes > 0)
        buffer_.append(chunk.left(needBytes));

    server::proto::Message message;
    if(size_ == 0)
    {
        // if we don't have the size in this point
        // means that propably some shit was written
        // on the socket but we will try to parse it
        // may be we are lucky?
        if(message.ParseFromArray(chunk.constData(), chunk.size()))
        {
            // holy... cow
        }
        else
        {
            // crap
            return;
        }
    }
    else if(buffer_.size() == size_)
    {
        if(!message.ParseFromArray(buffer_.constData(), buffer_.size()))
            return;
    }

    // ok now we have valid message
    buffer_.clear();
    size_ = 0;

    if(message.sender().empty() && message.text().empty())
        return;

    emit read(message);
}

QString IOSocket::errorString() const
{
    return socket_ ? socket_->errorString() : QString();
}

QString IOSocket::peerAddress() const
{
    return socket_ ? socket_->peerAddress().toString() : QString();
}

quint16 IOSocket::peerPort() const
{
    return socket_ ? socket_->peerPort() : 0;
}
