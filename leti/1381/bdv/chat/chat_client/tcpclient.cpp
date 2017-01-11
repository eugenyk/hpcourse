#include "tcpclient.h"

TcpClient::TcpClient()
{
    socket = new QTcpSocket();
    connect(socket, SIGNAL(connected()), this, SLOT(connected_slot()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(ready_read_slot()));
}

TcpClient::~TcpClient()
{
    delete socket;
}

void TcpClient::connect_(int portnum)
{
    socket->connectToHost("localhost", portnum);
}

bool TcpClient::isConnected()
{
    return (socket->state() == QAbstractSocket::ConnectedState);
}

void TcpClient::send(std::string msg)
{
    socket->write(msg.data(), msg.size()*sizeof(char));
    socket->flush();
}

std::string TcpClient::receive()
{
    char buffer[256];
    bzero(buffer, 256);
    std::string res(buffer, socket->read(buffer, 256));
    return res;
}

void TcpClient::close_()
{
    socket->close();
}

void TcpClient::ready_read_slot()
{
    emit ready_read();
}

void TcpClient::connected_slot()
{
    emit connected();
}

