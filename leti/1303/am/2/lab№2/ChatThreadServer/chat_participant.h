#ifndef CHATPARTICIPANT_H
#define CHATPARTICIPANT_H

#include  <QObject>
#include "message/message.pb.h"
#include "message/encode_chat_message.h"
class ChatParticipant :public QObject
{Q_OBJECT
    bool isReadAllMessage;
public:
    ChatParticipant(QObject *parent=0);
    virtual void sendMessage(EncodedChatMessage message)=0;
    virtual void readMessage()=0;
    void setIsReadAllMessage(bool _isReadAllMessage);
    bool getIsReadAllMessage();
signals:
    void signalJoin();
    void signalSend(EncodedChatMessage);
};

#endif // CHATPARTICIPANT_H
