#include "tcpclient.h"

TcpClient::TcpClient()
{
    socket = new QTcpSocket();
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

QTcpSocket* TcpClient::getSocket()
{
    return socket;
}
