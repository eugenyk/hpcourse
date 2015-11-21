#ifndef EXECUTION_THREAD_H
#define EXECUTION_THREAD_H

#include <QtCore/QWaitCondition>
#include <QtCore/QPointer>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QCache>

#include "Message.pb.h"

class IOSocketDecorator;
class Call;

class ExecutionThread : public QThread
{
    Q_OBJECT

public:
    explicit ExecutionThread(QObject* parent = 0);
    ~ExecutionThread();

    static ExecutionThread* instance();

    void enqueueCommand(const QPointer<IOSocketDecorator>& from, const server::proto::Message& command);

public slots:
    void start();
    void stop();

protected:
    void run();

private:
    void processError(int error);

private:
    struct QueueItem
    {
        QPointer<IOSocketDecorator> socket;
        server::proto::Message command;
    };

    QCache<QString,QDateTime> calls_;
    QQueue<QueueItem> queue_;
    QWaitCondition cond_;
    QMutex callsMutex_;
    QMutex cmdMutex_;
};

#endif // EXECUTION_THREAD_H
