#include <QtCore/QThread>
#include "my_threadpool.h"

class Thread : public QThread
{
    Q_OBJECT

public:
    QAtomicInteger<quint32> ref_;

    Thread() : ref_(1)
    {
    }
    ~Thread()
    {
        quit();
        wait();
    }

    // returns false if Thread has been marked to be deleted
    bool increaseExternalCount()
    {
        forever
        {
            const quint32 count = ref_.loadAcquire();
            if(count == 0)
                return false;
            else
            {
                if(ref_.testAndSetOrdered(count, count + 1))
                    return true;
            }
        }
    }

    bool freeExternalCount()
    {
        forever
        {
            const quint32 count = ref_.loadAcquire();
            if(count == 0)
                return false;
            else
            {
                if(ref_.testAndSetOrdered(count, count - 1))
                {
                    if(count == 1)
                        quit();

                    return true;
                }
            }
        }
    }
};

#include "my_threadpool.moc"

ThreadPool::ThreadPool(quint32 maxThreadCount) : maxThreadCount_(maxThreadCount)
{
}

ThreadPool::~ThreadPool()
{
    qDeleteAll(threads_);
}

quint32 ThreadPool::maxThreadCount() const
{
    return maxThreadCount_;
}

void ThreadPool::setMaxThreadCount(quint32 maxThreadCount)
{
    maxThreadCount_ = maxThreadCount;
}

QThread* ThreadPool::acquireThread()
{
    QPointer<Thread> thread;

    if(threads_.size() < maxThreadCount_)
    {
        thread = new Thread;
        thread->start();
        threads_.append(thread);
    }
    else
    {
        auto it = threads_.begin();
        while(it != threads_.end())
        {
            if((*it)->increaseExternalCount())
            {
                if(thread)
                {
                    quint32 ref = (*it)->ref_.loadAcquire();
                    if(thread->ref_.loadAcquire() > ref)
                    {
                        thread->freeExternalCount();
                        thread = *it;
                    }
                    else
                        (*it)->freeExternalCount();
                }
                else
                    thread = *it;
            }
            else
            {
                Thread* tmp = it->data();
                *it = nullptr;
                delete tmp;

                thread = new Thread;
                thread->start();
                *it = thread;
                break;
            }

            it++;
        }
    }

    Q_ASSERT(thread);
    return thread.data();
}

void ThreadPool::releaseThread(QThread* thread)
{
    Thread* t = qobject_cast<Thread*>(thread);
    Q_ASSERT(t);
    if(t)
        t->freeExternalCount();
}



