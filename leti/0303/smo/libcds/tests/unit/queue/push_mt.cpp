//$$CDS-header$$

#include "cppunit/thread.h"
#include "queue/queue_type.h"

// Multi-threaded queue test for push operation
namespace queue {

#define TEST_CASE( Q, V ) void Q() { test< Types<V>::Q >(); }
#define TEST_BOUNDED( Q, V ) void Q() { test_bounded< Types<V>::Q >(); }

    namespace ns_Queue_Push {
        static size_t s_nThreadCount = 8  ;
        static size_t s_nQueueSize = 20000000 ;   // no more than 20 million records

        struct SimpleValue {
            size_t      nNo ;

            SimpleValue(): nNo(0) {}
            SimpleValue( size_t n ): nNo(n) {}
            size_t getNo() const { return  nNo; }
        };
    }
    using namespace ns_Queue_Push ;

    class Queue_Push: public CppUnitMini::TestCase
    {
        template <class Queue>
        class Thread: public CppUnitMini::TestThread
        {
            virtual TestThread *    clone()
            {
                return new Thread( *this )  ;
            }
        public:
            Queue&              m_Queue         ;
            double              m_fTime         ;
            size_t              m_nStartItem    ;
            size_t              m_nEndItem      ;
            size_t              m_nPushError    ;

        public:
            Thread( CppUnitMini::ThreadPool& pool, Queue& q )
                : CppUnitMini::TestThread( pool )
                , m_Queue( q )
            {}
            Thread( Thread& src )
                : CppUnitMini::TestThread( src )
                , m_Queue( src.m_Queue )
            {}

            Queue_Push&  getTest()
            {
                return reinterpret_cast<Queue_Push&>( m_Pool.m_Test )   ;
            }

            virtual void init()
            {
                cds::threading::Manager::attachThread()     ;
            }
            virtual void fini()
            {
                cds::threading::Manager::detachThread()   ;
            }

            virtual void test()
            {
                m_fTime = m_Timer.duration()    ;

                m_nPushError = 0    ;
                for ( size_t nItem = m_nStartItem; nItem < m_nEndItem; ++nItem ) {
                    if ( !m_Queue.push( nItem ))
                        ++m_nPushError ;
                }

                m_fTime = m_Timer.duration() - m_fTime  ;
            }
        };

    protected:
        template <class Queue>
        void analyze( CppUnitMini::ThreadPool& pool, Queue& testQueue  )
        {
            size_t nThreadItems = s_nQueueSize / s_nThreadCount ;
            double fTime = 0    ;
            for ( CppUnitMini::ThreadPool::iterator it = pool.begin(); it != pool.end(); ++it ) {
                Thread<Queue> * pThread = reinterpret_cast<Thread<Queue> *>(*it)  ;
                fTime += pThread->m_fTime   ;
                if ( pThread->m_nPushError != 0 )
                    CPPUNIT_MSG("     ERROR: thread push error count=" << pThread->m_nPushError ) ;
            }
            CPPUNIT_MSG( "     Duration=" << (fTime / s_nThreadCount) )   ;
            CPPUNIT_CHECK( !testQueue.empty() )

            size_t * arr = new size_t[ s_nQueueSize ]           ;
            memset(arr, 0, sizeof(arr[0]) * s_nQueueSize )      ;

            cds::OS::Timer      timer   ;
            CPPUNIT_MSG( "   Pop (single-threaded)..." )    ;
            size_t nPopped = 0      ;
            SimpleValue val = SimpleValue() ;
            while ( testQueue.pop( val )) {
                nPopped++   ;
                ++arr[ val.getNo() ]    ;
            }
            CPPUNIT_MSG( "     Duration=" << timer.duration() )     ;

            size_t nTotalItems = nThreadItems * s_nThreadCount      ;
            size_t nError = 0   ;
            for ( size_t i = 0; i < nTotalItems; ++i ) {
                if ( arr[i] != 1 ) {
                    CPPUNIT_MSG( "   ERROR: Item " << i << " has not been pushed" ) ;
                    CPPUNIT_ASSERT( ++nError <= 10 ) ;
                }
            }

            delete [] arr ;
        }

