#include <QDataStream>
#include <QThread>
#include "unistd.h"

#include "message/message.pb.h"
#include "chat_session.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
using namespace std;
using namespace google::protobuf::io;

ChatSession::ChatSession(int _socketDescriptor,ChatRoom & _chatRoom):
    chatRoom(_chatRoom), socketDescriptor(_socketDescriptor)
{

    qRegisterMetaType <ChatMessage> ("ChatMessage");
    qRegisterMetaType <EncodedChatMessage> ("EncodedChatMessage");
    tcpSocket=new QTcpSocket;
    tcpSocket->setSocketDescriptor(socketDescriptor);
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(closeSession()));
    connect(this,SIGNAL(signalJoin()),this,SLOT(join()));

}
///Read Login
void ChatSession::readLogin(){
    bool ok;
    quint8 lengthData;
    QByteArray baLengthData,baData;
    baLengthData=tcpSocket->read(1);
    baLengthData=baLengthData.toHex();
    lengthData=baLengthData.toInt(&ok,16);

    baData=tcpSocket->read(lengthData);

    QString data=baData;
    message=data;
    qDebug()<<"Len: "<<lengthData<<" "<<data;

}

void ChatSession::readMessage(){
    ChatMessage chatMessage;
    bool ok;
    quint8 countBytes;

    QByteArray baCountBytes,baEncodeLength;
    baCountBytes= tcpSocket->read(1);
    baCountBytes= baCountBytes.toHex();
    countBytes= baCountBytes.toInt(&ok,16);

    baEncodeLength= tcpSocket->read(countBytes);

    quint32 lengthChatMessage;
    quint8 bufferDecod[4];

    for(int i=0;i<baEncodeLength.size();i++){
        bufferDecod[i]=baEncodeLength[i];
    }

    ArrayInputStream raw_input(bufferDecod,sizeof(bufferDecod));
    CodedInputStream coded_input(&raw_input);

    coded_input.ReadVarint32(&lengthChatMessage);

    QByteArray baChatMessage;
    baChatMessage= tcpSocket->read(lengthChatMessage);
    chatMessage.ParseFromString(baChatMessage.data());

    qDebug()<<"Sender:"<<chatMessage.sender().data();
    qDebug()<<"Text:"<<chatMessage.text().data();
    qDebug()<<"Size:"<<baEncodeLength.size();

    chatRoom.addMessage(chatMessage);
    emit readedMessage(chatMessage);
}

void ChatSession::startSession(){
    qDebug()<<"StartSession";
    tcpSocket->waitForReadyRead(5000);
    ///Read Login
    readLogin();
    connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(readMessage())/*/Qt::DirectConnection/*/);
    connect(this,SIGNAL(signalSend(EncodedChatMessage)),this,SLOT(sendMessage(EncodedChatMessage)));
    ///Join Client
    chatRoom.joinParticipant(this);
    emit joinedClient(socketDescriptor);

}

void ChatSession::sendMessage(EncodedChatMessage encodedchatMessage){
    tcpSocket->write(encodedchatMessage.baCountByte);
    tcpSocket->write(encodedchatMessage.baEncodeLength);
    tcpSocket->write(encodedchatMessage.baChatMessage);
}

void ChatSession::closeSession(){
    qDebug()<<"Close Session";
    tcpSocket->close();
    emit closedSession(socketDescriptor);
}

ChatSession::~ChatSession(){
            qDebug()<<"Destroy Chat Sesion:"<<socketDescriptor;
                disconnect(tcpSocket,SIGNAL(readyRead()),this,SLOT(readMessage()));
                disconnect(this,SIGNAL(signalSend(EncodedChatMessage)),this,SLOT(sendMessage(EncodedChatMessage)));
                disconnect(tcpSocket,SIGNAL(disconnected()),this,SLOT(closeSession()));
                tcpSocket->deleteLater();
}
