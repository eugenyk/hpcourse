#ifndef DIALOGCONNECT_H
#define DIALOGCONNECT_H

#include <QDialog>

#include <QString>
#include <QtNetwork>
#include <QNetworkSession>
#include <QTcpSocket>
#include <QHostInfo>
#include <QMessageBox>

#include "controller.h"

namespace Ui {
class DialogConnect;
}

class DialogConnect : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConnect(QWidget *parent = 0);
    ~DialogConnect();

private:
    Ui::DialogConnect *ui;
    Controller *m_controller;

private slots:
    void connectSocketError(QAbstractSocket::SocketError socketError);
    void setUp_Ip_Host();

};

#endif // DIALOGCONNECT_H
