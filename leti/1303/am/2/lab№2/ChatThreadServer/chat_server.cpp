#include "chat_server.h"
#include "chat_session.h"
#include <QtConcurrent/QtConcurrent>
#include <QThreadPool>
#include "runnable/send_all_messages.h"
#include "runnable/send_message_to_clients.h"
ChatServer::ChatServer(quint8 _countThread, QObject *parent) :
    countThread(_countThread),
    QTcpServer(parent)
{
    threadPool.setMaxThreadCount(countThread);
    qRegisterMetaType <quintptr> ("quintptr");
    if (listen(QHostAddress::Any,2323))
        qDebug()<<"Server Ready";
    else qDebug()<<"Error";
}


void ChatServer::incomingConnection(qintptr handle){
    qDebug()<<"IncommingConnection"<<handle;

    ChatSession *session= new ChatSession(handle,chatRoom);
    mapSession.insert(handle,session);
    connect(session,SIGNAL(joinedClient(quintptr)),this,SLOT(sendAllMessagesToClient(quintptr)));
    connect(session,SIGNAL(readedMessage(ChatMessage)),this,SLOT(sendMessageToAllClients(ChatMessage)));
    connect(session,SIGNAL(closedSession(quintptr)),this,SLOT(deleteSession(quintptr)));
    session->startSession();
}


void ChatServer::sendAllMessagesToClient(quintptr socketDescriptor){
    ChatSession *session=mapSession[socketDescriptor];
    RunnableSendAllMessageToClient *sendMessagesToClient=new RunnableSendAllMessageToClient(session,chatRoom.getVectorChatMessage());
    threadPool.start(sendMessagesToClient);
}


void ChatServer::sendMessageToAllClients(ChatMessage message){
    RunnableSendMessageToAllClients *sendMesageToClients=new RunnableSendMessageToAllClients(message,chatRoom.getVectorParticipant());
    threadPool.start(sendMesageToClients);
}

void ChatServer::deleteSession(quintptr socketDescriptor){
    ChatSession *session=mapSession[socketDescriptor];
    chatRoom.removeParticipant(session);
    mapSession.remove(socketDescriptor);
    delete session;
}

ChatServer::~ChatServer(){
    threadPool.clear();
    foreach (ChatSession *session, mapSession) {
       delete session;
    }
    mapSession.clear();

}
