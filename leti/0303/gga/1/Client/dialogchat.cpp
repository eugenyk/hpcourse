#include "dialogchat.h"
#include "ui_dialogchat.h"

DialogChat::DialogChat(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogChat)
{
    ui->setupUi(this);

    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(onButtonSendClick()));

}

DialogChat::~DialogChat()
{
    delete ui;
}

void DialogChat::printMessage(const server::proto::Message &message)
{
    if(!ui->textEdit->toPlainText().isEmpty())
        ui->textEdit->append("\n");

    if(message.sender().empty() && !message.data().empty())
    {
        ui->textEdit->setTextColor(Qt::gray);
        ui->textEdit->append(QString("%1: %2").arg(QString::fromStdString(message.data())).arg(QString::fromStdString(message.text())));
        ui->textEdit->setTextColor(Qt::black);
    }
    else
    {
        ui->textEdit->setTextColor(Qt::darkBlue);
        ui->textEdit->setFontWeight(QFont::Bold);

        QString senderEscaped = QString::fromStdString(message.sender()).toHtmlEscaped();
        senderEscaped.replace("&amp;", "&");
        ui->textEdit->append(senderEscaped);

        ui->textEdit->setFontWeight(QFont::Normal);
        ui->textEdit->setTextColor(Qt::blue);

        QString textEscaped = QString::fromStdString(message.text()).toHtmlEscaped();
        senderEscaped.replace("&amp;", "&");
        ui->textEdit->append(textEscaped);
    }

    qApp->processEvents(QEventLoop::AllEvents, 100);
}

void DialogChat::onButtonSendClick()
{

        QString text =ui->lineEdit->text();
        if(!text.simplified().isEmpty())
        {
            ui->lineEdit->clear();

            server::proto::Message message;
            message.set_sender(windowTitle().toStdString());
            message.set_text(text.toStdString()); // send text without escaping


            if(!ui->textEdit->toPlainText().isEmpty())
                ui->textEdit->append("\n");

            ui->textEdit->setTextColor(Qt::darkRed);
            ui->textEdit->setFontWeight(QFont::Bold);
            ui->textEdit->append(QString("You (%1)").arg("_m"));
            ui->textEdit->setFontWeight(QFont::Normal);
            ui->textEdit->setTextColor(Qt::red);
            ui->textEdit->append(text.toHtmlEscaped()); // escape text in the chat


            emit sendMessage(message);
        }

}

