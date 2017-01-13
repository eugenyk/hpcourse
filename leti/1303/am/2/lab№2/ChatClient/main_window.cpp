#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loginForm=new LoginForm(this);
    ui->mainLayout->addWidget(loginForm);

    areaMessageForm=new AreaMessageForm(this);
    ui->mainLayout->addWidget(areaMessageForm);
    areaMessageForm->hide();

    chatClient=new ChatClient();
    connect(loginForm,SIGNAL(gettedLoginFromForm(QString)),chatClient,SLOT(connectToServer(QString)));

    connect(chatClient,SIGNAL(connectedToServer()),loginForm,SLOT(hide()));
    //connect(chatClient,SIGNAL(connectedToServer()),this,SLOT(showAreaMessageForm()));
    connect(chatClient,SIGNAL(connectedToServer()),areaMessageForm,SLOT(show()));

    connect(chatClient,SIGNAL(readedMessage(QString)),areaMessageForm,SLOT(addMessageToMessageBrowser(QString)));
    connect(chatClient,SIGNAL(readedMessage(ChatMessage)),areaMessageForm,SLOT(addMessageToMessageBrowser(ChatMessage)));

    connect(areaMessageForm,SIGNAL(gettedTextMessageFromForm(QString)),this,SLOT(createChatMessage(QString)));
    connect(this,SIGNAL(createdChatMessage(ChatMessage)),chatClient,SLOT(sendMessage(ChatMessage)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createChatMessage(QString textMessage){
    ChatMessage chatMessage;
    chatMessage.set_sender(chatClient->login.toStdString());
    chatMessage.set_text(textMessage.toStdString());
    emit createdChatMessage(chatMessage);
}

void MainWindow::showAreaMessageForm(){
     areaMessageForm=new AreaMessageForm(this);
     ui->mainLayout->addWidget(areaMessageForm);
}
