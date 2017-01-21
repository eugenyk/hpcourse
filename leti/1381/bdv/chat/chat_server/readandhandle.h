#ifndef READANDHANDLE_H
#define READANDHANDLE_H

#include <QApplication>
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
                  std::vector<std::tuple<std::string, QTcpSocket*, QMutex*> >* sockets, QMutex* container_mutex);

signals:
    void need_move_to_thread(QTcpSocket* socket, QThread* thread);

private:
    QTcpSocket* socket;
    std::vector<std::tuple<std::string, QTcpSocket*, QMutex*> >* sockets;
    QMutex* c_mutex;

    //добавление и удаление из контейнера
    void add_username(QTcpSocket* sock, std::string name);
    void delete_user(QTcpSocket* sock);

    //поиск в контейнере
    std::string find_name_by_sock(QTcpSocket* sock);
    QTcpSocket* find_sock_by_name(std::string name);
    QMutex* find_mutex_by_sock(QTcpSocket* sock);

protected:
    void run();

};

#endif // READANDHANDLE_H
