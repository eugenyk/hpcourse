#ifndef READANDHANDLE_H
#define READANDHANDLE_H

#include <QRunnable>
#include <QTcpSocket>
#include <QThreadPool>
#include <QThread>
//#include <vector>
#include <tbb/concurrent_vector.h>

#include <string>
#include <iostream>
#include <unistd.h>

class ReadAndHandle : public QObject, public QRunnable
{
    Q_OBJECT
public:
    //считывает с сокета, находит в списке указатель на сокет получателя по имени и отправляет
    ReadAndHandle(QTcpSocket *socket,
                  std::vector<std::pair<std::string, QTcpSocket *> > sockets);

signals:
    void deleting_socket_from_store(QTcpSocket* socket);
    void setting_username(QTcpSocket* soctet, char* name, int size);

private:
    QTcpSocket* socket;
    std::vector<std::pair<std::string, QTcpSocket*> > sockets;

    static bool getnameandmsg(std::string rec_data, std::string& name, std::string& msg);
    static void setnameandmsg(std::string& snd_data, std::string name, std::string msg);
    void delete_name_sock(QTcpSocket *sock);
    std::string find_name_by_sock(QTcpSocket* sock);
    QTcpSocket* find_sock_by_name(std::string name);

protected:
    void run();

};

#endif // READANDHANDLE_H
