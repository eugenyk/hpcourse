//$$CDS-header$$

#include "cppunit/thread.h"
#include "queue/queue_type.h"

#include <vector>
#include <boost/type_traits/is_base_of.hpp>

// Multi-threaded queue test for random push/pop operation
namespace queue {

#define TEST_CASE( Q, V ) void Q() { test< Types<V>::Q >(); }

    namespace ns_Queue_Random {
        static size_t s_nThreadCount = 16         ;
        static size_t s_nQueueSize = 10000000     ;

        struct SimpleValue {
            size_t      nNo ;
            size_t      nThread ;

            SimpleValue() {}
            SimpleValue( size_t n ): nNo(n) {}
            size_t getNo() const { return  nNo; }
        };
    }

    using namespace ns_Queue_Random    ;

    class Queue_Random: public CppUnitMini::TestCase
    {
        typedef CppUnitMini::TestCase base_class ;

        template <class QUEUE>
        class Thread: public CppUnitMini::TestThread
        {
            virtual TestThread *    clone()
            {
                return new Thread( *this )  ;
            }
        public:
            QUEUE&              m_Queue ;
            double              m_fTime ;

            size_t  m_nPushCount        ;
            size_t  m_nPopCount         ;
            size_t  m_nEmptyPop         ;

            size_t  m_nUndefWriter      ;
            size_t  m_nRepeatValue      ;
            size_t  m_nPushError        ;    // push error count

            std::vector<size_t> m_arrLastRead ;
            std::vector<size_t> m_arrPopCountPerThread ;

        public:
            Thread( CppUnitMini::ThreadPool& pool, QUEUE& q )
                : CppUnitMini::TestThread( pool )
                , m_Queue( q )
            {}
            Thread( Thread& src )
                : CppUnitMini::TestThread( src )
                , m_Queue( src.m_Queue )
            {}

            Queue_Random&  getTest()
            {
                return reinterpret_cast<Queue_Random&>( m_Pool.m_Test )   ;
            }

            virtual void init()
            {
                cds::threading::Manager::attachThread()     ;
                m_nPushCount =
                    m_nPopCount =
                    m_nEmptyPop =
                    m_nUndefWriter =
                    m_nRepeatValue =
                    m_nPushError = 0    ;

                m_arrLastRead.resize( s_nThreadCount, 0 )   ;
                m_arrPopCountPerThread.resize( s_nThreadCount, 0 ) ;
            }
            virtual void fini()
            {
                cds::threading::Manager::detachThread()   ;
            }

            virtual void test()
            {
                size_t const nThreadCount = s_nThreadCount    ;
                size_t const nTotalPush = getTest().m_nThreadPushCount  ;

                SimpleValue node    ;

                m_fTime = m_Timer.duration()        ;

                bool bNextPop = false ;
                while ( m_nPushCount < nTotalPush ) {
                    if ( !bNextPop && (rand() & 3) != 3 ) {
                        // push
                        node.nThread = m_nThreadNo        ;
                        node.nNo = ++m_nPushCount        ;
                        if ( !m_Queue.push( node )) {
                            ++m_nPushError    ;
                            --m_nPushCount    ;
                        }

                    }
                    else {
                        // pop
                        pop( nThreadCount )        ;
                        bNextPop = false        ;
                    }
                }

                size_t nPopLoop = 0    ;
                while ( !m_Queue.empty() && nPopLoop < 1000000 ) {
                    if ( pop( nThreadCount ) )
                        nPopLoop = 0    ;
                    else
                        ++nPopLoop        ;
                }


                m_fTime = m_Timer.duration() - m_fTime  ;
            }

