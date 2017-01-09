#include "readandhandle.h"

ReadAndHandle::ReadAndHandle(QTcpSocket* socket, std::vector<std::pair<std::string, QTcpSocket *> > sockets)
{
    this->socket = socket;
    this->sockets = sockets;
}

void ReadAndHandle::run()
{
    char buffer[256];
    int n = socket->read(buffer, 255);
    std::string s(buffer, n);

    //если пришло сообщение отключения
    if(s[0] == '-')
    {
        std::cout << "Client disconnected" << std::endl;
        emit deleting_socket_from_store(socket);
        return;
    }
    //если пришло сообщение с инициализацией имени клиента
    if(s[0] == '+')
    {
        std::string name = s.substr(1);
        std::pair<std::string, QTcpSocket*> elem;
        elem.first = name;
        elem.second = socket;
        std::cout << "Client " << name << " connected." << std::endl;
        char* data = new char[name.size()];
        memcpy(data, name.data(), name.size()*sizeof(char));
        emit setting_username(socket, data, name.size());
        return;
    }
    //если пришло сообщение для другого клиента
    std::string name, msg;
    std::cout << "Received: " << s << std::endl;
    getnameandmsg(s, name, msg);
    QTcpSocket* rec_sock = find_sock_by_name(name);
    if(rec_sock != 0)
    {
        name = find_name_by_sock(socket);
        setnameandmsg(s, name, msg);

        std::cout << "Sent " << s << std::endl;
        int wdesc = rec_sock->socketDescriptor();

        //l
        write(wdesc, s.data(), s.size());
        //ul
    }
    else
    {
        std::cout << "Cant find client" << std::endl;
    }
}

std::string ReadAndHandle::find_name_by_sock(QTcpSocket* sock)
{
    for(auto it = sockets.begin(); it != sockets.end(); it++)
    {
        std::pair<std::string, QTcpSocket*> elem = *it;
        if(elem.second == sock)
            return elem.first;
    }
    return "";
}

QTcpSocket* ReadAndHandle::find_sock_by_name(std::string name)
{
    for(auto it = sockets.begin(); it != sockets.end(); it++)
    {
        std::pair<std::string, QTcpSocket*> elem = *it;
        if(elem.first == name)
            return elem.second;
    }
    return 0;
}

bool ReadAndHandle::getnameandmsg(std::string rec_data, std::string& name, std::string& msg)
{
    int name_size = (int)rec_data[0];
    rec_data.erase(rec_data.begin());
    name = rec_data.substr(0, name_size);
    msg = rec_data.erase(0, name_size);
    return true;
}

void ReadAndHandle::setnameandmsg(std::string& snd_data, std::string name, std::string msg)
{
    snd_data = "0" + name + msg;
    char name_size = name.size();
    snd_data[0] = name_size;
}