        template <class Queue>
        void test()
        {
            Queue testQueue ;

            CppUnitMini::ThreadPool pool( *this )   ;
            pool.add( new Thread<Queue>( pool, testQueue ), s_nThreadCount )       ;

            size_t nStart = 0   ;
            size_t nThreadItemCount = s_nQueueSize / s_nThreadCount ;
            for ( CppUnitMini::ThreadPool::iterator it = pool.begin(); it != pool.end(); ++it ) {
                Thread<Queue> * pThread = reinterpret_cast<Thread<Queue> *>(*it)  ;
                pThread->m_nStartItem = nStart  ;
                nStart += nThreadItemCount      ;
                pThread->m_nEndItem = nStart    ;
            }

            CPPUNIT_MSG( "   Push test, thread count=" << s_nThreadCount << " ...")   ;
            pool.run()  ;

            analyze( pool, testQueue )     ;

            CPPUNIT_MSG( testQueue.statistics() )   ;
        }

        template <class Queue>
        void test_bounded()
        {
            size_t nStart = 0   ;
            size_t nThreadItemCount = s_nQueueSize / s_nThreadCount ;

            Queue testQueue( s_nQueueSize ) ;

            CppUnitMini::ThreadPool pool( *this )   ;
            pool.add( new Thread<Queue>( pool, testQueue ), s_nThreadCount )       ;

            for ( CppUnitMini::ThreadPool::iterator it = pool.begin(); it != pool.end(); ++it ) {
                Thread<Queue> * pThread = reinterpret_cast<Thread<Queue> *>(*it)  ;
                pThread->m_nStartItem = nStart  ;
                nStart += nThreadItemCount      ;
                pThread->m_nEndItem = nStart    ;
            }

            CPPUNIT_MSG( "   Push test, thread count=" << s_nThreadCount << " ...")   ;
            pool.run()  ;

            analyze( pool, testQueue )     ;

            CPPUNIT_MSG( testQueue.statistics() )   ;
        }

        void setUpParams( const CppUnitMini::TestCfg& cfg ) {
            s_nThreadCount = cfg.getULong("ThreadCount", 8 ) ;
            s_nQueueSize = cfg.getULong("QueueSize", 20000000 );
        }

    protected:
        TEST_CASE( MoirQueue_HP, SimpleValue )
        TEST_CASE( MoirQueue_HP_seqcst, SimpleValue )
        TEST_CASE( MoirQueue_HP_ic, SimpleValue )
        TEST_CASE( MoirQueue_HP_stat, SimpleValue )
        TEST_CASE( MoirQueue_HRC, SimpleValue )
        TEST_CASE( MoirQueue_HRC_seqcst, SimpleValue )
        TEST_CASE( MoirQueue_HRC_ic, SimpleValue )
        TEST_CASE( MoirQueue_HRC_stat, SimpleValue )
        TEST_CASE( MoirQueue_PTB, SimpleValue )
        TEST_CASE( MoirQueue_PTB_seqcst, SimpleValue )
        TEST_CASE( MoirQueue_PTB_ic, SimpleValue )
        TEST_CASE( MoirQueue_PTB_stat, SimpleValue )

        TEST_CASE( MSQueue_HP, SimpleValue  )
        TEST_CASE( MSQueue_HP_seqcst, SimpleValue  )
        TEST_CASE( MSQueue_HP_ic, SimpleValue  )
        TEST_CASE( MSQueue_HP_stat, SimpleValue  )
        TEST_CASE( MSQueue_HRC, SimpleValue )
        TEST_CASE( MSQueue_HRC_seqcst, SimpleValue )
        TEST_CASE( MSQueue_HRC_ic, SimpleValue )
        TEST_CASE( MSQueue_HRC_stat, SimpleValue )
        TEST_CASE( MSQueue_PTB, SimpleValue )
        TEST_CASE( MSQueue_PTB_seqcst, SimpleValue )
        TEST_CASE( MSQueue_PTB_ic, SimpleValue )
        TEST_CASE( MSQueue_PTB_stat, SimpleValue )

        TEST_CASE(OptimisticQueue_HP, SimpleValue )
        TEST_CASE(OptimisticQueue_HP_seqcst, SimpleValue )
        TEST_CASE(OptimisticQueue_HP_ic, SimpleValue )
        TEST_CASE(OptimisticQueue_HP_stat, SimpleValue )
        TEST_CASE(OptimisticQueue_PTB, SimpleValue )
        TEST_CASE(OptimisticQueue_PTB_seqcst, SimpleValue )
        TEST_CASE(OptimisticQueue_PTB_ic, SimpleValue )
        TEST_CASE(OptimisticQueue_PTB_stat, SimpleValue )

