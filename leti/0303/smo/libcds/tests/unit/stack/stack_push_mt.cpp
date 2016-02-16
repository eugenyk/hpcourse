//$$CDS-header$$

#include "cppunit/thread.h"
#include "stack/stack_type.h"
#include "print_deque_stat.h"

// Multi-threaded stack test for push operation
namespace stack {

#define TEST_CASE( Q, V ) void Q() { test_unbounded< Types<V>::Q >(); }
#define TEST_BOUNDED( Q, V ) void Q() { test_bounded< Types<V>::Q >(); }

    namespace {
        static size_t s_nThreadCount = 8  ;
        static size_t s_nStackSize = 10000000 ;

        struct SimpleValue {
            size_t      nNo ;
            size_t      nThread ;

            SimpleValue() {}
            SimpleValue( size_t n ): nNo(n) {}
            size_t getNo() const { return  nNo; }
        };
    }

    class Stack_Push: public CppUnitMini::TestCase
    {
        template <class STACK>
        class Thread: public CppUnitMini::TestThread
        {
            virtual TestThread *    clone()
            {
                return new Thread( *this )  ;
            }
        public:
            STACK&              m_Stack         ;
            double              m_fTime         ;
            size_t              m_nStartItem    ;
            size_t              m_nEndItem      ;
            size_t              m_nPushError    ;

        public:
            Thread( CppUnitMini::ThreadPool& pool, STACK& s )
                : CppUnitMini::TestThread( pool )
                , m_Stack( s )
            {}
            Thread( Thread& src )
                : CppUnitMini::TestThread( src )
                , m_Stack( src.m_Stack )
            {}

            Stack_Push&  getTest()
            {
                return reinterpret_cast<Stack_Push&>( m_Pool.m_Test )   ;
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
                SimpleValue v   ;
                v.nThread = m_nThreadNo ;
                for ( v.nNo = m_nStartItem; v.nNo < m_nEndItem; ++v.nNo ) {
                    if ( !m_Stack.push( v ))
                        ++m_nPushError ;
                }

                m_fTime = m_Timer.duration() - m_fTime  ;
            }
        };

    protected:
        void setUpParams( const CppUnitMini::TestCfg& cfg ) {
            s_nThreadCount = cfg.getULong("ThreadCount", 8 ) ;
            s_nStackSize = cfg.getULong("StackSize", 10000000 );
        }

        template <class STACK>
        void analyze( CppUnitMini::ThreadPool& pool, STACK& testStack  )
        {
            size_t nThreadItems = s_nStackSize / s_nThreadCount ;
            std::vector<size_t> aThread ;
            aThread.resize(s_nThreadCount)  ;

            double fTime = 0    ;
            for ( CppUnitMini::ThreadPool::iterator it = pool.begin(); it != pool.end(); ++it ) {
                Thread<STACK> * pThread = reinterpret_cast<Thread<STACK> *>(*it)  ;
                fTime += pThread->m_fTime   ;
                if ( pThread->m_nPushError != 0 )
                    CPPUNIT_MSG("     ERROR: thread push error count=" << pThread->m_nPushError ) ;
                aThread[ pThread->m_nThreadNo] = pThread->m_nEndItem - 1    ;
            }
            CPPUNIT_MSG( "     Duration=" << (fTime / s_nThreadCount) )   ;
            CPPUNIT_ASSERT( !testStack.empty() )

            size_t * arr = new size_t[ s_nStackSize ]           ;
            memset(arr, 0, sizeof(arr[0]) * s_nStackSize )      ;

            cds::OS::Timer      timer   ;
            CPPUNIT_MSG( "   Pop (single-threaded)..." )    ;
            size_t nPopped = 0      ;
            SimpleValue val         ;
            while ( testStack.pop( val )) {
                nPopped++   ;
                ++arr[ val.getNo() ]    ;
                CPPUNIT_ASSERT( val.nThread < s_nThreadCount)   ;
                CPPUNIT_ASSERT( aThread[val.nThread] == val.nNo )   ;
                aThread[val.nThread]--  ;
            }
            CPPUNIT_MSG( "     Duration=" << timer.duration() )     ;

            size_t nTotalItems = nThreadItems * s_nThreadCount      ;
            size_t nError = 0   ;
            for ( size_t i = 0; i < nTotalItems; ++i ) {
                if ( arr[i] != 1 ) {
                    CPPUNIT_MSG( "   ERROR: Item " << i << " has not been pushed" ) ;
                    CPPUNIT_ASSERT( ++nError > 10 ) ;
                }
            }

            delete [] arr ;
        }

        // Unbounded stack test
        template <class STACK>
        void test_unbounded()
        {
            STACK testStack     ;
            test( testStack )   ;
        }

