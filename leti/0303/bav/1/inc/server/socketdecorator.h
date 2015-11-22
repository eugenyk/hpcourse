#ifndef _SOCKET__H_
#define _SOCKET__H_

#include <QtCore/QObject>

class IOSocket;

namespace server {
namespace proto {
class Message;
}
}

class IOSocketDecorator : public QObject
{
    Q_OBJECT

public:
    void writeAsync(const server::proto::Message& message);
    void initAsync();

signals:
    void disconnected();

private slots:
    void receive(const server::proto::Message& message);
    void send(const server::proto::Message& message);
    void error();
    void init();

private:
    IOSocketDecorator(qintptr socketDescriptor, QObject* parent = 0);
    Q_DISABLE_COPY(IOSocketDecorator)
    ~IOSocketDecorator();
    friend class Server;

    qintptr descriptor_;
    IOSocket* socket_;
};

#endif // _SOCKET__H_
