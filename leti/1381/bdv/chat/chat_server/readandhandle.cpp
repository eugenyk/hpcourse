#include "readandhandle.h"

ReadAndHandle::ReadAndHandle(QTcpSocket* socket, std::vector<std::tuple<std::string, QTcpSocket *, QMutex *> >* sockets, QMutex* container_mutex)
{
    this->socket = socket;
    this->sockets = sockets;
    this->c_mutex = container_mutex;
}

void ReadAndHandle::run()
{
    emit need_move_to_thread(socket, QThread::currentThread());
    std::cout << "Socket-sender moved to thread of thread pool" << std::endl;

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
        delete_user(socket);
        return;
    }
    //если пришло сообщение с инициализацией имени клиента
    if(msg.msgtype() == ChatMessage::Command::ChatMessage_Command_INIT)
    {
        std::string name = msg.name();
        std::cout << "Client " << name << " connected." << std::endl;
        add_username(socket, name);
    }
    //если пришло сообщение для другого клиента
    if(msg.msgtype() == ChatMessage::Command::ChatMessage_Command_SEND)
    {
        std::string name = msg.name();
        QMutex* mutex;
        QTcpSocket* rec_sock = find_sock_by_name(name);
        if(rec_sock != 0)
        {
            emit need_move_to_thread(rec_sock, QThread::currentThread());
            std::cout << "Socket-receiver moved to thread of thread pool" << std::endl;
            name = find_name_by_sock(socket);
            mutex = find_mutex_by_sock(rec_sock);
            msg.set_name(name);
            msg.SerializeToString(&s);
            mutex->lock();
            rec_sock->write(s.data(), s.size());
            mutex->unlock();
            rec_sock->moveToThread(QApplication::instance()->thread());
            std::cout << "Socket-receiver moved to main thread" << std::endl;
        }
        else
        {
            std::cout << "Cant find client" << std::endl;
            mutex = find_mutex_by_sock(socket);
            msg.set_name("SERVER");
            msg.set_message("Cant find client.");
            msg.SerializeToString(&s);
            mutex->lock();
            socket->write(s.data(), s.size());
            mutex->unlock();
        }
    }
    socket->moveToThread(QCoreApplication::instance()->thread());
    std::cout << "Socket-sender moved to main thread" << std::endl;
}

void ReadAndHandle::add_username(QTcpSocket* sock, std::string name)
{
    QMutexLocker locker(c_mutex);
    for(auto it = sockets->begin(); it != sockets->end(); it++)
    {
        if(std::get<1>(*it) == sock)
        {
            std::get<0>(*it) = name;
            return;
        }
    }
}

void ReadAndHandle::delete_user(QTcpSocket* sock)
{
    QMutexLocker locker(c_mutex);
    QMutex* mutex;
    for(auto it = sockets->begin(); it != sockets->end(); it++)
    {
        if(std::get<1>(*it) == sock)
        {
            mutex = std::get<2>(*it);
            sockets->erase(it);
            break;
        }
    }
    locker.unlock();
    mutex->lock();
    sock->close();
    delete sock;
    mutex->unlock();
    delete mutex;
}

QMutex* ReadAndHandle::find_mutex_by_sock(QTcpSocket* sock)
{
    QMutexLocker locker(c_mutex);
    for(auto it = sockets->begin(); it != sockets->end(); it++)
    {
        if(std::get<1>(*it) == sock)
            return std::get<2>(*it);
    }
    return 0;
}

std::string ReadAndHandle::find_name_by_sock(QTcpSocket* sock)
{
    QMutexLocker locker(c_mutex);
    for(auto it = sockets->begin(); it != sockets->end(); it++)
    {
        if(std::get<1>(*it) == sock)
            return std::get<0>(*it);
    }
    return "";
}

QTcpSocket* ReadAndHandle::find_sock_by_name(std::string name)
{
    QMutexLocker locker(c_mutex);
    for(auto it = sockets->begin(); it != sockets->end(); it++)
    {
        if(std::get<0>(*it) == name)
            return std::get<1>(*it);
    }
    return 0;
}
