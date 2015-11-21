#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <QtCore/QLinkedList>
#include <QtCore/QPointer>
#include <QtGlobal>

class QThread;
class Thread;

// it works with QThread instead of QRunnable as QThreadPool does
class ThreadPool
{
public:
    ThreadPool(quint32 maxThreadCount = 1);
    ~ThreadPool();

    quint32 maxThreadCount() const;
    void setMaxThreadCount(quint32 maxThreadCount);

    QThread* acquireThread();
    void releaseThread(QThread* thread);

private:
    QLinkedList<QPointer<Thread>> threads_;
    quint32 maxThreadCount_;
};

#endif // _THREADPOOL_H_
