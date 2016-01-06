#include "controller.h"

#include "Message.pb.h"
#include "dialogchat.h"
Controller::Controller(QObject *parent) : QObject(parent)
{

    tcpSocket = new QTcpSocket(this);
    m_socketHandler=new Socket_Handler(tcpSocket);


    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(emitDisplayError(QAbstractSocket::SocketError)));

    connect(m_socketHandler,SIGNAL(read(server::proto::Message)),this,SLOT(setFortune_Text_T(server::proto::Message)));
    connect(m_socketHandler,SIGNAL(disconnected()),qApp,SLOT(quit()));
}

void Controller::setUp_Connection_With_Server(QString hostName, quint16 hostPort)
{
    m_hostName=hostName;
    m_hostPort=hostPort;

    tcpSocket->connectToHost(m_hostName,m_hostPort);
    emit connectionEstablished();

    dialogChat=new DialogChat();
    dialogChat->show();

    connect(dialogChat,SIGNAL(sendMessage(server::proto::Message)),this,SLOT(sendMessage(server::proto::Message)));
}

void Controller::sendMessage(const server::proto::Message& message)
{

    if(m_socketHandler->state() == QAbstractSocket::ConnectedState)
    {

            if(m_socketHandler->write(message))
            {
                qDebug()<<"Message send";
            }
//            else
                //ERROR
    }


}

void Controller::setFortune_Text_T(const server::proto::Message &message)
{    
    dialogChat->printMessage(message);
}

void Controller::emitDisplayError(QAbstractSocket::SocketError socketError)
{
    emit displayError(socketError);
}