            bool pop( size_t nThreadCount )
            {
                SimpleValue node ;
                node.nThread = -1    ;
                node.nNo = -1        ;
                if ( m_Queue.pop( node )) {
                    ++m_nPopCount    ;
                    if ( node.nThread < nThreadCount ) {
                        m_arrPopCountPerThread[ node.nThread ] += 1    ;
                        if ( m_arrLastRead[ node.nThread ] < node.nNo ) {
                            m_arrLastRead[ node.nThread ] = node.nNo    ;
                        }
                        else
                            ++m_nRepeatValue    ;

                        //if ( node.nNo < m_Test.m_nPushCount )
                        //    m_Test.m_pRead[ node.nWriter ][ node.nNo ] = node.nNo    ;
                    }
                    else {
                        ++m_nUndefWriter        ;
                    }
                }
                else {
                    ++m_nEmptyPop    ;
                    return false    ;
                }
                return true ;
            }
        };

    protected:
        size_t  m_nThreadPushCount  ;

    protected:
        template <class QUEUE>
        void analyze( CppUnitMini::ThreadPool& pool, QUEUE& testQueue  )
        {
            CPPUNIT_CHECK( testQueue.empty() ) ;

            std::vector< size_t > arrPushCount  ;
            arrPushCount.resize( s_nThreadCount, 0 ) ;

            size_t nPushTotal = 0   ;
            size_t nPopTotal  = 0   ;
            double fTime = 0    ;
            size_t nPushError = 0   ;

            for ( CppUnitMini::ThreadPool::iterator it = pool.begin(); it != pool.end(); ++it ) {
                Thread<QUEUE> * pThread = static_cast<Thread<QUEUE> *>( *it ) ;
                CPPUNIT_CHECK( pThread->m_nUndefWriter == 0 )  ;
                CPPUNIT_CHECK_EX( pThread->m_nRepeatValue == 0, "nRepeatValue=" << pThread->m_nRepeatValue )  ;
                if ( !boost::is_base_of<cds::bounded_container, QUEUE>::value ) {
                    CPPUNIT_CHECK( pThread->m_nPushError == 0 )    ;
                }
                else
                    nPushError += pThread->m_nPushError ;

                arrPushCount[ pThread->m_nThreadNo ] += pThread->m_nPushCount   ;

                nPushTotal += pThread->m_nPushCount ;
                nPopTotal += pThread->m_nPopCount   ;
                fTime += pThread->m_fTime           ;
            }

            CPPUNIT_MSG( "     Duration=" << (fTime /= s_nThreadCount) )   ;
            if ( boost::is_base_of<cds::bounded_container, QUEUE>::value ) {
                CPPUNIT_MSG( "         push error (when queue is full)=" << nPushError )    ;
            }

            size_t nTotalItems = m_nThreadPushCount * s_nThreadCount    ;

            CPPUNIT_CHECK_EX( nPushTotal == nTotalItems, "nPushTotal=" << nPushTotal << ", nTotalItems=" << nTotalItems ) ;
            CPPUNIT_CHECK_EX( nPopTotal == nTotalItems, "nPopTotal=" << nPopTotal << ", nTotalItems=" << nTotalItems ) ;

            for ( size_t i = 0; i < s_nThreadCount; ++i )
                CPPUNIT_CHECK( arrPushCount[i] == m_nThreadPushCount ) ;
        }

        template <class QUEUE>
        void test()
        {
            CPPUNIT_MSG( "Random push/pop test\n    thread count=" << s_nThreadCount << ", push count=" << s_nQueueSize << " ..." )   ;

            m_nThreadPushCount = s_nQueueSize / s_nThreadCount  ;

            QUEUE testQueue ;
            CppUnitMini::ThreadPool pool( *this )   ;
            pool.add( new Thread<QUEUE>( pool, testQueue ), s_nThreadCount )       ;

            pool.run()  ;

            analyze( pool, testQueue )      ;
            CPPUNIT_MSG( testQueue.statistics() )   ;
        }

        void setUpParams( const CppUnitMini::TestCfg& cfg ) {
            s_nThreadCount = cfg.getULong("ThreadCount", 8 ) ;
            s_nQueueSize = cfg.getULong("QueueSize", 20000000 );
        }

