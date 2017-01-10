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

private slots:
    void ready_read_slot();
public:
    TcpClient();
    ~TcpClient();
    void connect_(int portnum);
    bool isConnected();
    void send(std::string msg);
    std::string receive();
    void close_();

signals:
    void ready_read();

};

#endif // TCPCLIENT_H
