//$$CDS-header$$

#ifndef __UNIT_QUEUE_STD_QUEUE_H
#define __UNIT_QUEUE_STD_QUEUE_H

#include <queue>
#include <cds/lock/spinlock.h>

namespace queue {

    template <typename T, class CONTAINER, class LOCK = cds::lock::Spin >
    class StdQueue: public std::queue<T, CONTAINER >
    {
        typedef std::queue<T, CONTAINER >   base_class   ;
        LOCK    m_Locker    ;

    public:
        bool enqueue( const T& data )
        {
            cds::lock::scoped_lock<LOCK> a(m_Locker) ;

            base_class::push( data ) ;
            return true;
        }
        bool push( const T& data )  { return enqueue( data ) ; }
        bool dequeue( T& data )
        {
            cds::lock::scoped_lock<LOCK> a(m_Locker) ;
            if ( base_class::empty() )
                return false        ;

            data = base_class::front()   ;
            base_class::pop()    ;
            return true     ;
        }
        bool pop( T& data )         { return dequeue( data ) ; }

        cds::opt::none statistics() const
        {
            return cds::opt::none() ;
        }
    };

    template <typename T, class LOCK = cds::lock::Spin >
    class StdQueue_deque: public StdQueue<T, std::deque<T>, LOCK >
    {};

    template <typename T, class LOCK = cds::lock::Spin >
    class StdQueue_list: public StdQueue<T, std::list<T>, LOCK >
    {};
}

#endif // #ifndef __UNIT_QUEUE_STD_QUEUE_H
