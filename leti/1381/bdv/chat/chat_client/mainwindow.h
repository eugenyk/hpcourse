#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QScrollBar>
#include "tcpclient.h"

#include <pthread.h>

struct listening_thread_args
{
    QWidget* widget;
    QLabel* text;
    TcpClient tcp;
};

bool getnameandmsg(std::string rec_data, std::string& name, std::string& msg);
void setnameandmsg(std::string& snd_data, std::string name, std::string msg);
void* listen_tcp(void* _args);

class MainWindow : public QWidget
{
    Q_OBJECT

private:
    TcpClient tcp_client;
    QLabel* name_info;
    QLineEdit* name;
    QPushButton* connect_btn;
    QScrollArea* scroll;
    QLabel* text;
    QLabel* rec_info;
    QLineEdit* rec_id;
    QLabel* msg_info;
    QLineEdit* msg;
    QPushButton* send;

    QGridLayout* la;

    std::string id;
    pthread_t listening_thread;
    struct listening_thread_args* args;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void connect_to_srv();
    void send_text();
    void slide_max(int min, int max);
};

#endif // MAINWINDOW_H
