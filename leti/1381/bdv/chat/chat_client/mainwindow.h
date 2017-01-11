#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QApplication>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QScrollBar>
#include <QThreadPool>
#include "tcpclient.h"
#include "../protobuff/chatmessage.pb.h"

class MainWindow : public QWidget
{
    Q_OBJECT

private:
    TcpClient* tcp_client;
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
    void disconnect_from_srv();
    void connected_to_srv();
    void send_text();
    void slide_max(int min, int max);
    void read_message();

};

#endif // MAINWINDOW_H
