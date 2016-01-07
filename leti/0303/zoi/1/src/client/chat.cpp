#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLineEdit>

#include <QtCore/QSystemSemaphore>

#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QHostInfo>

#include "Message.pb.h"
#include "socket.h"
#include "chat.h"

static QString acquireUniqueName(QSharedMemory& sharedMemory)
{
    bool created = false;

    QSystemSemaphore semaphone("sharedMemory", 1, QSystemSemaphore::Open);
    semaphone.acquire();

    if(!sharedMemory.attach())
    {
        if(!(sharedMemory.create(sizeof(qintptr)) || sharedMemory.attach()))
        {
            QMessageBox::critical(0, QObject::tr("Error"), QObject::tr("Cannot obtain access to the shared memory\n") + sharedMemory.errorString());
            qApp->quit();
        }
        else
            created = true;
    }

    semaphone.release();

    QString name;

#if defined(Q_OS_WIN)
    name = QString::fromLocal8Bit(qgetenv("USERNAME").constData()).toUtf8();
#else //if defined(Q_OS_UNIX)
    name = qgetenv("USER").constData();
#endif

    if(name.isEmpty())
        name = QHostInfo().localHostName();

    Q_ASSUME(!name.isEmpty());
    if(Q_UNLIKELY(name.isEmpty()))
    {
        for(const QHostAddress& host : QNetworkInterface::allAddresses())
            if(host != QHostAddress::LocalHost && host.protocol() == QAbstractSocket::IPv4Protocol)
            {
                name = host.toString();
                break;
            }

        if(name.isEmpty())
            name = QHostAddress(QHostAddress::LocalHost).toString();
    }

    sharedMemory.lock();
    if(created)
    {
        void* data = sharedMemory.data();
        *((qintptr*)data) = (qintptr)1;
    }
    else
    {
        qintptr count = *((qintptr*)sharedMemory.data());
        ++count;
        void* data = sharedMemory.data();
        *((qintptr*)data) = count;
        name.append(" - ");
        name.append(QString::number(count));
    }
    sharedMemory.unlock();
    return name;
}

static void releaseUniqueName(QSharedMemory& sharedMemory)
{
    sharedMemory.lock();
    qintptr count = *((qintptr*)sharedMemory.data());
    --count;
    void* data = sharedMemory.data();
    *((qintptr*)data) = count;
    sharedMemory.unlock();
}

Chat::Chat(QTcpSocket* socket, QWidget* parent)
    : QWidget(parent)
    , textArea_(new QTextEdit(this))
    , defaultTextColor_(textArea_->textColor())
    , socket_(new IOSocket(socket))
{
    textArea_->setUndoRedoEnabled(false);
    textArea_->setReadOnly(true);
    textArea_->setFocusPolicy(Qt::NoFocus);

    lineEdit_ = new QLineEdit(this);
    lineEdit_->setClearButtonEnabled(true);
    lineEdit_->setPlaceholderText(tr("Input your message here..."));
    connect(lineEdit_, &QLineEdit::returnPressed, this, &Chat::sendMessage);
    lineEdit_->setFocusPolicy(Qt::StrongFocus);

    sendButton_ = new QPushButton(tr("Send"), this);
    connect(sendButton_, &QPushButton::clicked, this, &Chat::sendMessage);
    sendButton_->setFocusPolicy(Qt::ClickFocus);

    QHBoxLayout* botLayout = new QHBoxLayout;
    botLayout->addWidget(lineEdit_, 1);
    botLayout->addWidget(sendButton_);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(textArea_, 1);
    layout->addLayout(botLayout);

    connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketError()));
    connect(socket_, &IOSocket::read, this, &Chat::printMessage);

    sharedMemory_.setKey("ChatUserId");
    name_ = acquireUniqueName(sharedMemory_);
    setWindowTitle(name_);

    resize(400, 500);
}

Chat::~Chat()
{
    releaseUniqueName(sharedMemory_);
}

void Chat::printMessage(const server::proto::Message& message)
{
    if(!textArea_->toPlainText().isEmpty())
        textArea_->append("\n");

    if(message.sender().empty() && !message.data().empty())
    {
        textArea_->setTextColor(Qt::gray);
        textArea_->append(QString("%1: %2").arg(QString::fromStdString(message.data())).arg(QString::fromStdString(message.text())));
        textArea_->setTextColor(defaultTextColor_);
    }
    else
    {
        textArea_->setTextColor(Qt::darkBlue);
        textArea_->setFontWeight(QFont::Bold);

        QString senderEscaped = QString::fromStdString(message.sender()).toHtmlEscaped();
        senderEscaped.replace("&amp;", "&");
        textArea_->append(senderEscaped);

        textArea_->setFontWeight(QFont::Normal);
        textArea_->setTextColor(defaultTextColor_);

        QString textEscaped = QString::fromStdString(message.text()).toHtmlEscaped();
        senderEscaped.replace("&amp;", "&");
        textArea_->append(textEscaped);
    }

    qApp->processEvents(QEventLoop::AllEvents, 100);
}

void Chat::sendMessage()
{
    if(socket_->state() == QAbstractSocket::ConnectedState)
    {
        QString text = lineEdit_->text();
        if(!text.simplified().isEmpty())
        {
            lineEdit_->clear();

            server::proto::Message message;
            message.set_sender(windowTitle().toStdString());
            message.set_text(text.toStdString()); // send text without escaping

            if(socket_->write(message))
            {
                if(!textArea_->toPlainText().isEmpty())
                    textArea_->append("\n");

                textArea_->setTextColor(Qt::darkRed);
                textArea_->setFontWeight(QFont::Bold);
                textArea_->append(QString("You (%1)").arg(name_));
                textArea_->setFontWeight(QFont::Normal);
                textArea_->setTextColor(defaultTextColor_);
                textArea_->append(text.toHtmlEscaped()); // escape text in the chat

                textArea_->verticalScrollBar()->setValue(textArea_->verticalScrollBar()->maximum());
            }
            else
                socketError();
        }
    }
}

void Chat::socketError()
{
    if(socket_->error() != QAbstractSocket::UnknownSocketError)
    {
        if(socket_->error() == QAbstractSocket::RemoteHostClosedError)
        {
            QMessageBox::information(this, tr("Shutdown"), socket_->errorString());
            qApp->quit();
        }
        else
            printError(socket_->errorString());
    }
}

void Chat::printError(const QString& error)
{
    if(!textArea_->toPlainText().isEmpty())
        textArea_->append("\n");

    textArea_->setTextColor(Qt::magenta);
    textArea_->setFontItalic(true);
    textArea_->append(error);
    textArea_->setFontItalic(false);
    textArea_->setTextColor(defaultTextColor_);
}
