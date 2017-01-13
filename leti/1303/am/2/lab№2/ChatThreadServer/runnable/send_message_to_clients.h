#ifndef SENDALLRUNNABLE_H
#define SENDALLRUNNABLE_H

#include <QObject>
#include <QRunnable>
#include <QVector>
#include "chat_participant.h"
#include "message/message.pb.h"
class RunnableSendMessageToAllClients :
        public QObject,
        public QRunnable
{
public:
    QVector<ChatParticipant*> *vectorParticipant;
    ChatMessage chatMessage;
    void run();
    RunnableSendMessageToAllClients(ChatMessage _chatMessage, QVector<ChatParticipant*> *_vectorParticipant,QObject *parent=0);
};

#endif // SENDALLRUNNABLE_H