    protected:
        TEST_CASE( MoirQueue_HP, SimpleValue )
        TEST_CASE( MoirQueue_HP_michaelAlloc, SimpleValue )
        TEST_CASE( MoirQueue_HP_seqcst, SimpleValue )
        TEST_CASE( MoirQueue_HP_ic, SimpleValue )
        TEST_CASE( MoirQueue_HP_stat, SimpleValue )
        TEST_CASE( MoirQueue_HRC, SimpleValue )
        TEST_CASE( MoirQueue_HRC_michaelAlloc, SimpleValue )
        TEST_CASE( MoirQueue_HRC_seqcst, SimpleValue )
        TEST_CASE( MoirQueue_HRC_ic, SimpleValue )
        TEST_CASE( MoirQueue_HRC_stat, SimpleValue )
        TEST_CASE( MoirQueue_PTB, SimpleValue )
        TEST_CASE( MoirQueue_PTB_michaelAlloc, SimpleValue )
        TEST_CASE( MoirQueue_PTB_seqcst, SimpleValue )
        TEST_CASE( MoirQueue_PTB_ic, SimpleValue )
        TEST_CASE( MoirQueue_PTB_stat, SimpleValue )

        TEST_CASE( MSQueue_HP, SimpleValue  )
        TEST_CASE( MSQueue_HP_michaelAlloc, SimpleValue  )
        TEST_CASE( MSQueue_HP_seqcst, SimpleValue  )
        TEST_CASE( MSQueue_HP_ic, SimpleValue  )
        TEST_CASE( MSQueue_HP_stat, SimpleValue  )
        TEST_CASE( MSQueue_HRC, SimpleValue )
        TEST_CASE( MSQueue_HRC_michaelAlloc, SimpleValue )
        TEST_CASE( MSQueue_HRC_seqcst, SimpleValue )
        TEST_CASE( MSQueue_HRC_ic, SimpleValue )
        TEST_CASE( MSQueue_HRC_stat, SimpleValue )
        TEST_CASE( MSQueue_PTB, SimpleValue )
        TEST_CASE( MSQueue_PTB_michaelAlloc, SimpleValue )
        TEST_CASE( MSQueue_PTB_seqcst, SimpleValue )
        TEST_CASE( MSQueue_PTB_ic, SimpleValue )
        TEST_CASE( MSQueue_PTB_stat, SimpleValue )

        TEST_CASE(OptimisticQueue_HP, SimpleValue )
        TEST_CASE(OptimisticQueue_HP_michaelAlloc, SimpleValue )
        TEST_CASE(OptimisticQueue_HP_seqcst, SimpleValue )
        TEST_CASE(OptimisticQueue_HP_ic, SimpleValue )
        TEST_CASE(OptimisticQueue_HP_stat, SimpleValue )
        TEST_CASE(OptimisticQueue_PTB, SimpleValue )
        TEST_CASE(OptimisticQueue_PTB_michaelAlloc, SimpleValue )
        TEST_CASE(OptimisticQueue_PTB_seqcst, SimpleValue )
        TEST_CASE(OptimisticQueue_PTB_ic, SimpleValue )
        TEST_CASE(OptimisticQueue_PTB_stat, SimpleValue )

        TEST_CASE( BasketQueue_HP, SimpleValue  )
        TEST_CASE( BasketQueue_HP_michaelAlloc, SimpleValue  )
        TEST_CASE( BasketQueue_HP_seqcst, SimpleValue  )
        TEST_CASE( BasketQueue_HP_ic, SimpleValue  )
        TEST_CASE( BasketQueue_HP_stat, SimpleValue  )
        TEST_CASE( BasketQueue_HRC, SimpleValue )
        TEST_CASE( BasketQueue_HRC_michaelAlloc, SimpleValue )
        TEST_CASE( BasketQueue_HRC_seqcst, SimpleValue )
        TEST_CASE( BasketQueue_HRC_ic, SimpleValue )
        TEST_CASE( BasketQueue_HRC_stat, SimpleValue )
        TEST_CASE( BasketQueue_PTB, SimpleValue )
        TEST_CASE( BasketQueue_PTB_michaelAlloc, SimpleValue )
        TEST_CASE( BasketQueue_PTB_seqcst, SimpleValue )
        TEST_CASE( BasketQueue_PTB_ic, SimpleValue )
        TEST_CASE( BasketQueue_PTB_stat, SimpleValue )