        TEST_CASE( BasketQueue_HP, SimpleValue  )
        TEST_CASE( BasketQueue_HP_seqcst, SimpleValue  )
        TEST_CASE( BasketQueue_HP_ic, SimpleValue  )
        TEST_CASE( BasketQueue_HP_stat, SimpleValue  )
        TEST_CASE( BasketQueue_HRC, SimpleValue )
        TEST_CASE( BasketQueue_HRC_seqcst, SimpleValue )
        TEST_CASE( BasketQueue_HRC_ic, SimpleValue )
        TEST_CASE( BasketQueue_HRC_stat, SimpleValue )
        TEST_CASE( BasketQueue_PTB, SimpleValue )
        TEST_CASE( BasketQueue_PTB_seqcst, SimpleValue )
        TEST_CASE( BasketQueue_PTB_ic, SimpleValue )
        TEST_CASE( BasketQueue_PTB_stat, SimpleValue )

        TEST_CASE( RWQueue_Spin, SimpleValue )
        TEST_CASE( RWQueue_Spin_ic, SimpleValue )
        TEST_CASE( RWQueue_Spin_stat, SimpleValue )

        TEST_BOUNDED(TsigasCycleQueue_dyn, SimpleValue)
        TEST_BOUNDED(TsigasCycleQueue_dyn_ic, SimpleValue)

        TEST_BOUNDED(VyukovMPMCCycleQueue_dyn, SimpleValue)
        TEST_BOUNDED(VyukovMPMCCycleQueue_dyn_ic, SimpleValue)

        TEST_BOUNDED(MichaelDequeL_HP, SimpleValue)
        TEST_BOUNDED(MichaelDequeL_HP_ic, SimpleValue)
        TEST_BOUNDED(MichaelDequeL_HP_michaelAlloc, SimpleValue)
        TEST_BOUNDED(MichaelDequeL_HP_stat, SimpleValue)
        TEST_BOUNDED(MichaelDequeL_PTB, SimpleValue)
        TEST_BOUNDED(MichaelDequeL_PTB_ic, SimpleValue)
        TEST_BOUNDED(MichaelDequeL_PTB_michaelAlloc, SimpleValue)
        TEST_BOUNDED(MichaelDequeL_PTB_stat, SimpleValue)

        TEST_BOUNDED(MichaelDequeR_HP, SimpleValue)
        TEST_BOUNDED(MichaelDequeR_HP_ic, SimpleValue)
        TEST_BOUNDED(MichaelDequeR_HP_michaelAlloc, SimpleValue)
        TEST_BOUNDED(MichaelDequeR_HP_stat, SimpleValue)
        TEST_BOUNDED(MichaelDequeR_PTB, SimpleValue)
        TEST_BOUNDED(MichaelDequeR_PTB_ic, SimpleValue)
        TEST_BOUNDED(MichaelDequeR_PTB_michaelAlloc, SimpleValue)
        TEST_BOUNDED(MichaelDequeR_PTB_stat, SimpleValue)

        TEST_CASE( StdQueue_deque_Spinlock, SimpleValue )
        TEST_CASE( StdQueue_list_Spinlock, SimpleValue )
        TEST_CASE( StdQueue_deque_BoostMutex, SimpleValue )
        TEST_CASE( StdQueue_list_BoostMutex, SimpleValue )
#ifdef UNIT_LOCK_WIN_CS
        TEST_CASE( StdQueue_deque_WinCS, SimpleValue )
        TEST_CASE( StdQueue_list_WinCS, SimpleValue )
#endif

        CPPUNIT_TEST_SUITE(Queue_Push)
            CPPUNIT_TEST(MoirQueue_HP)              ;
            CPPUNIT_TEST(MoirQueue_HP_seqcst)       ;
            CPPUNIT_TEST(MoirQueue_HP_ic)           ;
            CPPUNIT_TEST(MoirQueue_HP_stat)         ;
            CPPUNIT_TEST(MoirQueue_HRC)             ;
            CPPUNIT_TEST(MoirQueue_HRC_seqcst)      ;
            CPPUNIT_TEST(MoirQueue_HRC_ic)          ;
            CPPUNIT_TEST(MoirQueue_HRC_stat)        ;
            CPPUNIT_TEST(MoirQueue_PTB)             ;
            CPPUNIT_TEST(MoirQueue_PTB_seqcst)      ;
            CPPUNIT_TEST(MoirQueue_PTB_ic)          ;
            CPPUNIT_TEST(MoirQueue_PTB_stat)        ;

