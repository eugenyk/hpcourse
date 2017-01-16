#ifndef AREA_MESSAGE_FORM_H
#define AREA_MESSAGE_FORM_H

#include <QWidget>
#include "message/message.pb.h"
namespace Ui {
class AreaMessageForm;
}

class AreaMessageForm : public QWidget
{
    Q_OBJECT

public:
    explicit AreaMessageForm(QWidget *parent = 0);
    ~AreaMessageForm();

private:
    Ui::AreaMessageForm *ui;
signals:
    void gettedTextMessageFromForm(QString);
public slots:
    void getTextMessageFromForm();
    void addMessageToMessageBrowser(QString);
     void addMessageToMessageBrowser(ChatMessage);
};

#endif // AREA_MESSAGE_FORM_H
