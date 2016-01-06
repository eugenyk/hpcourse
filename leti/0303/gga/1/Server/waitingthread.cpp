#include <QtCore/QDateTime>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

#ifdef Q_OS_WIN
#include <QtCore/QTextCodec>
#endif

#include "socketdecorator.h"
#include "waitingthread.h"

#define START_PROCESS_TIMEOUT   1000
#define FINISH_PROCESS_TIMEOUT  2000
#define COMMAND_TIMEOUT         5000

WaitingThread* WaitingThread::instance()
{
    static WaitingThread instance_;
    return &instance_;
}

WaitingThread::WaitingThread(QObject* parent) : QThread(parent), calls_(10000)
{
}

WaitingThread::~WaitingThread()
{
    stop();
    wait();
}

void WaitingThread::enqueueCommand(const QPointer<SocketDecorator>& from, const server::proto::Message& command)
{
    QueueItem item;
    item.socket = from;
    item.command = command;

    QString name = QString::fromStdString(item.command.sender());
    QDateTime timestamp = QDateTime::currentDateTime();
    bool commandAccepted = true;

    item.command.set_data(item.command.text());
    item.command.set_sender("");

    callsMutex_.lock();
    QDateTime* lastCall = calls_.object(name);
    if(lastCall)
    {
        int timeout = COMMAND_TIMEOUT - lastCall->msecsTo(timestamp);
        if(timeout > 0)
        {
            callsMutex_.unlock();
            item.command.set_text(tr("You can't send commands within %1 ms").arg(timeout).toStdString());
            commandAccepted = false;
        }
        else
        {
            lastCall->swap(timestamp);
            callsMutex_.unlock();
        }
    }
    else
    {
        calls_.insert(name, new QDateTime(timestamp));
        callsMutex_.unlock();
    }

    if(commandAccepted)
    {
        QMutexLocker lock(&cmdMutex_);
        queue_.enqueue(item);
        cond_.wakeOne();
    }
    else
    {
        if(item.socket)
            item.socket->m_writeAsync(item.command);
    }
}

void WaitingThread::start()
{
    QThread::start(QThread::LowPriority);
}

void WaitingThread::stop()
{
    QMutexLocker lock(&cmdMutex_);
    requestInterruption();
    cond_.wakeOne();
}

void WaitingThread::run()
{
    while(!isInterruptionRequested())
    {
        cmdMutex_.lock();
        while(!queue_.isEmpty() && !isInterruptionRequested())
        {
            QueueItem item = queue_.dequeue();
            cmdMutex_.unlock();

            server::proto::Message msg = item.command;

            QProcess process;
            process.setProcessChannelMode(QProcess::MergedChannels);
            process.start(msg.text().c_str());
            msg.clear_text();

            bool success = false;

            if(process.waitForStarted(START_PROCESS_TIMEOUT))
            {
                if(process.waitForFinished(FINISH_PROCESS_TIMEOUT))
                {
                    success = true;

                    if(process.exitCode() == 0 || process.error() == QProcess::UnknownError)
                    {
                        QByteArray buffer = process.readAll();
#ifdef Q_OS_WIN
                        static QTextCodec* codec = QTextCodec::codecForName("IBM 866");
                        if(!buffer.isEmpty())
                            msg.set_text(codec->toUnicode(buffer).toStdString());
#else
                        if(!buffer.isEmpty())
                            msg.set_text(buffer.constData(), buffer.size());
#endif
                    }
                    else
                    {
                        QByteArray buffer = process.errorString().toLocal8Bit();
                        if(!buffer.isEmpty())
                            msg.set_text(buffer.constData(), buffer.size());
                    }
                }
            }

            if(!success)
            {
                if(process.error() != QProcess::UnknownError)
                {
                    QByteArray buffer = process.errorString().toLocal8Bit();
                    if(!buffer.isEmpty())
                        msg.set_text(buffer.constData(), buffer.size());
                    process.kill();
                }
            }

            if(!msg.text().empty() && item.socket)
                item.socket->m_writeAsync(msg);

            cmdMutex_.lock();
        }

        if(!isInterruptionRequested())
            cond_.wait(&cmdMutex_);
        cmdMutex_.unlock();
    }
}
