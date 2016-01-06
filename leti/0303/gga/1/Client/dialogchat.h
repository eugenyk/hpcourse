#ifndef DIALOGCHAT_H
#define DIALOGCHAT_H

#include <QDialog>

#include "Message.pb.h"

namespace Ui {
class DialogChat;
}

class DialogChat : public QDialog
{
    Q_OBJECT

public:
    explicit DialogChat(QWidget *parent = 0);
    ~DialogChat();

public slots:
    void printMessage(const server::proto::Message& message);
    void onButtonSendClick();

signals:
    void sendMessage(const server::proto::Message& message);

private:
    Ui::DialogChat *ui;
};

#endif // DIALOGCHAT_H
