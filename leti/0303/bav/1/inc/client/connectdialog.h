#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QtWidgets/QDialog>

class QPushButton;
class QTcpSocket;
class QLineEdit;
class QCheckBox;

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    ConnectDialog(QTcpSocket* socket, QWidget* parent = 0);

public slots:
    int exec();

private slots:
    void connectPressed();
    void socketError();

private:
    QPushButton* connectButton_;
    QCheckBox* endianBox_;
    QLineEdit* hostEdit_;
    QLineEdit* portEdit_;
    QTcpSocket* socket_;
};

#endif // CONNECTDIALOG_H
