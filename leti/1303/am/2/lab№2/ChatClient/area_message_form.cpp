#include "area_message_form.h"
#include "ui_area_message_form.h"

AreaMessageForm::AreaMessageForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AreaMessageForm)
{
    ui->setupUi(this);
    connect(ui->sendMessageButton,SIGNAL(pressed()),this,SLOT(getTextMessageFromForm()));
}

void AreaMessageForm::getTextMessageFromForm(){
    QString textMessage= ui->messageLineEdit->text();
    emit gettedTextMessageFromForm(textMessage);
}

void AreaMessageForm::addMessageToMessageBrowser(QString message){

    ui->messageBrowser->append(message);
}

void AreaMessageForm::addMessageToMessageBrowser(ChatMessage message){
   std:: string line=message.sender()+":"+ message.text();
   ui->messageBrowser->append(line.data());
}


AreaMessageForm::~AreaMessageForm()
{
    delete ui;
}