        // Bounded stack test
        template <class STACK>
        void test_bounded()
        {
            STACK testStack( s_nStackSize )  ;
            test( testStack )   ;
        }

        template <class STACK>
        void test( STACK& testStack )
        {
            CppUnitMini::ThreadPool pool( *this )   ;
            pool.add( new Thread<STACK>( pool, testStack ), s_nThreadCount )       ;

            size_t nStart = 0   ;
            size_t nThreadItemCount = s_nStackSize / s_nThreadCount ;
            for ( CppUnitMini::ThreadPool::iterator it = pool.begin(); it != pool.end(); ++it ) {
                Thread<STACK> * pThread = reinterpret_cast<Thread<STACK> *>(*it)  ;
                pThread->m_nStartItem = nStart  ;
                nStart += nThreadItemCount      ;
                pThread->m_nEndItem = nStart    ;
            }

            CPPUNIT_MSG( "   Push test, thread count=" << s_nThreadCount
                << " items=" << (nThreadItemCount * s_nThreadCount)
                << "...")   ;
            pool.run()  ;

            analyze( pool, testStack )     ;
            CPPUNIT_MSG( testStack.statistics() )   ;
        }

    protected:
        TEST_CASE( Treiber_HP,          SimpleValue )
        TEST_CASE( Treiber_HP_yield,    SimpleValue )
        TEST_CASE( Treiber_HP_pause,    SimpleValue )
        TEST_CASE( Treiber_HP_exp,      SimpleValue )
        TEST_CASE( Treiber_HP_relaxed,          SimpleValue )
        TEST_CASE( Treiber_HP_yield_relaxed,    SimpleValue )
        TEST_CASE( Treiber_HP_pause_relaxed,    SimpleValue )
        TEST_CASE( Treiber_HP_exp_relaxed,      SimpleValue )
        TEST_CASE( Treiber_HP_stat,             SimpleValue )

        TEST_CASE( Treiber_HRC,         SimpleValue )
        TEST_CASE( Treiber_HRC_yield,   SimpleValue )
        TEST_CASE( Treiber_HRC_pause,   SimpleValue )
        TEST_CASE( Treiber_HRC_exp,     SimpleValue )
        TEST_CASE( Treiber_HRC_relaxed,         SimpleValue )
        TEST_CASE( Treiber_HRC_yield_relaxed,   SimpleValue )
        TEST_CASE( Treiber_HRC_pause_relaxed,   SimpleValue )
        TEST_CASE( Treiber_HRC_exp_relaxed,     SimpleValue )
        TEST_CASE( Treiber_HRC_stat,            SimpleValue )

        TEST_CASE( Treiber_PTB,         SimpleValue )
        TEST_CASE( Treiber_PTB_yield,   SimpleValue )
        TEST_CASE( Treiber_PTB_pause,   SimpleValue )
        TEST_CASE( Treiber_PTB_exp,     SimpleValue )
        TEST_CASE( Treiber_PTB_relaxed,         SimpleValue )
        TEST_CASE( Treiber_PTB_yield_relaxed,   SimpleValue )
        TEST_CASE( Treiber_PTB_pause_relaxed,   SimpleValue )
        TEST_CASE( Treiber_PTB_exp_relaxed,     SimpleValue )
        TEST_CASE( Treiber_PTB_stat,            SimpleValue )

        TEST_BOUNDED( MichaelDequeL_HP, SimpleValue )
        TEST_BOUNDED( MichaelDequeL_HP_seqcst, SimpleValue )
        TEST_BOUNDED( MichaelDequeL_HP_ic, SimpleValue )
        TEST_BOUNDED( MichaelDequeL_HP_exp, SimpleValue )
        TEST_BOUNDED( MichaelDequeL_HP_yield, SimpleValue )
        TEST_BOUNDED( MichaelDequeL_HP_stat, SimpleValue )

        TEST_BOUNDED( MichaelDequeL_PTB, SimpleValue )
        TEST_BOUNDED( MichaelDequeL_PTB_seqcst, SimpleValue )
        TEST_BOUNDED( MichaelDequeL_PTB_ic, SimpleValue )
        TEST_BOUNDED( MichaelDequeL_PTB_exp, SimpleValue )
        TEST_BOUNDED( MichaelDequeL_PTB_yield, SimpleValue )
        TEST_BOUNDED( MichaelDequeL_PTB_stat, SimpleValue )

        TEST_BOUNDED( MichaelDequeR_HP, SimpleValue )
        TEST_BOUNDED( MichaelDequeR_HP_seqcst, SimpleValue )
        TEST_BOUNDED( MichaelDequeR_HP_ic, SimpleValue )
        TEST_BOUNDED( MichaelDequeR_HP_exp, SimpleValue )
        TEST_BOUNDED( MichaelDequeR_HP_yield, SimpleValue )
        TEST_BOUNDED( MichaelDequeR_HP_stat, SimpleValue )

