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
    connect(QApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(disconnect_from_srv()));

    tcp_client = new TcpClient();
}

MainWindow::~MainWindow()
{
    delete tcp_client;

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
}

void MainWindow::connect_to_srv()
{
    tcp_client->connect_(32165);
    connect_btn->setVisible(false);
    name_info->setText("Your name:");
    name->setReadOnly(true);
    text->setText("Welcome, " + name->text() + "\n");
    std::string s = "+" + name->text().toStdString();
    tcp_client->send(s);
    connect(tcp_client, SIGNAL(ready_read()), this, SLOT(read_message()));
    connect(send, SIGNAL(released()), this, SLOT(send_text()));
}

void MainWindow::disconnect_from_srv()
{
    tcp_client->send("----");
    tcp_client->close_();
}

void MainWindow::send_text()
{
    std::string snd_data, rec_name, msg_;
    text->setText(text->text() + "To " + rec_id->text() + ": " + msg->text() + "\n");
    rec_name = rec_id->text().toStdString();
    msg_ = msg->text().toStdString();
    setnameandmsg(snd_data, rec_name, msg_);
    //std::cout << "Sent: " << snd_data << std::endl;
    tcp_client->send(snd_data);
    msg->setText("");
}

void MainWindow::slide_max(int min, int max)
{
    QScrollBar* bar = scroll->verticalScrollBar();
    bar->setValue(max);
}

void MainWindow::read_message()
{
    std::string rec_data = tcp_client->receive();
    //std::cout << "Received: " << rec_data << std::endl;
    std::string snd_name, msg;
    getnameandmsg(rec_data, snd_name, msg);
    rec_data = "From " + snd_name + ": " + msg + "\n";
    text->setText((text->text().toStdString() + rec_data).c_str());
    update();
}
