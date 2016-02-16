//$$CDS-header$$

#include "cppunit/thread.h"
#include "queue/queue_type.h"

#include <cds/int_algo.h>

// Bounded queue tests
namespace queue {
#define TEST_CASE( Q, V ) void Q() { test< Types<V>::Q >(); }

    namespace {
        static size_t s_nPassCount = 100  ;
        static size_t s_nQueueSize = 256 ;   // no more than 20 million records

        struct SimpleValue {
            size_t      nNo ;

            SimpleValue() {}
            SimpleValue( size_t n ): nNo(n) {}
            size_t getNo() const { return  nNo; }
        };
    }

    class Queue_bounded_empty_ST: public CppUnitMini::TestCase
    {
        template <class Q>
        void test()
        {
            //const size_t nLog2 = cds::beans::ceil2( s_nQueueSize )   ;
            const size_t nSize = cds::beans::ceil2( s_nQueueSize )   ;
            Q   queue( nSize ) ;

            CPPUNIT_MSG( "  queue.empty(), queue size=" << nSize << ", pass count=" << s_nPassCount ) ;

            for ( size_t nPass = 0; nPass < s_nPassCount; ++nPass ) {
                CPPUNIT_ASSERT_EX( queue.empty(), "start pass=" << nPass )    ;
                for ( size_t i = 0; i < queue.capacity(); ++i ) {
                    CPPUNIT_ASSERT_EX( queue.push(i), "item=" << i << " pass=" << nPass )    ;
                }
                CPPUNIT_ASSERT_EX( !queue.empty(), "pass=" << nPass )   ;

                // try to push to full queue
                CPPUNIT_ASSERT_EX( !queue.push(nSize + 1), "push to full queue" )   ;
                CPPUNIT_ASSERT_EX( !queue.empty(), "check emptiness of full queue" )   ;

                size_t nItem    ;
                for ( size_t i = 0; i < queue.capacity(); ++i ) {
                    CPPUNIT_ASSERT_EX( queue.pop(nItem), "item=" << i << " pass=" << nPass) ;
                    CPPUNIT_ASSERT_EX( nItem == i, "item=" << i << ", popped=" << nItem << " pass=" << nPass)    ;
                }
                CPPUNIT_ASSERT_EX( queue.empty(), "before end pass=" << nPass )    ;

                // Push/pop to offset head/tail position in queue
                CPPUNIT_ASSERT_EX( queue.push(nSize), "head/tail offset" << " pass=" << nPass )  ;
                CPPUNIT_ASSERT_EX( !queue.empty(), "head/tail offset" << " pass=" << nPass )    ;
                CPPUNIT_ASSERT_EX( queue.pop(nItem), "head/tail offset" << " pass=" << nPass)   ;
                CPPUNIT_ASSERT_EX( nItem == nSize, "item=" << nSize << ", popped=" << nItem << " pass=" << nPass)    ;

                CPPUNIT_ASSERT_EX( queue.empty(), "end pass=" << nPass )    ;
            }
        }

        void setUpParams( const CppUnitMini::TestCfg& cfg ) {
            s_nPassCount = cfg.getULong("PassCount", (unsigned long) s_nPassCount ) ;
            s_nQueueSize = cfg.getULong("QueueSize", (unsigned long) s_nQueueSize ) ;
        }


        TEST_CASE( TsigasCycleQueue_dyn, size_t )
        TEST_CASE( TsigasCycleQueue_dyn_ic, size_t )

        TEST_CASE( VyukovMPMCCycleQueue_dyn, size_t )
        TEST_CASE( VyukovMPMCCycleQueue_dyn_ic, size_t )

        CPPUNIT_TEST_SUITE(Queue_bounded_empty_ST)
            CPPUNIT_TEST( TsigasCycleQueue_dyn )
            CPPUNIT_TEST( TsigasCycleQueue_dyn_ic )

            CPPUNIT_TEST( VyukovMPMCCycleQueue_dyn )
            CPPUNIT_TEST( VyukovMPMCCycleQueue_dyn_ic )
        CPPUNIT_TEST_SUITE_END();
    };
}   // namespace queue

CPPUNIT_TEST_SUITE_REGISTRATION(queue::Queue_bounded_empty_ST);
