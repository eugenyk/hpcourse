#ifndef CHATSESSION_H
#define CHATSESSION_H
#include <QTcpSocket>
#include <QRunnable>
#include "chat_room.h"
#include "chat_participant.h"
#include "message/encode_chat_message.h"
Q_DECLARE_METATYPE(ChatMessage)

class ChatSession:public ChatParticipant
{
    Q_OBJECT
private:
    ChatRoom &chatRoom;
    QString message;
    int socketDescriptor;

private:
    ///readLogin
    void readLogin();
public: QTcpSocket *tcpSocket;
    ChatSession(int socetDescriptor,ChatRoom & chatRoom);
    ~ChatSession();
     void startSession();
signals:
    void joinedClient(quintptr);
    void readedMessage(ChatMessage);
    void closedSession(quintptr);
public slots:
    virtual void readMessage();
    void closeSession();
    virtual void sendMessage(EncodedChatMessage encodedChatMessage);

};

#endif // CHATSESSION_H
