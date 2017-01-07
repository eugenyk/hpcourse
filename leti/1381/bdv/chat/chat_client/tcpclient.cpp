#include "tcpclient.h"

TcpClient::TcpClient()
{

}

bool TcpClient::connect_(int portnum)
{
    port = portnum;
    socket_descr = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descr < 0)
    {
        std::cout << "Opening socket error" << std::endl;
        return false;
    }

    server = gethostbyname("localhost");
    if (server == NULL)
    {
        std::cout << "ERROR, no such host" << std::endl;
        return false;
    }
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);
    int res = connect(socket_descr, (struct sockaddr *) &server_addr,sizeof(server_addr));
    if (res < 0)
    {
        std::cout << "Connecting error" << std::endl;
        return false;
    }
    return true;
}

void TcpClient::send(std::string msg)
{
    int res = write(socket_descr, msg.data(), msg.size()*sizeof(char));
    if(res < 0)
    {
        std::cout << "Sending error" << std::endl;
    }
}

std::string TcpClient::receive()
{
    char buffer[256];
    bzero(buffer, 256);
    int res = read(socket_descr, buffer, 255);
    if(res < 0)
    {
        if(res == -1)
        {
            std::cout << "Receiving error" << std::endl;
            printf("%s\n", strerror(errno));
        }
        return "";
    }
    else
    {
        std::string result(buffer, res);
        return result;
    }
}

void TcpClient::close_()
{
    close(socket_descr);
}
