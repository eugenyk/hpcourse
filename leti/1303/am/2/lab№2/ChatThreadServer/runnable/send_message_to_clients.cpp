#include "send_message_to_clients.h"
#include <QDebug>
#include <QThread>
#include "message/encode_chat_message.h"
RunnableSendMessageToAllClients::RunnableSendMessageToAllClients(ChatMessage _chatMesage,QVector<ChatParticipant*> *_vectorParticipant,QObject *parent):
QObject(parent),chatMessage(_chatMesage),vectorParticipant(_vectorParticipant)
{

}

void RunnableSendMessageToAllClients::run(){
    EncodedChatMessage encodedChatMessage= EncodeChatMessage::encodeChatMessage(chatMessage);;
    ChatParticipant *participant;
   // foreach (ChatParticipant *participant,*vectorParticipant){
    for(int i=0;i<vectorParticipant->size();i++){
       //qDebug()<<"SD_SenAll:"<<i;
      //QThread::sleep(1);
       participant=(*vectorParticipant)[i];
       //encodedChatMessage= EncodeChatMessage::encodeChatMessage(chatMessage);

       if(participant->getIsReadAllMessage())
       emit participant->signalSend(encodedChatMessage);

    }
}