        TEST_CASE( RWQueue_Spin, SimpleValue )
        TEST_CASE( RWQueue_Spin_ic, SimpleValue )
        TEST_CASE( RWQueue_Spin_stat, SimpleValue )

        TEST_CASE(TsigasCycleQueue_dyn, SimpleValue)
        TEST_CASE(TsigasCycleQueue_dyn_michaelAlloc, SimpleValue)
        TEST_CASE(TsigasCycleQueue_dyn_ic, SimpleValue)

        TEST_CASE(VyukovMPMCCycleQueue_dyn, SimpleValue)
        TEST_CASE(VyukovMPMCCycleQueue_dyn_michaelAlloc, SimpleValue)
        TEST_CASE(VyukovMPMCCycleQueue_dyn_ic, SimpleValue)

        TEST_CASE(MichaelDequeL_HP, SimpleValue)
        TEST_CASE(MichaelDequeL_HP_ic, SimpleValue)
        TEST_CASE(MichaelDequeL_HP_michaelAlloc, SimpleValue)
        TEST_CASE(MichaelDequeL_HP_stat, SimpleValue)
        TEST_CASE(MichaelDequeL_PTB, SimpleValue)
        TEST_CASE(MichaelDequeL_PTB_ic, SimpleValue)
        TEST_CASE(MichaelDequeL_PTB_michaelAlloc, SimpleValue)
        TEST_CASE(MichaelDequeL_PTB_stat, SimpleValue)

        TEST_CASE(MichaelDequeR_HP, SimpleValue)
        TEST_CASE(MichaelDequeR_HP_ic, SimpleValue)
        TEST_CASE(MichaelDequeR_HP_michaelAlloc, SimpleValue)
        TEST_CASE(MichaelDequeR_HP_stat, SimpleValue)
        TEST_CASE(MichaelDequeR_PTB, SimpleValue)
        TEST_CASE(MichaelDequeR_PTB_ic, SimpleValue)
        TEST_CASE(MichaelDequeR_PTB_michaelAlloc, SimpleValue)
        TEST_CASE(MichaelDequeR_PTB_stat, SimpleValue)

        TEST_CASE( StdQueue_deque_Spinlock, SimpleValue )
        TEST_CASE( StdQueue_list_Spinlock, SimpleValue )
        TEST_CASE( StdQueue_deque_BoostMutex, SimpleValue )
        TEST_CASE( StdQueue_list_BoostMutex, SimpleValue )
#ifdef UNIT_LOCK_WIN_CS
        TEST_CASE( StdQueue_deque_WinCS, SimpleValue )
        TEST_CASE( StdQueue_list_WinCS, SimpleValue )
#endif

        CPPUNIT_TEST_SUITE(Queue_Random)
            CPPUNIT_TEST(BasketQueue_HP)                ;
            CPPUNIT_TEST(BasketQueue_HP_michaelAlloc)   ;
            CPPUNIT_TEST(BasketQueue_HP_seqcst)         ;
            CPPUNIT_TEST(BasketQueue_HP_ic)             ;
            CPPUNIT_TEST(BasketQueue_HP_stat)           ;
            CPPUNIT_TEST(BasketQueue_HRC)               ;
            CPPUNIT_TEST(BasketQueue_HRC_michaelAlloc)  ;
            CPPUNIT_TEST(BasketQueue_HRC_seqcst)        ;
            CPPUNIT_TEST(BasketQueue_HRC_ic)            ;
            CPPUNIT_TEST(BasketQueue_HRC_stat)          ;
            CPPUNIT_TEST(BasketQueue_PTB)               ;
            CPPUNIT_TEST(BasketQueue_PTB_michaelAlloc)  ;
            CPPUNIT_TEST(BasketQueue_PTB_seqcst)        ;
            CPPUNIT_TEST(BasketQueue_PTB_ic)            ;
            CPPUNIT_TEST(BasketQueue_PTB_stat)          ;

