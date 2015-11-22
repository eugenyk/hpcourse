#ifndef _CHAT_WINDOW_H_
#define _CHAT_WINDOW_H_

#include <QtCore/QSharedMemory>
#include <QtWidgets/QWidget>

class QPushButton;
class QTcpSocket;
class QTextEdit;
class QLineEdit;
class IOSocket;

namespace server {
namespace proto {
class Message;
}
}

class Chat : public QWidget
{
    Q_OBJECT

public:
    Chat(QTcpSocket* socket, QWidget* parent = 0);
    ~Chat();

private slots:
    void printMessage(const server::proto::Message& message);
    void sendMessage();
    void socketError();

private:
    void printError(const QString& error);

    QSharedMemory sharedMemory_;
    QPushButton* sendButton_;
    QTextEdit* textArea_;
    QLineEdit* lineEdit_;
    IOSocket* socket_;
    QString name_;

    const QColor defaultTextColor_;
};

#endif // _CHAT_WINDOW_H_
