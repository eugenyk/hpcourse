#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QtNetwork>


#include <dialogchat.h>

#include "socket_handler.h"
namespace server {
namespace proto {
class Message;
}
}
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = 0);

signals:
    void displayError(QAbstractSocket::SocketError socketError);
    void sendDialogNewFortune_Text();
    void connectionEstablished();

public slots:

    void setUp_Connection_With_Server(QString hostName,quint16 hostPort);
    void sendMessage(const server::proto::Message& message);

private:
    QTcpSocket *tcpSocket;
    Socket_Handler *m_socketHandler;

    QString currentFortune;
    quint16 blockSize;

    QNetworkSession *networkSession;


    QString m_hostName;
    quint16 m_hostPort;

    DialogChat *dialogChat;

private slots:

    void setFortune_Text_T(const server::proto::Message& message);

    void emitDisplayError(QAbstractSocket::SocketError socketError);
};

#endif // CONTROLLER_H
