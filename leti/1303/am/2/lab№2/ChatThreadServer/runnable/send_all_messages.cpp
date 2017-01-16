#include "send_all_messages.h"
#include <QDebug>
#include <QThread>
#include "message/encode_chat_message.h"

RunnableSendAllMessageToClient::RunnableSendAllMessageToClient(ChatParticipant *_part,QVector<ChatMessage> *_vectorChatMessage,QObject *parent):
    QObject(parent),
    part(_part),
    vectorChatMessage(_vectorChatMessage)
{

}

void RunnableSendAllMessageToClient::run(){
    EncodedChatMessage encodedChatMessage;
  //  qDebug()<<"Begin";
    //foreach(ChatMessage message,*vectorChatMessage)
    for (int i=0;i<vectorChatMessage->size();i++)
    {
        ChatMessage &message=(*vectorChatMessage)[i];
        encodedChatMessage=  EncodeChatMessage::encodeChatMessage(message);

        //  qDebug()<<"Size:"<<vectorChatMessage->size()<<"Message:"<<message.text().data();
         // QThread::sleep(1);

        emit part->signalSend(encodedChatMessage);
    }
//    qDebug()<<"end";
    part->setIsReadAllMessage(true);

}
