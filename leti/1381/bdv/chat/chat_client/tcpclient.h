#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <string>
#include <QTcpSocket>
#include <QObject>

class TcpClient : public QObject
{
    Q_OBJECT

private:
    QTcpSocket* socket;

public:
    TcpClient();
    ~TcpClient();
    void connect_(int portnum);
    bool isConnected();
    void send(std::string msg);
    std::string receive();
    void close_();
    QTcpSocket* getSocket();
};

#endif // TCPCLIENT_H
