//$$CDS-header$$

#include "cppunit/cppunit_proxy.h"
#include <cds/intrusive/treiber_stack.h>

namespace stack {

    namespace ci = cds::intrusive   ;

    class TreiberIntrusiveTestHeader: public CppUnitMini::TestCase
    {
    public:
        template <typename GC>
        struct base_hook_item: public ci::single_link::node< GC >
        {
            int nVal    ;
            int nDisposeCount ;

            base_hook_item()
                : nDisposeCount(0)
            {}
        };

        template <typename GC>
        struct member_hook_item
        {
            int nVal    ;
            int nDisposeCount ;
            ci::single_link::node< GC > hMember ;

            member_hook_item()
                : nDisposeCount(0)
            {}
        };

        struct faked_disposer
        {
            template <typename T>
            void operator ()( T * p )
            {
                ++p->nDisposeCount  ;
            }
        };

        template <class STACK>
        void test()
        {
            typedef typename STACK::value_type  value_type  ;
            STACK stack ;

            CPPUNIT_ASSERT( stack.empty() ) ;

            value_type v1, v2, v3   ;
            v1.nVal = 1 ;
            v2.nVal = 2 ;
            v3.nVal = 3 ;
            CPPUNIT_ASSERT( stack.push(v1))  ;
            CPPUNIT_ASSERT( !stack.empty() ) ;
            CPPUNIT_ASSERT( stack.push(v2))  ;
            CPPUNIT_ASSERT( !stack.empty() ) ;
            CPPUNIT_ASSERT( stack.push(v3))  ;
            CPPUNIT_ASSERT( !stack.empty() ) ;

            value_type * pv ;
            pv = stack.pop()  ;
            CPPUNIT_ASSERT( pv != NULL )    ;
            CPPUNIT_ASSERT( pv->nVal == 3 ) ;
            CPPUNIT_ASSERT( !stack.empty() ) ;
            pv = stack.pop()  ;
            CPPUNIT_ASSERT( pv != NULL )    ;
            CPPUNIT_ASSERT( pv->nVal == 2 ) ;
            CPPUNIT_ASSERT( !stack.empty() ) ;
            pv = stack.pop()  ;
            CPPUNIT_ASSERT( pv != NULL )    ;
            CPPUNIT_ASSERT( pv->nVal == 1 ) ;
            CPPUNIT_ASSERT( stack.empty() ) ;
            pv = stack.pop()  ;
            CPPUNIT_ASSERT( pv == NULL )    ;

            CPPUNIT_ASSERT( v1.nDisposeCount == 0 ) ;
            CPPUNIT_ASSERT( v2.nDisposeCount == 0 ) ;
            CPPUNIT_ASSERT( v3.nDisposeCount == 0 ) ;

            stack.push(v1)  ;
            stack.push(v2)  ;
            stack.push(v3)  ;

            stack.clear()   ;
            CPPUNIT_ASSERT( stack.empty() ) ;

            STACK::gc::scan()   ;
            if ( !std::is_same<typename STACK::disposer, ci::opt::v::empty_disposer>::value ) {
                CPPUNIT_ASSERT( v1.nDisposeCount == 1 ) ;
                CPPUNIT_ASSERT( v2.nDisposeCount == 1 ) ;
                CPPUNIT_ASSERT( v3.nDisposeCount == 1 ) ;
            }
        }

        void test_Treiber_HP_default();
        void test_Treiber_HP_base();
        void test_Treiber_HP_base_disposer();
        void test_Treiber_HP_member();
        void test_Treiber_HP_member_disposer();
        void test_Treiber_HRC_base();
        void test_Treiber_HRC_base_disposer();
        void test_Treiber_PTB_base();
        void test_Treiber_PTB_base_disposer();
        void test_Treiber_PTB_member();
        void test_Treiber_PTB_member_disposer();

        void test_Treiber_HP_default_relaxed();
        void test_Treiber_HP_base_relaxed();
        void test_Treiber_HP_base_disposer_relaxed();
        void test_Treiber_HP_member_relaxed();
        void test_Treiber_HP_member_disposer_relaxed();
        void test_Treiber_HRC_base_relaxed();
        void test_Treiber_HRC_base_disposer_relaxed();
        void test_Treiber_PTB_base_relaxed();
        void test_Treiber_PTB_base_disposer_relaxed();
        void test_Treiber_PTB_member_relaxed();
        void test_Treiber_PTB_member_disposer_relaxed();

        CPPUNIT_TEST_SUITE(TreiberIntrusiveTestHeader)
            CPPUNIT_TEST(test_Treiber_HP_default)
            CPPUNIT_TEST(test_Treiber_HP_default_relaxed)
            CPPUNIT_TEST(test_Treiber_HP_base)
            CPPUNIT_TEST(test_Treiber_HP_base_relaxed)
            CPPUNIT_TEST(test_Treiber_HP_base_disposer)
            CPPUNIT_TEST(test_Treiber_HP_base_disposer_relaxed)
            CPPUNIT_TEST(test_Treiber_HP_member)
            CPPUNIT_TEST(test_Treiber_HP_member_relaxed)
            CPPUNIT_TEST(test_Treiber_HP_member_disposer)
            CPPUNIT_TEST(test_Treiber_HP_member_disposer_relaxed)
            CPPUNIT_TEST(test_Treiber_HRC_base)
            CPPUNIT_TEST(test_Treiber_HRC_base_relaxed)
            CPPUNIT_TEST(test_Treiber_HRC_base_disposer)
            CPPUNIT_TEST(test_Treiber_HRC_base_disposer_relaxed)
            CPPUNIT_TEST(test_Treiber_PTB_base)
            CPPUNIT_TEST(test_Treiber_PTB_base_relaxed)
            CPPUNIT_TEST(test_Treiber_PTB_base_disposer)
            CPPUNIT_TEST(test_Treiber_PTB_base_disposer_relaxed)
            CPPUNIT_TEST(test_Treiber_PTB_member)
            CPPUNIT_TEST(test_Treiber_PTB_member_relaxed)
            CPPUNIT_TEST(test_Treiber_PTB_member_disposer)
            CPPUNIT_TEST(test_Treiber_PTB_member_disposer_relaxed)

        CPPUNIT_TEST_SUITE_END()
    };

} // namespace stack
