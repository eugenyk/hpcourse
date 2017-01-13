#include "chat_room.h"
#include <QDebug>
#include <QThread>
ChatRoom::ChatRoom()
{

    ChatMessage msg1,msg2,msg3;
    msg1.set_sender("Sender_1");
    msg1.set_text("Message_1");

    msg2.set_sender("Sender_2");
    msg2.set_text("Message_2");

    msg3.set_sender("Sender_3");
    msg3.set_text("Message_3");

    vectorChatMessage.append(msg1);
    vectorChatMessage.append(msg2);
    vectorChatMessage.append(msg3);
}

void ChatRoom::joinParticipant(ChatParticipant *participant){
    vectorParticipant.append(participant);
}
void ChatRoom::addMessage(ChatMessage message){
    vectorChatMessage.append(message);
}

int ChatRoom::getCounMessages(){
   return vectorChatMessage.size();
}

int ChatRoom::getCountParticipant(){
   return vectorParticipant.size();
}

QVector<ChatMessage>* ChatRoom::getVectorChatMessage(){
    return &vectorChatMessage;
}

QVector<ChatParticipant*>* ChatRoom::getVectorParticipant(){
    return &vectorParticipant;
}

void ChatRoom::removeParticipant(ChatParticipant *participant){
    vectorParticipant.removeOne(participant);
}
