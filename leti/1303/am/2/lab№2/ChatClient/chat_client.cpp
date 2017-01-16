#include "chat_client.h"
#include <QDataStream>
#include "message/message.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
using namespace std;
using namespace google::protobuf::io;

unsigned long long all_count_pack;

ChatClient::ChatClient(){
    tcpSocket=new QTcpSocket(this);
}

void ChatClient::connectToServer(QString _login){
    login=_login;
    tcpSocket->connectToHost(QHostAddress::LocalHost,2323);
    connect(tcpSocket,SIGNAL(connected()),this,SLOT(sendLogin()));
    connect(tcpSocket,SIGNAL(connected()),this,SIGNAL(connectedToServer()));

    connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(readMessage()),Qt::DirectConnection);
}

void ChatClient::sendLogin(){

    quint8 lenghtLogin=login.length();
    QByteArray baLengthMessage,baMessage;
    QDataStream outBALengthMessage(&baLengthMessage,QIODevice::WriteOnly);

    outBALengthMessage<<lenghtLogin;
    baMessage.append(login);

    tcpSocket->write(baLengthMessage);
    tcpSocket->write(baMessage);
}

void ChatClient::sendMessage(ChatMessage chatMessage){
qDebug()<<"Is open "<<tcpSocket->isOpen();
qDebug()<<"Is write "<<tcpSocket->isWritable();
qDebug()<<"Is valid "<<tcpSocket->isValid();
    QString chatMessageAsString=chatMessage.SerializeAsString().data();

    quint8 countByte;
    quint32 lenghtChatMessage=chatMessageAsString.length();
    quint8 bufferEncodeLength[4];
    QByteArray baCountByte,baEncodeLength;
    QByteArray baChatMessage;
    QDataStream outBaEncodeLength(&baEncodeLength,QIODevice::WriteOnly);

    ZeroCopyOutputStream* raw_output = new ArrayOutputStream(bufferEncodeLength,sizeof(bufferEncodeLength));
    CodedOutputStream* coded_output = new CodedOutputStream(raw_output);
    coded_output->WriteVarint32(lenghtChatMessage);

    countByte=coded_output->ByteCount();

    baCountByte.append(countByte);
    for(quint8 i=0;i<countByte;i++){
        outBaEncodeLength<<bufferEncodeLength[i];
    }
    baChatMessage.append(chatMessageAsString);

    tcpSocket->write(baCountByte);
    tcpSocket->write(baEncodeLength);
    tcpSocket->write(baChatMessage);

}

void ChatClient::readMessage(){
    qDebug()<<"ReadMes";
    bool ok;
    ChatMessage chatMessage;
    quint8 countBytes;
    QByteArray baCountBytes,baEncodeLength,baChatMessage;;
    quint8 bufferDecod[4];
    quint32 lengthChatMessage;

   // ZeroCopyInputStream* raw_input=new ArrayInputStream(bufferDecod,sizeof(bufferDecod));
    //CodedInputStream *coded_input=new CodedInputStream(raw_input);

    qDebug()<<sizeof(bufferDecod);
    while(tcpSocket->bytesAvailable()){

        baCountBytes= tcpSocket->read(1);
        baCountBytes= baCountBytes.toHex();
        countBytes= baCountBytes.toInt(&ok,16);

        baEncodeLength= tcpSocket->read(countBytes);

        for(int i=0;i<baEncodeLength.size();i++){
            bufferDecod[i]=baEncodeLength[i];
        }

       // raw_input = new ArrayInputStream(bufferDecod,sizeof(bufferDecod));
        //coded_input = new CodedInputStream(raw_input);
        ArrayInputStream raw_input(bufferDecod,sizeof(bufferDecod));
        CodedInputStream coded_input(&raw_input);


        coded_input.ReadVarint32(&lengthChatMessage);
        baChatMessage= tcpSocket->read(lengthChatMessage);
        chatMessage.ParseFromString(baChatMessage.data());

        qDebug()<<"Sender:"<<chatMessage.sender().data();
        qDebug()<<"Text:"<<chatMessage.text().data();
        qDebug()<<"Size:"<<lengthChatMessage<<" "<<baEncodeLength.size();
        qDebug()<<bufferDecod[0]<<" "<<bufferDecod[1]<<" "<<bufferDecod[2]<<" "<<bufferDecod[3];
        for(int i=0;i<4;i++){
            bufferDecod[i]=0;
        }
       // QThread::sleep(2);
        emit readedMessage(chatMessage);
    }

}
