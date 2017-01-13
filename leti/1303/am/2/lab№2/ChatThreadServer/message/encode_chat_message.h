#ifndef ENCODECHATMESSAGE_H
#define ENCODECHATMESSAGE_H

#include "message.pb.h"
#include <QByteArray>
struct EncodedChatMessage{
    QByteArray baCountByte;
    QByteArray baEncodeLength;
    QByteArray baChatMessage;
    EncodedChatMessage(){}
    EncodedChatMessage(
    QByteArray _baCountByte,
    QByteArray _baEncodeLength,
    QByteArray _baChatMessage
    ){
        baCountByte=_baCountByte;
        baEncodeLength=_baEncodeLength;
        baChatMessage=_baChatMessage;
    }
};

class EncodeChatMessage
{
public:

   EncodeChatMessage();
  static  EncodedChatMessage encodeChatMessage(ChatMessage &_chatMessage);

};

#endif // ENCODECHATMESSAGE_H
