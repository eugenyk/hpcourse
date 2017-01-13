#include "chat_participant.h"

ChatParticipant::ChatParticipant(QObject *parent):isReadAllMessage(false)
{

}

void ChatParticipant::setIsReadAllMessage(bool _isReadAllMessage){
    isReadAllMessage=_isReadAllMessage;
}
bool ChatParticipant::getIsReadAllMessage(){
    return isReadAllMessage;
}
