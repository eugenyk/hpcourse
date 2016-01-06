#ifndef _SOCKET_HANDLER_H_
#define _SOCKET_HANDLER_H_

#include <QtNetwork/QTcpSocket>
#include <QtCore/QDateTime>
#include <QtCore/QPointer>


#include "Message.pb.h"

class Socket_Handler : public QObject
{
    Q_OBJECT

public:
    static void setEndian(QSysInfo::Endian endian);
    static QSysInfo::Endian endian();

    Socket_Handler(QTcpSocket* socket, QObject* parent = 0);
    ~Socket_Handler();

    QAbstractSocket::SocketState state() const;
    QAbstractSocket::SocketError error() const;
    QString errorString() const;
    QString peerAddress() const;
    quint16 peerPort() const;

    bool write(const server::proto::Message& message);

signals:
    void read(const server::proto::Message& message);
    void error(QAbstractSocket::SocketError error);
    void disconnected();
    void connected();

private slots:
    void readyRead();

private:
    QDateTime lastPacketTimestamp_;
    QPointer<QTcpSocket> socket_;
    QByteArray buffer_;
    qint32 size_;
};

#endif // _SOCKET_HANDLER_H_
