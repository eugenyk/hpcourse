#ifndef CHATSERVER_H
#define CHATSERVER_H
#include <QTcpSocket>
#include <QByteArray>
#include <QTcpServer>
#include <QFile>
#include <QRunnable>
#include "chat_room.h"
#include "chat_session.h"

//Q_DECLARE_METATYPE(quintptr)
#include "QThreadPool"
class ChatServer : public QTcpServer
{
    Q_OBJECT
private:
    ChatRoom chatRoom;
    quint8 countThread;
    QMap <quintptr,ChatSession*> mapSession;
    QThreadPool threadPool;
private:
    void incomingConnection(qintptr handle);
    void readData();

public:
    explicit ChatServer(quint8 _countThread,QObject *parent = 0);
    ~ChatServer();
signals:
public slots:
    void sendAllMessagesToClient(quintptr);
    void sendMessageToAllClients(ChatMessage);
    void deleteSession(quintptr);
};

#endif // CHATSERVER_H
