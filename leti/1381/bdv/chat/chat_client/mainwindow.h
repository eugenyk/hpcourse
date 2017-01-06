#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include "tcpclient.h"

#include <pthread.h>

struct listening_thread_args
{
    QWidget* widget;
    QLabel* text;
    TcpClient tcp;
};

void* listen_tcp(void* _args);

class MainWindow : public QWidget
{
    Q_OBJECT

private:
    TcpClient tcp_client;
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
    void send_text();
};

#endif // MAINWINDOW_H
