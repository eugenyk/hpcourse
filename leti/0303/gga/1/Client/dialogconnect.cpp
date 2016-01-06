#include "dialogconnect.h"
#include "ui_dialogconnect.h"

DialogConnect::DialogConnect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConnect)
{
    ui->setupUi(this);


    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        ui->comboBox->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty())
            ui->comboBox->addItem(name + QChar('.') + domain);
    }
    if (name != QString("localhost"))
        ui->comboBox->addItem(QString("localhost"));
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback())
            ui->comboBox->addItem(ipAddressesList.at(i).toString());
    }


    connect(ui->quitButton,SIGNAL(clicked(bool)),this,SLOT(close()));


    m_controller=new Controller(this);
    connect(ui->connectButton, SIGNAL(clicked()),this,SLOT(setUp_Ip_Host()));
    connect(m_controller,SIGNAL(connectionEstablished()),this,SLOT(close()));
    connect(m_controller,SIGNAL(displayError(QAbstractSocket::SocketError)),this,SLOT(connectSocketError(QAbstractSocket::SocketError)));

}

DialogConnect::~DialogConnect()
{
    delete ui;
}


void DialogConnect::connectSocketError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Client"),
                                 tr("The following error occurred: %1.")
                                 .arg("tcpSocket->errorString())"));
    }

}

void DialogConnect::setUp_Ip_Host()
{
    m_controller->setUp_Connection_With_Server(ui->comboBox->currentText(),ui->port_LineEdit->text().toInt());
}



