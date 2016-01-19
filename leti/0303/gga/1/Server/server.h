#ifndef _SERVER_H_
#define _SERVER_H_

#include <QtNetwork/QTcpServer>

#include <QtCore/QReadWriteLock>
#include <QtCore/QPointer>
#include <QtCore/QHash>

#include <QThreadPool>

#include "socketdecorator.h"
#include "waitingthread.h"

#include "Message.pb.h"


class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = 0);
    ~Server();

    static Server* instance();

    void broadcast(const QPointer<SocketDecorator>& from, const server::proto::Message& msg);
    void setThreadPoolSize(quint32 size);
    quint32 threadPoolSize() const;

public slots:
    bool start(quint16 port = 0);
    void stop();

protected:
    void incomingConnection(qintptr socketDescriptor);

private slots:
    void printError(QAbstractSocket::SocketError error);
    void socketDisconnected();

private:
    QHash<qintptr,QPointer<SocketDecorator>> clients_;
    QReadWriteLock rwmutex_;
//    ThreadPool* threadPool_;

    QThreadPool *m_threadPool;

    int m_threadPool_Size;
};

#endif // _SERVER_H_
