#include <QtNetwork/QHostAddress>
#include <QtEndian>

#include "Message.pb.h"
#include "socket_handler.h"

template<typename T>
static inline T castWithEndian(T src)
{
    if(Socket_Handler::endian() == QSysInfo::BigEndian)
        return qToBigEndian(src);
    else
        return qToLittleEndian(src);
}

static QSysInfo::Endian endian = QSysInfo::ByteOrder;

QSysInfo::Endian Socket_Handler::endian()
{
    return ::endian;
}

void Socket_Handler::setEndian(QSysInfo::Endian endian)
{
    ::endian = endian;
}

Socket_Handler::Socket_Handler(QTcpSocket* socket, QObject* parent) : QObject(parent), socket_(socket), lastPacketTimestamp_(QDateTime::currentDateTime()), size_(0)
{
    connect(socket_.data(), SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError)));
    connect(socket_.data(), &QTcpSocket::disconnected, this, &Socket_Handler::disconnected);
    connect(socket_.data(), &QTcpSocket::connected, this, &Socket_Handler::connected);
    connect(socket_.data(), &QTcpSocket::readyRead, this, &Socket_Handler::readyRead);
}

Socket_Handler::~Socket_Handler()
{
}

QAbstractSocket::SocketState Socket_Handler::state() const
{
    return socket_ ? socket_->state() : QAbstractSocket::UnconnectedState;
}

QAbstractSocket::SocketError Socket_Handler::error() const
{
    return socket_ ? socket_->error() : QAbstractSocket::UnknownSocketError;
}

bool Socket_Handler::write(const server::proto::Message& message)
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

void Socket_Handler::readyRead()
{
    QDateTime timestamp = QDateTime::currentDateTime();
    if(lastPacketTimestamp_.msecsTo(timestamp) > 1000)
    {
        buffer_.clear();
        size_ = 0;
    }

    lastPacketTimestamp_.swap(timestamp);

    QByteArray chunk = socket_->readAll();

    // try to read first bytes
    if(size_ == 0)
    {
        size_ = castWithEndian(*((qint32*)chunk.left(sizeof(qint32)).constData()));
        if(size_ < 0)
            size_ = 0;

        // 4 bytes remove
        if(size_)
            chunk.remove(0, sizeof(qint32));
    }

    // append current chunk to the buffer
    qint32 needBytes = size_ - buffer_.size();
    if(needBytes > 0)
        buffer_.append(chunk.left(needBytes));

    server::proto::Message message;
    if(size_ == 0)
    {

        if(message.ParseFromArray(chunk.constData(), chunk.size()))
        {
           //good
        }
        else
        {
            //bad
            return;
        }
    }
    else if(buffer_.size() == size_)
    {
        if(!message.ParseFromArray(buffer_.constData(), buffer_.size()))
            return;
    }

    buffer_.clear();
    size_ = 0;

    if(message.sender().empty() && message.text().empty())
        return;

    emit read(message);


    google::protobuf::ShutdownProtobufLibrary();
}

QString Socket_Handler::errorString() const
{
    return socket_ ? socket_->errorString() : QString();
}

QString Socket_Handler::peerAddress() const
{
    return socket_ ? socket_->peerAddress().toString() : QString();
}

quint16 Socket_Handler::peerPort() const
{
    return socket_ ? socket_->peerPort() : 0;
}
