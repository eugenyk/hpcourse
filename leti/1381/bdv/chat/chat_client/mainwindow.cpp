#include "mainwindow.h"


bool getnameandmsg(std::string rec_data, std::string& name, std::string& msg)
{
    int name_size = (int)rec_data[0];
    rec_data.erase(rec_data.begin());
    name = rec_data.substr(0, name_size);
    msg = rec_data.erase(0, name_size);
    return true;
}

void setnameandmsg(std::string& snd_data, std::string name, std::string msg)
{
    snd_data = "0" + name + msg;
    char name_size = name.size();
    snd_data[0] = name_size;
}

void* listen_tcp(void* _args)
{
    struct listening_thread_args* args = (struct listening_thread_args*) _args;
    while(true)
    {
        std::string rec_data = args->tcp.receive();
        std::string snd_name, msg;
        getnameandmsg(rec_data, snd_name, msg);
        rec_data = "From " + snd_name + ": " + msg + "\n";
        args->text->setText((args->text->text().toStdString() + rec_data).c_str());
        args->widget->update();
    }
    pthread_exit(0);
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    la = new QGridLayout(0);
    name_info = new QLabel("Enter your name");
    la->addWidget(name_info, 0, 0, 1, 1);
    name = new QLineEdit();
    la->addWidget(name, 0, 1, 1, 1);
    connect_btn = new QPushButton("Connect");
    la->addWidget(connect_btn, 0, 2, 1, 1);
    text = new QLabel(0);
    text->setText(0);
    scroll = new QScrollArea(0);
    scroll->setWidgetResizable(true);
    scroll->setWidget(text);
    la->addWidget(scroll, 1, 0, 1, 3);
    rec_info = new QLabel("Receiver ID");
    la->addWidget(rec_info, 2, 0, 1, 1);
    rec_id = new QLineEdit();
    la->addWidget(rec_id, 2, 1, 1, 1);
    msg_info = new QLabel("Message:");
    la->addWidget(msg_info, 3, 0, 1, 1);
    msg = new QLineEdit();
    la->addWidget(msg, 3, 1, 1, 1);
    send = new QPushButton("Send");
    la->addWidget(send, 3, 2, 1, 1);

    this->setLayout(la);

    connect(connect_btn, SIGNAL(released()), this, SLOT(connect_to_srv()));
    connect(scroll->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(slide_max(int,int)));
}

MainWindow::~MainWindow()
{
    delete send;
    delete msg;
    delete msg_info;
    delete rec_id;
    delete rec_info;
    delete text;
    delete scroll;
    delete name_info;
    delete name;
    delete connect_btn;
    delete la;
    pthread_cancel(listening_thread);
    tcp_client.send("----");
    tcp_client.close_();
    delete args;
}

void MainWindow::connect_to_srv()
{
    connect_btn->setVisible(false);
    name_info->setText("Your name:");
    name->setReadOnly(true);

    if(tcp_client.connect_(32165))
    {
        tcp_client.send("+" + name->text().toStdString());
        text->setText("Welcome, " + name->text() + "\n");

        args = new struct listening_thread_args;
        args->widget = this;
        args->text = text;
        args->tcp = tcp_client;
        pthread_create(&listening_thread, 0, listen_tcp, (void *)args);
        connect(send, SIGNAL(released()), this, SLOT(send_text()));
    }
}

void MainWindow::send_text()
{
    std::string snd_data, rec_name, msg_;
    text->setText(text->text() + "To " + rec_id->text() + ": " + msg->text() + "\n");
    rec_name = rec_id->text().toStdString();
    msg_ = msg->text().toStdString();
    setnameandmsg(snd_data, rec_name, msg_);
    tcp_client.send(snd_data);
    msg->setText("");
}

void MainWindow::slide_max(int min, int max)
{
    QScrollBar* bar = scroll->verticalScrollBar();
    bar->setValue(max);
}
