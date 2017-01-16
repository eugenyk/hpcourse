#ifndef CHATCLIENT_H
#define CHATCLIENT_H
#include <QTcpSocket>
#include <QNetworkInterface>
#include <iostream>
#include "message/message.pb.h"

extern unsigned long long all_count_pack;
extern unsigned long long receive_pack;
#define server_addr "192.168.1.223"
using namespace std;

class ChatClient:public QObject
{
    Q_OBJECT
private:
    QTcpSocket *tcpSocket;

public:
    QString login;
    ChatClient();
signals:
    void connectedToServer();
    void readedMessage(QString);
    void readedMessage(ChatMessage);
public slots:
    void connectToServer(QString);
    void sendLogin();
    void sendMessage(ChatMessage);
    void readMessage();
};

#endif // CHATCLIENT_H
