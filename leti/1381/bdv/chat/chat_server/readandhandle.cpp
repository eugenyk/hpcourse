#include "readandhandle.h"

ReadAndHandle::ReadAndHandle(QTcpSocket* socket, std::vector<std::tuple<std::string, QTcpSocket*, QMutex*> > sockets)
{
    this->socket = socket;
    this->sockets = sockets;
}

void ReadAndHandle::run()
{
    char buffer[256];
    int n = socket->read(buffer, 255);
    if(n < 1) return;
    std::string s(buffer, n);
    std::cout << s << std::endl;
    ChatMessage msg;
    msg.ParseFromString(s);

    //если пришло сообщение отключения
    if(msg.msgtype() == ChatMessage::Command::ChatMessage_Command_DISC)
    {
        std::cout << "Client disconnected" << std::endl;
        emit deleting_socket_from_store(socket);
        return;
    }
    //если пришло сообщение с инициализацией имени клиента
    if(msg.msgtype() == ChatMessage::Command::ChatMessage_Command_INIT)
    {
        //std::string name = s.substr(1);
        std::string name = msg.name();
        std::cout << "Client " << name << " connected." << std::endl;
        char* data = new char[name.size()];
        memcpy(data, name.data(), name.size()*sizeof(char));
        emit setting_username(socket, data, name.size());
        return;
    }
    //если пришло сообщение для другого клиента
    if(msg.msgtype() == ChatMessage::Command::ChatMessage_Command_SEND)
    {
        std::string name = msg.name();
        QMutex* rec_mutex;
        QMutex* snd_mutex;
        QTcpSocket* rec_sock = find_sock_by_name(name, &rec_mutex);
        if(rec_sock != 0)
        {
            name = find_name_by_sock(socket, &snd_mutex);
            msg.set_name(name);
            msg.SerializeToString(&s);
            int wdesc = rec_sock->socketDescriptor();
            rec_mutex->lock();
            write(wdesc, s.data(), s.size());
            rec_mutex->unlock();
        }
        else
        {
            std::cout << "Cant find client" << std::endl;
            find_name_by_sock(socket, &snd_mutex);
            msg.set_name("SERVER");
            msg.set_message("Cant find client.");
            msg.SerializeToString(&s);
            int wdesc = socket->socketDescriptor();
            snd_mutex->lock();
            write(wdesc, s.data(), s.size());
            snd_mutex->unlock();
        }
    }
}

std::string ReadAndHandle::find_name_by_sock(QTcpSocket* sock, QMutex **mutex)
{
    for(auto it = sockets.begin(); it != sockets.end(); it++)
        if(std::get<1>(*it) == sock)
        {
            mutex[0] = std::get<2>(*it);
            return std::get<0>(*it);
        }
    return "";
}

QTcpSocket* ReadAndHandle::find_sock_by_name(std::string name, QMutex** mutex)
{
    for(auto it = sockets.begin(); it != sockets.end(); it++)
        if(std::get<0>(*it) == name)
        {
            mutex[0] = std::get<2>(*it);
            return std::get<1>(*it);
        }
    return 0;
}
