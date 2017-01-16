#ifndef ROOMRUNNABLE_H
#define ROOMRUNNABLE_H

#include <QRunnable>
#include <QObject>
#include <QVector>
#include "message/message.pb.h"
#include "chat_participant.h"
class RunnableSendAllMessageToClient :
        public QObject,
        public QRunnable
{
public:
    QVector<ChatMessage> *vectorChatMessage;
    ChatParticipant *part;
    void run();
    RunnableSendAllMessageToClient(ChatParticipant *_part,QVector<ChatMessage> *vectorChatMessage,QObject *parent=0);
};

#endif // ROOMRUNNABLE_H
