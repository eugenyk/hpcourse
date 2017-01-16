#ifndef CHATROOM_H
#define CHATROOM_H

#include <QVector>
#include <QRunnable>
#include "chat_participant.h"
#include "message/message.pb.h"
class ChatRoom:public QObject
{
    QVector<ChatParticipant*> vectorParticipant;
    QVector<ChatMessage> vectorChatMessage;
public:
    void run();

    ChatRoom();
    void joinParticipant(ChatParticipant *participant);
    void addMessage(ChatMessage message);
    int getCounMessages();
    int getCountParticipant();
    void sendMessageAllParticipant(ChatMessage message);
    QVector<ChatParticipant*>* getVectorParticipant();
    QVector<ChatMessage>* getVectorChatMessage();
    void removeParticipant(ChatParticipant *);
};


#endif // CHATROOM_H