            CPPUNIT_TEST(MoirQueue_HP)              ;
            CPPUNIT_TEST(MoirQueue_HP_michaelAlloc) ;
            CPPUNIT_TEST(MoirQueue_HP_seqcst)       ;
            CPPUNIT_TEST(MoirQueue_HP_ic)           ;
            CPPUNIT_TEST(MoirQueue_HP_stat)         ;
            CPPUNIT_TEST(MoirQueue_HRC)             ;
            CPPUNIT_TEST(MoirQueue_HRC_michaelAlloc);
            CPPUNIT_TEST(MoirQueue_HRC_seqcst)      ;
            CPPUNIT_TEST(MoirQueue_HRC_ic)          ;
            CPPUNIT_TEST(MoirQueue_HRC_stat)        ;
            CPPUNIT_TEST(MoirQueue_PTB)             ;
            CPPUNIT_TEST(MoirQueue_PTB_michaelAlloc);
            CPPUNIT_TEST(MoirQueue_PTB_seqcst)      ;
            CPPUNIT_TEST(MoirQueue_PTB_ic)          ;
            CPPUNIT_TEST(MoirQueue_PTB_stat)        ;

            CPPUNIT_TEST(MSQueue_HP)                ;
            CPPUNIT_TEST(MSQueue_HP_michaelAlloc)   ;
            CPPUNIT_TEST(MSQueue_HP_seqcst)         ;
            CPPUNIT_TEST(MSQueue_HP_ic)             ;
            CPPUNIT_TEST(MSQueue_HP_stat)           ;
            CPPUNIT_TEST(MSQueue_HRC)               ;
            CPPUNIT_TEST(MSQueue_HRC_michaelAlloc)  ;
            CPPUNIT_TEST(MSQueue_HRC_seqcst)        ;
            CPPUNIT_TEST(MSQueue_HRC_ic)            ;
            CPPUNIT_TEST(MSQueue_HRC_stat)          ;
            CPPUNIT_TEST(MSQueue_PTB)               ;
            CPPUNIT_TEST(MSQueue_PTB_michaelAlloc)  ;
            CPPUNIT_TEST(MSQueue_PTB_seqcst)        ;
            CPPUNIT_TEST(MSQueue_PTB_ic)            ;
            CPPUNIT_TEST(MSQueue_PTB_stat)          ;

            CPPUNIT_TEST(OptimisticQueue_HP)              ;
            CPPUNIT_TEST(OptimisticQueue_HP_michaelAlloc) ;
            CPPUNIT_TEST(OptimisticQueue_HP_seqcst)       ;
            CPPUNIT_TEST(OptimisticQueue_HP_ic)           ;
            CPPUNIT_TEST(OptimisticQueue_HP_stat)         ;
            CPPUNIT_TEST(OptimisticQueue_PTB)             ;
            CPPUNIT_TEST(OptimisticQueue_PTB_michaelAlloc);
            CPPUNIT_TEST(OptimisticQueue_PTB_seqcst)      ;
            CPPUNIT_TEST(OptimisticQueue_PTB_ic)          ;
            CPPUNIT_TEST(OptimisticQueue_PTB_stat)        ;

            CPPUNIT_TEST(TsigasCycleQueue_dyn)      ;
            CPPUNIT_TEST(TsigasCycleQueue_dyn_michaelAlloc)     ;
            CPPUNIT_TEST(TsigasCycleQueue_dyn_ic)   ;

            CPPUNIT_TEST(VyukovMPMCCycleQueue_dyn)  ;
            CPPUNIT_TEST(VyukovMPMCCycleQueue_dyn_michaelAlloc) ;
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

            CPPUNIT_TEST(RWQueue_Spin)              ;
            CPPUNIT_TEST(RWQueue_Spin_ic)           ;
            CPPUNIT_TEST(RWQueue_Spin_stat)         ;

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

CPPUNIT_TEST_SUITE_REGISTRATION(queue::Queue_Random);
