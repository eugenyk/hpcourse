#ifndef _SERVER_H_
#define _SERVER_H_

#include <QtNetwork/QTcpServer>

#include <QtCore/QReadWriteLock>
#include <QtCore/QPointer>
#include <QtCore/QHash>

#include "Message.pb.h"

class IOSocketDecorator;
class ExecutionThread;
class ThreadPool;

Q_DECLARE_METATYPE(server::proto::Message)

class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = 0);
    ~Server();

    static Server* instance();

    void broadcast(const QPointer<IOSocketDecorator>& from, const server::proto::Message& msg);
    void setThreadPoolSize(quint32 size);
    quint32 threadPoolSize() const;

public slots:
    bool start(quint16 port = 0);
    void stop();

protected:
    void incomingConnection(qintptr handle);

private slots:
    void printError(QAbstractSocket::SocketError error);
    void socketDisconnected();

private:
    QHash<qintptr,QPointer<IOSocketDecorator>> clients_;
    QReadWriteLock rwmutex_;
    ThreadPool* threadPool_;
};

#endif // _SERVER_H_
