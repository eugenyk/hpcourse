#ifndef READANDHANDLE_H
#define READANDHANDLE_H

#include <QRunnable>
#include <QTcpSocket>
#include <QThreadPool>
#include <QThread>
#include <QMutex>
#include <vector>

#include <string>
#include <iostream>
#include <unistd.h>

#include "../protobuff/chatmessage.pb.h"

class ReadAndHandle : public QObject, public QRunnable
{
    Q_OBJECT
public:
    //считывает с сокета, находит в списке указатель на сокет получателя по имени и отправляет
    ReadAndHandle(QTcpSocket *socket,
                  std::vector<std::tuple<std::string, QTcpSocket*, QMutex*> > sockets, QMutex* container_mutex);

signals:
    void deleting_socket_from_store(QTcpSocket* socket);
    void setting_username(QTcpSocket* soctet, char* name, int size);

private:
    QTcpSocket* socket;
    std::vector<std::tuple<std::string, QTcpSocket*, QMutex*> > sockets;
    QMutex* c_mutex;

    void delete_name_sock(QTcpSocket *sock);
    std::string find_name_by_sock(QTcpSocket* sock, QMutex **mutex);
    QTcpSocket* find_sock_by_name(std::string name, QMutex **mutex);

protected:
    void run();

};

#endif // READANDHANDLE_H
