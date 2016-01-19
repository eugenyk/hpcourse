#ifndef _SOCKET__H_
#define _SOCKET__H_

#include <QtCore/QObject>

#include <QRunnable>
#include <QTcpSocket>


#include <QEventLoop>

#include "socket_handler.h"
#include "Message.pb.h"

class SocketDecorator : public QObject, public QRunnable
{
    Q_OBJECT

public:    
    QEventLoop* wait_loop;

    void writeAsync(const server::proto::Message& message);

signals:
    void disconnected();

private slots:
    void receive(const server::proto::Message& message);
    void send(const server::proto::Message& message);
    void error();



private:
    SocketDecorator(qintptr socketDescriptor, QObject* parent = 0);
    Q_DISABLE_COPY(SocketDecorator)
    ~SocketDecorator();
    friend class Server;

    qintptr descriptor_;
    Socket_Handler* socket_handler;
    QTcpSocket* socket;

protected:
    void run() Q_DECL_OVERRIDE;
};

#endif // _SOCKET__H_
