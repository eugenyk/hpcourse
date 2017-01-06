#include "mainwindow.h"

void* listen_tcp(void* _args)
{
    struct listening_thread_args* args = (struct listening_thread_args*) _args;

    while(true)
    {
        std::string msg = args->tcp.receive();
        if(msg != "")
            std::cout << "Received: " << msg << std::endl;

        args->text->setText(msg.c_str());
        args->widget->update();
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    text = new QLabel(0);
    text->setText(0);
    scroll = new QScrollArea(0);
    //scroll->setWidget(text);
    la = new QGridLayout(0);
    //la->addWidget(scroll, 0, 0, 1, 3);
    la->addWidget(text, 0, 0, 1, 3);
    rec_info = new QLabel("Receiver ID");
    la->addWidget(rec_info, 1, 0, 1, 1);
    rec_id = new QLineEdit();
    la->addWidget(rec_id, 1, 1, 1, 1);
    msg_info = new QLabel("Message:");
    la->addWidget(msg_info, 2, 0, 1, 1);
    msg = new QLineEdit();
    la->addWidget(msg, 2, 1, 1, 1);
    send = new QPushButton("Send");
    la->addWidget(send, 2, 2, 1, 1);

    this->setLayout(la);

    if(tcp_client.connect_(32165))
    {
        id = tcp_client.receive();
        text->setText("My ID: " + QString(id.c_str()));

        //struct listening_thread_args args;
        args = new struct listening_thread_args;
        args->widget = this;
        args->text = text;
        args->tcp = tcp_client;
        pthread_create(&listening_thread, 0, listen_tcp, (void *)args);
    }

    connect(send, SIGNAL(released()), this, SLOT(send_text()));
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
    delete la;
    pthread_cancel(listening_thread);
    tcp_client.send("-");
    tcp_client.close_();
    delete args;
}

void MainWindow::send_text()
{
    tcp_client.send((rec_id->text()+msg->text()).toStdString());
}
