#include <QtCore/QThread>
#include "threadpool.h"

class Thread : public QThread
{
    Q_OBJECT
public:
    Thread(QObject* parent = 0) : QThread(parent), ref_(1) {}
    quint64 ref_;
};

#include "threadpool.moc"

ThreadPool::ThreadPool(quint32 maxThreadCount) : maxThreadCount_(maxThreadCount)
{
}

ThreadPool::~ThreadPool()
{
    auto it = threads_.cbegin();
    while(it != threads_.cend())
    {
        auto thread = *it;
        if(thread)
        {
            thread->quit();
            thread->wait();
        }
        it++;
    }
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
    auto it = threads_.begin();
    while(it != threads_.end())
    {
        if(*it)
        {
            if(!thread)
                thread = *it;
            else
            {
                int ref = (*it)->ref_;
                if(ref == 0)
                    return it->data();
                else if(thread->ref_ > ref)
                    thread = *it;
            }

            it++;
        }
        else
            it = threads_.erase(it);
    }

    if(threads_.size() < maxThreadCount_)
    {
        //qDebug("create thread");
        thread = new Thread;
        thread->start();
        threads_.append(thread);
    }
    else
    {
        if(thread)
            ++thread->ref_;
        else
        {
            //qDebug("create thread");
            thread = new Thread;
            thread->start();
            threads_.append(thread);
        }
    }

    return thread.data();
}

void ThreadPool::releaseThread(QThread* thread)
{
    Thread* t = qobject_cast<Thread*>(thread);
    Q_ASSERT(t);
    if(t)
    {
        if(--t->ref_ == 0)
        {
            //qDebug("delete thread");
            t->quit();
            t->wait();
            delete t;
        }
    }
}



