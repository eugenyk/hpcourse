#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "chat_client.h"
#include "area_message_form.h"
#include "login_form.h"
#include "message/message.pb.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    ChatClient *chatClient;
    AreaMessageForm *areaMessageForm;
    LoginForm *loginForm;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:

private:
    Ui::MainWindow *ui;
signals:
    void createdChatMessage(ChatMessage);
public slots:
    void showAreaMessageForm();
     void createChatMessage(QString textMessage);
};

#endif // MAIN_WINDOW_H