        TEST_BOUNDED( MichaelDequeR_PTB, SimpleValue )
        TEST_BOUNDED( MichaelDequeR_PTB_seqcst, SimpleValue )
        TEST_BOUNDED( MichaelDequeR_PTB_ic, SimpleValue )
        TEST_BOUNDED( MichaelDequeR_PTB_exp, SimpleValue )
        TEST_BOUNDED( MichaelDequeR_PTB_yield, SimpleValue )
        TEST_BOUNDED( MichaelDequeR_PTB_stat, SimpleValue )

        CPPUNIT_TEST_SUITE(Stack_Push)
            CPPUNIT_TEST( Treiber_HP        )
            CPPUNIT_TEST( Treiber_HP_relaxed)
            CPPUNIT_TEST( Treiber_HP_yield  )
            CPPUNIT_TEST( Treiber_HP_yield_relaxed  )
            CPPUNIT_TEST( Treiber_HP_pause  )
            CPPUNIT_TEST( Treiber_HP_pause_relaxed  )
            CPPUNIT_TEST( Treiber_HP_exp    )
            CPPUNIT_TEST( Treiber_HP_exp_relaxed    )
            CPPUNIT_TEST( Treiber_HP_stat  )

            CPPUNIT_TEST( Treiber_HRC       )
            CPPUNIT_TEST( Treiber_HRC_relaxed       )
            CPPUNIT_TEST( Treiber_HRC_yield )
            CPPUNIT_TEST( Treiber_HRC_yield_relaxed )
            CPPUNIT_TEST( Treiber_HRC_pause )
            CPPUNIT_TEST( Treiber_HRC_pause_relaxed )
            CPPUNIT_TEST( Treiber_HRC_exp   )
            CPPUNIT_TEST( Treiber_HRC_exp_relaxed   )
            CPPUNIT_TEST( Treiber_HRC_stat )

            CPPUNIT_TEST( Treiber_PTB       )
            CPPUNIT_TEST( Treiber_PTB_relaxed       )
            CPPUNIT_TEST( Treiber_PTB_yield )
            CPPUNIT_TEST( Treiber_PTB_yield_relaxed )
            CPPUNIT_TEST( Treiber_PTB_pause )
            CPPUNIT_TEST( Treiber_PTB_pause_relaxed )
            CPPUNIT_TEST( Treiber_PTB_exp   )
            CPPUNIT_TEST( Treiber_PTB_exp_relaxed   )
            CPPUNIT_TEST( Treiber_PTB_stat )

            CPPUNIT_TEST( MichaelDequeL_HP )
            CPPUNIT_TEST( MichaelDequeL_HP_seqcst )
            CPPUNIT_TEST( MichaelDequeL_HP_ic )
            CPPUNIT_TEST( MichaelDequeL_HP_exp )
            CPPUNIT_TEST( MichaelDequeL_HP_yield )
            CPPUNIT_TEST( MichaelDequeL_HP_stat )

            CPPUNIT_TEST( MichaelDequeL_PTB )
            CPPUNIT_TEST( MichaelDequeL_PTB_seqcst )
            CPPUNIT_TEST( MichaelDequeL_PTB_ic )
            CPPUNIT_TEST( MichaelDequeL_PTB_exp )
            CPPUNIT_TEST( MichaelDequeL_PTB_yield )
            CPPUNIT_TEST( MichaelDequeL_PTB_stat )

            CPPUNIT_TEST( MichaelDequeR_HP )
            CPPUNIT_TEST( MichaelDequeR_HP_seqcst )
            CPPUNIT_TEST( MichaelDequeR_HP_ic )
            CPPUNIT_TEST( MichaelDequeR_HP_exp )
            CPPUNIT_TEST( MichaelDequeR_HP_yield )
            CPPUNIT_TEST( MichaelDequeR_HP_stat )

            CPPUNIT_TEST( MichaelDequeR_PTB )
            CPPUNIT_TEST( MichaelDequeR_PTB_seqcst )
            CPPUNIT_TEST( MichaelDequeR_PTB_ic )
            CPPUNIT_TEST( MichaelDequeR_PTB_exp )
            CPPUNIT_TEST( MichaelDequeR_PTB_yield )
            CPPUNIT_TEST( MichaelDequeR_PTB_stat )

        CPPUNIT_TEST_SUITE_END();
    };

} // namespace stack

CPPUNIT_TEST_SUITE_REGISTRATION(stack::Stack_Push);
