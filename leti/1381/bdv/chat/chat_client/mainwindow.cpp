#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

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
    rec_info = new QLabel("Receiver name");
    la->addWidget(rec_info, 2, 0, 1, 1);
    rec_id = new QLineEdit();
    la->addWidget(rec_id, 2, 1, 1, 1);
    msg_info = new QLabel("Message:");
    la->addWidget(msg_info, 3, 0, 1, 1);
    msg = new QLineEdit();
    la->addWidget(msg, 3, 1, 1, 1);
    send = new QPushButton("Send");
    la->addWidget(send, 3, 2, 1, 1);

    rec_info->setVisible(false);
    rec_id->setVisible(false);
    msg_info->setVisible(false);
    msg->setVisible(false);
    send->setVisible(false);

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

    google::protobuf::ShutdownProtobufLibrary();
}

void MainWindow::connect_to_srv()
{
    connect(tcp_client->getSocket(), SIGNAL(connected()), this, SLOT(connected_to_srv()));
    tcp_client->connect_(32165);
}

void MainWindow::disconnect_from_srv()
{
    ChatMessage msg;
    msg.set_msgtype(ChatMessage::Command::ChatMessage_Command_DISC);
    msg.set_name(name->text().toStdString());
    std::string s;
    msg.SerializeToString(&s);
    tcp_client->send(s);
    tcp_client->close_();
}

void MainWindow::send_text()
{
    std::string snd_data, rec_name, msg_;
    text->setText(text->text() + "To " + rec_id->text() + ": " + msg->text() + "\n");
    rec_name = rec_id->text().toStdString();
    msg_ = msg->text().toStdString();
    ChatMessage msg;
    msg.set_msgtype(ChatMessage::Command::ChatMessage_Command_SEND);
    msg.set_name(rec_name);
    msg.set_message(msg_);
    msg.SerializeToString(&snd_data);
    tcp_client->send(snd_data);
    this->msg->setText("");
}

void MainWindow::slide_max(int min, int max)
{
    QScrollBar* bar = scroll->verticalScrollBar();
    bar->setValue(max);
}

void MainWindow::read_message()
{
    std::string rec_data = tcp_client->receive();
    ChatMessage msg;
    msg.ParseFromString(rec_data);
    rec_data = "From " + msg.name() + ": " + msg.message() + "\n";
    text->setText((text->text().toStdString() + rec_data).c_str());
    update();
}

void MainWindow::connected_to_srv()
{
    rec_info->setVisible(true);
    rec_id->setVisible(true);
    msg_info->setVisible(true);
    msg->setVisible(true);
    send->setVisible(true);

    connect_btn->setVisible(false);
    name_info->setText("Your name:");
    name->setReadOnly(true);
    text->setText("Welcome, " + name->text() + "\n");
    ChatMessage msg;
    msg.set_msgtype(ChatMessage::Command::ChatMessage_Command_INIT);
    msg.set_name(name->text().toStdString());
    std::string s;
    msg.SerializeToString(&s);
    tcp_client->send(s);
    connect(tcp_client->getSocket(), SIGNAL(readyRead()), this, SLOT(read_message()));
    connect(send, SIGNAL(released()), this, SLOT(send_text()));
}