            CPPUNIT_TEST(MSQueue_HP)                ;
            CPPUNIT_TEST(MSQueue_HP_seqcst)         ;
            CPPUNIT_TEST(MSQueue_HP_ic)             ;
            CPPUNIT_TEST(MSQueue_HP_stat)           ;
            CPPUNIT_TEST(MSQueue_HRC)               ;
            CPPUNIT_TEST(MSQueue_HRC_seqcst)        ;
            CPPUNIT_TEST(MSQueue_HRC_ic)            ;
            CPPUNIT_TEST(MSQueue_HRC_stat)          ;
            CPPUNIT_TEST(MSQueue_PTB)               ;
            CPPUNIT_TEST(MSQueue_PTB_seqcst)        ;
            CPPUNIT_TEST(MSQueue_PTB_ic)            ;
            CPPUNIT_TEST(MSQueue_PTB_stat)          ;

            CPPUNIT_TEST(OptimisticQueue_HP)              ;
            CPPUNIT_TEST(OptimisticQueue_HP_seqcst)       ;
            CPPUNIT_TEST(OptimisticQueue_HP_ic)           ;
            CPPUNIT_TEST(OptimisticQueue_HP_stat)         ;
            CPPUNIT_TEST(OptimisticQueue_PTB)             ;
            CPPUNIT_TEST(OptimisticQueue_PTB_seqcst)      ;
            CPPUNIT_TEST(OptimisticQueue_PTB_ic)          ;
            CPPUNIT_TEST(OptimisticQueue_PTB_stat)        ;

            CPPUNIT_TEST(BasketQueue_HP)                ;
            CPPUNIT_TEST(BasketQueue_HP_seqcst)         ;
            CPPUNIT_TEST(BasketQueue_HP_ic)             ;
            CPPUNIT_TEST(BasketQueue_HP_stat)           ;
            CPPUNIT_TEST(BasketQueue_HRC)               ;
            CPPUNIT_TEST(BasketQueue_HRC_seqcst)        ;
            CPPUNIT_TEST(BasketQueue_HRC_ic)            ;
            CPPUNIT_TEST(BasketQueue_HRC_stat)          ;
            CPPUNIT_TEST(BasketQueue_PTB)               ;
            CPPUNIT_TEST(BasketQueue_PTB_seqcst)        ;
            CPPUNIT_TEST(BasketQueue_PTB_ic)            ;
            CPPUNIT_TEST(BasketQueue_PTB_stat)          ;

            CPPUNIT_TEST(RWQueue_Spin)              ;
            CPPUNIT_TEST(RWQueue_Spin_ic)           ;
            CPPUNIT_TEST(RWQueue_Spin_stat)         ;

            CPPUNIT_TEST(TsigasCycleQueue_dyn)      ;
            CPPUNIT_TEST(TsigasCycleQueue_dyn_ic)   ;

            CPPUNIT_TEST(VyukovMPMCCycleQueue_dyn)  ;
            CPPUNIT_TEST(VyukovMPMCCycleQueue_dyn_ic);

            CPPUNIT_TEST(MichaelDequeL_HP )
            CPPUNIT_TEST(MichaelDequeL_HP_ic )
            CPPUNIT_TEST(MichaelDequeL_HP_michaelAlloc )
            CPPUNIT_TEST(MichaelDequeL_HP_stat )
            CPPUNIT_TEST(MichaelDequeL_PTB )
            CPPUNIT_TEST(MichaelDequeL_PTB_ic )
            CPPUNIT_TEST(MichaelDequeL_PTB_michaelAlloc )
            CPPUNIT_TEST(MichaelDequeL_PTB_stat )

            CPPUNIT_TEST(MichaelDequeR_HP )
            CPPUNIT_TEST(MichaelDequeR_HP_ic )
            CPPUNIT_TEST(MichaelDequeR_HP_michaelAlloc )
            CPPUNIT_TEST(MichaelDequeR_HP_stat )
            CPPUNIT_TEST(MichaelDequeR_PTB )
            CPPUNIT_TEST(MichaelDequeR_PTB_ic )
            CPPUNIT_TEST(MichaelDequeR_PTB_michaelAlloc )
            CPPUNIT_TEST(MichaelDequeR_PTB_stat )

            CPPUNIT_TEST(StdQueue_deque_Spinlock)   ;
            CPPUNIT_TEST(StdQueue_list_Spinlock)    ;
            CPPUNIT_TEST(StdQueue_deque_BoostMutex) ;
            CPPUNIT_TEST(StdQueue_list_BoostMutex)  ;
#ifdef UNIT_LOCK_WIN_CS
            CPPUNIT_TEST(StdQueue_deque_WinCS) ;
            CPPUNIT_TEST(StdQueue_list_WinCS)  ;
#endif
        CPPUNIT_TEST_SUITE_END();
    };

} // namespace queue

CPPUNIT_TEST_SUITE_REGISTRATION(queue::Queue_Push);
