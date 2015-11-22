#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>

#include <QtNetwork/QTcpSocket>

#include <QtGui/QRegExpValidator>

#include "socket.h"
#include "Message.pb.h"
#include "connectdialog.h"

ConnectDialog::ConnectDialog(QTcpSocket* socket, QWidget* parent) : QDialog(parent), socket_(socket)
{
    setWindowTitle(tr("Connecting"));

    hostEdit_ = new QLineEdit("127.0.0.1", this);
    hostEdit_->setValidator(new QRegExpValidator(QRegExp("^(25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}|[0-9]{2}|[0-9])(\\.(25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}|[0-9]{2}|[0-9])){3}$"), this));
    portEdit_ = new QLineEdit("10000", this);
    portEdit_->setValidator(new QIntValidator(1, 65535, this));

    connectButton_ = new QPushButton(tr("Connect"), this);
    connect(connectButton_, &QPushButton::clicked, this, &ConnectDialog::connectPressed);
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, &QPushButton::clicked, this, &ConnectDialog::reject);

    QFormLayout* topLayout = new QFormLayout;
    topLayout->addRow("IP-address", hostEdit_);
    topLayout->addRow("Port", portEdit_);

    QHBoxLayout* botLayout = new QHBoxLayout;
    botLayout->addStretch();
    botLayout->addWidget(connectButton_);
    botLayout->addWidget(cancelButton);

    QFrame* hline = new QFrame(this);
    hline->setFrameShape(QFrame::HLine);

    QLabel* label = new QLabel(this);
    label->setText(tr("This machine uses %1").arg((QSysInfo::ByteOrder == QSysInfo::BigEndian ? tr("big-endian") : tr("little-endian"))));

    endianBox_ = new QCheckBox(this);
    endianBox_->setText(tr("Force use %1").arg((QSysInfo::ByteOrder == QSysInfo::BigEndian ? tr("little-endian") : tr("big-endian"))));
    endianBox_->setChecked(false);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(topLayout, 1);
    layout->addWidget(hline);
    layout->addWidget(label);
    layout->addWidget(endianBox_);
    layout->addStretch();
    layout->addLayout(botLayout);

    connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketError()));
    connect(socket_, &QTcpSocket::connected, this, &ConnectDialog::accept);
}

void ConnectDialog::connectPressed()
{
    if(hostEdit_->hasAcceptableInput())
    {
        if(portEdit_->hasAcceptableInput())
        {
            connectButton_->setEnabled(false);
            socket_->connectToHost(hostEdit_->text(), portEdit_->text().toInt());
        }
        else
            QMessageBox::critical(this, tr("Error"), tr("Incorrect port (<1 or >65535)"));
    }
    else
        QMessageBox::critical(this, tr("Error"), tr("Incorrect IP-address format"));
}

int ConnectDialog::exec()
{
    int ret = QDialog::exec();

    if(ret == QDialog::Accepted)
    {
        if(endianBox_->isChecked())
        {
            if(QSysInfo::ByteOrder == QSysInfo::BigEndian)
                IOSocket::setEndian(QSysInfo::LittleEndian);
            else
                IOSocket::setEndian(QSysInfo::BigEndian);
        }
    }

    return ret;
}

void ConnectDialog::socketError()
{
    switch(socket_->error())
    {
    case QAbstractSocket::HostNotFoundError :
        QMessageBox::information(this, tr("Failed to connect"), tr("The host was not found. Please check the IP-addess and port settings"));
        break;
    case QAbstractSocket::ConnectionRefusedError :
        QMessageBox::information(this, tr("Failed to connect"), tr("The connection was refused by the peer. Make sure the server is running, and check that the IP-address and port settings are correct"));
        break;
    default :
        QMessageBox::information(this, tr("Failed to connect"), tr("The following error occured: %1").arg(socket_->errorString()));
        break;
    }

    connectButton_->setEnabled(true);
}
