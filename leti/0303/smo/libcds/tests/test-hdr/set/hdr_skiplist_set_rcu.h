//$$CDS-header$$

#include "set/hdr_set.h"

namespace set {

    class SkipListSetRCUHdrTest: public HashSetHdrTest
    {
        typedef HashSetHdrTest base_class ;

        struct other_item
        {
            int nKey ;
            int nVal ;

            other_item& operator=( base_class::item const& src )
            {
                nKey = src.nKey ;
                nVal = src.nVal ;
                return *this;
            }
        };

        struct extract_copy_functor {
            void operator()( other_item& dest, base_class::item const& src ) const
            {
                dest = src;
            }
        };

        template <class Set, typename PrintStat >
        void test()
        {
            Set s ;
            base_class::test_int_with( s ) ;

            static int const nLimit = 10000 ;
            typedef typename Set::iterator          set_iterator ;
            typedef typename Set::const_iterator    const_set_iterator ;
            typedef typename Set::gc::scoped_lock   rcu_lock ;

            int nCount = 0  ;
            int nPrevKey = 0;

            int arrRandom[nLimit];
            for ( int i = 0; i < nLimit; ++i )
                arrRandom[i] = i ;
            std::random_shuffle( arrRandom, arrRandom + nLimit );

            // Test iterator - ascending order
            s.clear() ;
            CPPUNIT_ASSERT( s.empty() ) ;

            for ( int i = 0; i < nLimit; ++i ) {
                CPPUNIT_ASSERT( s.insert(i) ) ;
            }
            CPPUNIT_MSG( PrintStat()(s, "Iterator test, ascending insert order") ) ;

            nCount = 0  ;
            nPrevKey = 0;
            {
                rcu_lock l ;
                for ( set_iterator it = s.begin(), itEnd = s.end(); it != itEnd; ++it ) {
                    CPPUNIT_ASSERT( (*it).nKey == it->nVal )    ;
                    CPPUNIT_ASSERT( s.find( it->nKey )) ;
                    it->nVal = (*it).nKey * 2  ;
                    ++nCount    ;
                    if ( it != s.begin() ) {
                        CPPUNIT_ASSERT( nPrevKey + 1 == it->nKey ) ;
                    }
                    nPrevKey = it->nKey ;
                }
            }
            CPPUNIT_ASSERT( nCount == nLimit ) ;

            nCount = 0  ;
            {
                rcu_lock l ;
                for ( const_set_iterator it = s.cbegin(), itEnd = s.cend(); it != itEnd; ++it ) {
                    CPPUNIT_ASSERT( (*it).nKey * 2 == it->nVal )    ;
                    ++nCount    ;
                    if ( it != s.cbegin() ) {
                        CPPUNIT_ASSERT( nPrevKey + 1 == it->nKey ) ;
                    }
                    nPrevKey = it->nKey ;
                }
            }
            CPPUNIT_ASSERT( nCount == nLimit ) ;

            // Test iterator - descending order
            s.clear() ;
            CPPUNIT_ASSERT( s.empty() ) ;

            for ( int i = nLimit; i > 0; --i ) {
                CPPUNIT_ASSERT( s.insert( std::make_pair(i - 1, (i-1) * 2) )) ;
            }
            CPPUNIT_MSG( PrintStat()(s, "Iterator test, descending insert order") ) ;

            nCount = 0  ;
            nPrevKey = 0;
            {
                rcu_lock l ;
                for ( set_iterator it = s.begin(), itEnd = s.end(); it != itEnd; ++it ) {
                    CPPUNIT_ASSERT( (*it).nKey * 2 == it->nVal )    ;
                    CPPUNIT_ASSERT( s.find( it->nKey )) ;
                    it->nVal = (*it).nKey  ;
                    ++nCount    ;
                    if ( it != s.begin() ) {
                        CPPUNIT_ASSERT( nPrevKey + 1 == it->nKey ) ;
                    }
                    nPrevKey = it->nKey ;
                }
            }
            CPPUNIT_ASSERT( nCount == nLimit ) ;

            nCount = 0  ;
            {
                rcu_lock l ;
                for ( const_set_iterator it = s.cbegin(), itEnd = s.cend(); it != itEnd; ++it ) {
                    CPPUNIT_ASSERT( (*it).nKey == it->nVal )    ;
                    ++nCount    ;
                    if ( it != s.cbegin() ) {
                        CPPUNIT_ASSERT( nPrevKey + 1 == it->nKey ) ;
                    }
                    nPrevKey = it->nKey ;
                }
            }
            CPPUNIT_ASSERT( nCount == nLimit ) ;

            // Test iterator - random order
            s.clear() ;
            CPPUNIT_ASSERT( s.empty() ) ;
            {
                for ( int i = 0; i < nLimit; ++i ) {
                    CPPUNIT_ASSERT( s.insert( arrRandom[i] ) ) ;
                }
                CPPUNIT_MSG( PrintStat()(s, "Iterator test, random insert order") ) ;
            }

            nCount = 0  ;
            nPrevKey = 0;
            {
                rcu_lock l ;
                for ( set_iterator it = s.begin(), itEnd = s.end(); it != itEnd; ++it ) {
                    CPPUNIT_ASSERT( (*it).nKey == it->nVal )    ;
                    CPPUNIT_ASSERT( s.find( it->nKey )) ;
                    it->nVal = (*it).nKey * 2  ;
                    ++nCount    ;
                    if ( it != s.begin() ) {
                        CPPUNIT_ASSERT( nPrevKey + 1 == it->nKey ) ;
                    }
                    nPrevKey = it->nKey ;
                }
            }
            CPPUNIT_ASSERT( nCount == nLimit ) ;

            nCount = 0  ;
            {
                rcu_lock l ;
                for ( const_set_iterator it = s.cbegin(), itEnd = s.cend(); it != itEnd; ++it ) {
                    CPPUNIT_ASSERT( (*it).nKey * 2 == it->nVal )    ;
                    ++nCount    ;
                    if ( it != s.cbegin() ) {
                        CPPUNIT_ASSERT( nPrevKey + 1 == it->nKey ) ;
                    }
                    nPrevKey = it->nKey ;
                }
            }
            CPPUNIT_ASSERT( nCount == nLimit ) ;

            // extract/get tests
            {
                typedef typename base_class::less<typename Set::value_type> less_predicate ;
                typename Set::value_type * pVal ;
                typename Set::value_type * pVal2 ;

                // extract/get
                for ( int i = 0; i < nLimit; ++i ) {
                    rcu_lock l;
                    int nKey = arrRandom[i] ;
                    pVal = s.get( nKey );
                    CPPUNIT_ASSERT( pVal != NULL );
                    CPPUNIT_CHECK( pVal->nKey == nKey );
                    CPPUNIT_CHECK( pVal->nVal == nKey * 2 );

                    pVal2 = s.extract( nKey );
                    CPPUNIT_ASSERT( pVal2 != NULL );
                    CPPUNIT_CHECK( pVal2 == pVal );
                    CPPUNIT_CHECK( pVal2->nKey == nKey );
                    CPPUNIT_CHECK( pVal2->nVal == nKey * 2 );

                    CPPUNIT_CHECK( s.get( nKey ) == NULL );
                    CPPUNIT_CHECK( s.extract( nKey ) == NULL );
                }
                CPPUNIT_CHECK( s.empty());
                s.force_dispose();

                // extract_with/get_with
                for ( int i = 0; i < nLimit; ++i ) 
                    CPPUNIT_ASSERT( s.insert( arrRandom[i]) ) ;

                for ( int i = 0; i < nLimit; ++i ) {
                    rcu_lock l;
                    int nKey = arrRandom[i] ;
                    pVal = s.get_with( nKey, less_predicate() );
                    CPPUNIT_ASSERT( pVal != NULL );
                    CPPUNIT_CHECK( pVal->nKey == nKey );
                    CPPUNIT_CHECK( pVal->nVal == nKey );

                    pVal2 = s.extract_with( nKey, less_predicate() );
                    CPPUNIT_ASSERT( pVal2 != NULL );
                    CPPUNIT_CHECK( pVal2 == pVal );
                    CPPUNIT_CHECK( pVal2->nKey == nKey );
                    CPPUNIT_CHECK( pVal2->nVal == nKey );

                    CPPUNIT_CHECK( s.get_with( nKey, less_predicate() ) == NULL );
                    CPPUNIT_CHECK( s.extract_with( nKey, less_predicate() ) == NULL );
                }
                CPPUNIT_CHECK( s.empty());
                s.force_dispose();

                // extract_min
                for ( int i = 0; i < nLimit; ++i ) 
                    CPPUNIT_ASSERT( s.insert( arrRandom[i]) ) ;

                for ( int i = 0; i < nLimit; ++i ) {
                    rcu_lock l;
                    typename Set::value_type * pMin = s.get_min();
                    CPPUNIT_ASSERT( pMin != NULL );
                    CPPUNIT_CHECK( pMin->nKey == i );
                    CPPUNIT_CHECK( pMin->nVal == i );

                    pVal = s.extract_min();
                    CPPUNIT_ASSERT( pVal != NULL );
                    CPPUNIT_CHECK( pVal == pMin );
                    CPPUNIT_CHECK( pVal->nKey == i );
                    CPPUNIT_CHECK( pVal->nVal == i );
                    CPPUNIT_CHECK( !s.find(i) );
                }
                CPPUNIT_CHECK( s.empty());
                s.force_dispose();
                {
                    rcu_lock l;
                    CPPUNIT_CHECK( s.get_min() == NULL );
                    CPPUNIT_CHECK( s.get_max() == NULL );
                    CPPUNIT_CHECK( s.extract_min() == NULL );
                    CPPUNIT_CHECK( s.extract_max() == NULL );
                }

                for ( int i = 0; i < nLimit; ++i ) 
                    CPPUNIT_ASSERT( s.insert( arrRandom[i]) ) ;

                for ( int i = 0; i < nLimit; ++i ) {
                    other_item itm ;
                    CPPUNIT_ASSERT( s.extract_min(itm)) ;
                    CPPUNIT_CHECK( itm.nKey == i );
                    CPPUNIT_CHECK( itm.nVal == i );
                    CPPUNIT_CHECK( !s.find( i ));
                }
                CPPUNIT_ASSERT( s.empty() );

                for ( int i = 0; i < nLimit; ++i ) 
                    CPPUNIT_ASSERT( s.insert( arrRandom[i]) ) ;

                for ( int i = 0; i < nLimit; ++i ) {
                    other_item itm ;
                    CPPUNIT_ASSERT( s.extract_min(itm, extract_copy_functor() )) ;
                    CPPUNIT_CHECK( itm.nKey == i );
                    CPPUNIT_CHECK( itm.nVal == i );
                    CPPUNIT_CHECK( !s.find( i ));
                }
                CPPUNIT_ASSERT( s.empty() );

                // extract_max
                for ( int i = 0; i < nLimit; ++i ) 
                    CPPUNIT_ASSERT( s.insert( arrRandom[i]) ) ;

                for ( int i = nLimit-1; i >= 0; --i ) {
                    rcu_lock l;
                    typename Set::value_type * pMax = s.get_max();
                    CPPUNIT_ASSERT( pMax != NULL );
                    CPPUNIT_CHECK( pMax->nKey == i );
                    CPPUNIT_CHECK( pMax->nVal == i );

                    pVal = s.extract_max();
                    CPPUNIT_ASSERT( pVal != NULL );
                    CPPUNIT_CHECK( pMax == pVal );
                    CPPUNIT_CHECK( pVal->nKey == i );
                    CPPUNIT_CHECK( pVal->nVal == i );
                    CPPUNIT_CHECK( !s.find(i) );
                }
                CPPUNIT_CHECK( s.empty());
                s.force_dispose();
                {
                    rcu_lock l;
                    CPPUNIT_CHECK( s.get_min() == NULL );
                    CPPUNIT_CHECK( s.get_max() == NULL );
                    CPPUNIT_CHECK( s.extract_min() == NULL );
                    CPPUNIT_CHECK( s.extract_max() == NULL );
                }

                for ( int i = 0; i < nLimit; ++i ) 
                    CPPUNIT_ASSERT( s.insert( arrRandom[i]) ) ;

                for ( int i = nLimit-1; i >= 0; --i ) {
                    other_item itm ;
                    CPPUNIT_ASSERT( s.extract_max(itm)) ;
                    CPPUNIT_CHECK( itm.nKey == i );
                    CPPUNIT_CHECK( itm.nVal == i );
                    CPPUNIT_CHECK( !s.find( i ));
                }
                CPPUNIT_ASSERT( s.empty() );

                for ( int i = 0; i < nLimit; ++i ) 
                    CPPUNIT_ASSERT( s.insert( arrRandom[i]) ) ;

                for ( int i = nLimit-1; i >= 0; --i ) {
                    other_item itm ;
                    CPPUNIT_ASSERT( s.extract_max(itm, extract_copy_functor() )) ;
                    CPPUNIT_CHECK( itm.nKey == i );
                    CPPUNIT_CHECK( itm.nVal == i );
                    CPPUNIT_CHECK( !s.find( i ));
                }
                CPPUNIT_ASSERT( s.empty() );
            }

            CPPUNIT_MSG( PrintStat()(s, "End of test") ) ;
        }

    public:
        void SkipList_RCU_GPI_less()  ;
        void SkipList_RCU_GPI_cmp()  ;
        void SkipList_RCU_GPI_cmpless()  ;
        void SkipList_RCU_GPI_less_stat()  ;
        void SkipList_RCU_GPI_cmp_stat()  ;
        void SkipList_RCU_GPI_cmpless_stat()  ;
        void SkipList_RCU_GPI_xorshift_less()  ;
        void SkipList_RCU_GPI_xorshift_cmp()  ;
        void SkipList_RCU_GPI_xorshift_cmpless()  ;
        void SkipList_RCU_GPI_xorshift_less_stat()  ;
        void SkipList_RCU_GPI_xorshift_cmp_stat()  ;
        void SkipList_RCU_GPI_xorshift_cmpless_stat()  ;
        void SkipList_RCU_GPI_turbopas_less()  ;
        void SkipList_RCU_GPI_turbopas_cmp()  ;
        void SkipList_RCU_GPI_turbopas_cmpless()  ;
        void SkipList_RCU_GPI_turbopas_less_stat()  ;
        void SkipList_RCU_GPI_turbopas_cmp_stat()  ;
        void SkipList_RCU_GPI_turbopas_cmpless_stat()  ;
        void SkipList_RCU_GPI_michaelalloc_less()  ;
        void SkipList_RCU_GPI_michaelalloc_cmp()  ;
        void SkipList_RCU_GPI_michaelalloc_cmpless()  ;
        void SkipList_RCU_GPI_michaelalloc_less_stat()  ;
        void SkipList_RCU_GPI_michaelalloc_cmp_stat()  ;
        void SkipList_RCU_GPI_michaelalloc_cmpless_stat()  ;

        void SkipList_RCU_GPB_less()  ;
        void SkipList_RCU_GPB_cmp()  ;
        void SkipList_RCU_GPB_cmpless()  ;
        void SkipList_RCU_GPB_less_stat()  ;
        void SkipList_RCU_GPB_cmp_stat()  ;
        void SkipList_RCU_GPB_cmpless_stat()  ;
        void SkipList_RCU_GPB_xorshift_less()  ;
        void SkipList_RCU_GPB_xorshift_cmp()  ;
        void SkipList_RCU_GPB_xorshift_cmpless()  ;
        void SkipList_RCU_GPB_xorshift_less_stat()  ;
        void SkipList_RCU_GPB_xorshift_cmp_stat()  ;
        void SkipList_RCU_GPB_xorshift_cmpless_stat()  ;
        void SkipList_RCU_GPB_turbopas_less()  ;
        void SkipList_RCU_GPB_turbopas_cmp()  ;
        void SkipList_RCU_GPB_turbopas_cmpless()  ;
        void SkipList_RCU_GPB_turbopas_less_stat()  ;
        void SkipList_RCU_GPB_turbopas_cmp_stat()  ;
        void SkipList_RCU_GPB_turbopas_cmpless_stat()  ;
        void SkipList_RCU_GPB_michaelalloc_less()  ;
        void SkipList_RCU_GPB_michaelalloc_cmp()  ;
        void SkipList_RCU_GPB_michaelalloc_cmpless()  ;
        void SkipList_RCU_GPB_michaelalloc_less_stat()  ;
        void SkipList_RCU_GPB_michaelalloc_cmp_stat()  ;
        void SkipList_RCU_GPB_michaelalloc_cmpless_stat()  ;

        void SkipList_RCU_GPT_less()  ;
        void SkipList_RCU_GPT_cmp()  ;
        void SkipList_RCU_GPT_cmpless()  ;
        void SkipList_RCU_GPT_less_stat()  ;
        void SkipList_RCU_GPT_cmp_stat()  ;
        void SkipList_RCU_GPT_cmpless_stat()  ;
        void SkipList_RCU_GPT_xorshift_less()  ;
        void SkipList_RCU_GPT_xorshift_cmp()  ;
        void SkipList_RCU_GPT_xorshift_cmpless()  ;
        void SkipList_RCU_GPT_xorshift_less_stat()  ;
        void SkipList_RCU_GPT_xorshift_cmp_stat()  ;
        void SkipList_RCU_GPT_xorshift_cmpless_stat()  ;
        void SkipList_RCU_GPT_turbopas_less()  ;
        void SkipList_RCU_GPT_turbopas_cmp()  ;
        void SkipList_RCU_GPT_turbopas_cmpless()  ;
        void SkipList_RCU_GPT_turbopas_less_stat()  ;
        void SkipList_RCU_GPT_turbopas_cmp_stat()  ;
        void SkipList_RCU_GPT_turbopas_cmpless_stat()  ;
        void SkipList_RCU_GPT_michaelalloc_less()  ;
        void SkipList_RCU_GPT_michaelalloc_cmp()  ;
        void SkipList_RCU_GPT_michaelalloc_cmpless()  ;
        void SkipList_RCU_GPT_michaelalloc_less_stat()  ;
        void SkipList_RCU_GPT_michaelalloc_cmp_stat()  ;
        void SkipList_RCU_GPT_michaelalloc_cmpless_stat()  ;

        void SkipList_RCU_SHB_less()  ;
        void SkipList_RCU_SHB_cmp()  ;
        void SkipList_RCU_SHB_cmpless()  ;
        void SkipList_RCU_SHB_less_stat()  ;
        void SkipList_RCU_SHB_cmp_stat()  ;
        void SkipList_RCU_SHB_cmpless_stat()  ;
        void SkipList_RCU_SHB_xorshift_less()  ;
        void SkipList_RCU_SHB_xorshift_cmp()  ;
        void SkipList_RCU_SHB_xorshift_cmpless()  ;
        void SkipList_RCU_SHB_xorshift_less_stat()  ;
        void SkipList_RCU_SHB_xorshift_cmp_stat()  ;
        void SkipList_RCU_SHB_xorshift_cmpless_stat()  ;
        void SkipList_RCU_SHB_turbopas_less()  ;
        void SkipList_RCU_SHB_turbopas_cmp()  ;
        void SkipList_RCU_SHB_turbopas_cmpless()  ;
        void SkipList_RCU_SHB_turbopas_less_stat()  ;
        void SkipList_RCU_SHB_turbopas_cmp_stat()  ;
        void SkipList_RCU_SHB_turbopas_cmpless_stat()  ;
        void SkipList_RCU_SHB_michaelalloc_less()  ;
        void SkipList_RCU_SHB_michaelalloc_cmp()  ;
        void SkipList_RCU_SHB_michaelalloc_cmpless()  ;
        void SkipList_RCU_SHB_michaelalloc_less_stat()  ;
        void SkipList_RCU_SHB_michaelalloc_cmp_stat()  ;
        void SkipList_RCU_SHB_michaelalloc_cmpless_stat()  ;

        void SkipList_RCU_SHT_less()  ;
        void SkipList_RCU_SHT_cmp()  ;
        void SkipList_RCU_SHT_cmpless()  ;
        void SkipList_RCU_SHT_less_stat()  ;
        void SkipList_RCU_SHT_cmp_stat()  ;
        void SkipList_RCU_SHT_cmpless_stat()  ;
        void SkipList_RCU_SHT_xorshift_less()  ;
        void SkipList_RCU_SHT_xorshift_cmp()  ;
        void SkipList_RCU_SHT_xorshift_cmpless()  ;
        void SkipList_RCU_SHT_xorshift_less_stat()  ;
        void SkipList_RCU_SHT_xorshift_cmp_stat()  ;
        void SkipList_RCU_SHT_xorshift_cmpless_stat()  ;
        void SkipList_RCU_SHT_turbopas_less()  ;
        void SkipList_RCU_SHT_turbopas_cmp()  ;
        void SkipList_RCU_SHT_turbopas_cmpless()  ;
        void SkipList_RCU_SHT_turbopas_less_stat()  ;
        void SkipList_RCU_SHT_turbopas_cmp_stat()  ;
        void SkipList_RCU_SHT_turbopas_cmpless_stat()  ;
        void SkipList_RCU_SHT_michaelalloc_less()  ;
        void SkipList_RCU_SHT_michaelalloc_cmp()  ;
        void SkipList_RCU_SHT_michaelalloc_cmpless()  ;
        void SkipList_RCU_SHT_michaelalloc_less_stat()  ;
        void SkipList_RCU_SHT_michaelalloc_cmp_stat()  ;
        void SkipList_RCU_SHT_michaelalloc_cmpless_stat()  ;

        CPPUNIT_TEST_SUITE(SkipListSetRCUHdrTest)
            CPPUNIT_TEST(SkipList_RCU_GPI_less)
            CPPUNIT_TEST(SkipList_RCU_GPI_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPI_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPI_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_xorshift_less)
            CPPUNIT_TEST(SkipList_RCU_GPI_xorshift_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPI_xorshift_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPI_xorshift_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_xorshift_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_xorshift_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_turbopas_less)
            CPPUNIT_TEST(SkipList_RCU_GPI_turbopas_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPI_turbopas_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPI_turbopas_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_turbopas_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_turbopas_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_michaelalloc_less)
            CPPUNIT_TEST(SkipList_RCU_GPI_michaelalloc_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPI_michaelalloc_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPI_michaelalloc_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_michaelalloc_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPI_michaelalloc_cmpless_stat)

            CPPUNIT_TEST(SkipList_RCU_GPB_less)
            CPPUNIT_TEST(SkipList_RCU_GPB_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPB_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPB_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_xorshift_less)
            CPPUNIT_TEST(SkipList_RCU_GPB_xorshift_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPB_xorshift_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPB_xorshift_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_xorshift_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_xorshift_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_turbopas_less)
            CPPUNIT_TEST(SkipList_RCU_GPB_turbopas_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPB_turbopas_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPB_turbopas_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_turbopas_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_turbopas_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_michaelalloc_less)
            CPPUNIT_TEST(SkipList_RCU_GPB_michaelalloc_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPB_michaelalloc_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPB_michaelalloc_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_michaelalloc_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPB_michaelalloc_cmpless_stat)

            CPPUNIT_TEST(SkipList_RCU_GPT_less)
            CPPUNIT_TEST(SkipList_RCU_GPT_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPT_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPT_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_xorshift_less)
            CPPUNIT_TEST(SkipList_RCU_GPT_xorshift_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPT_xorshift_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPT_xorshift_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_xorshift_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_xorshift_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_turbopas_less)
            CPPUNIT_TEST(SkipList_RCU_GPT_turbopas_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPT_turbopas_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPT_turbopas_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_turbopas_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_turbopas_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_michaelalloc_less)
            CPPUNIT_TEST(SkipList_RCU_GPT_michaelalloc_cmp)
            CPPUNIT_TEST(SkipList_RCU_GPT_michaelalloc_cmpless)
            CPPUNIT_TEST(SkipList_RCU_GPT_michaelalloc_less_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_michaelalloc_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_GPT_michaelalloc_cmpless_stat)

            CPPUNIT_TEST(SkipList_RCU_SHB_less)
            CPPUNIT_TEST(SkipList_RCU_SHB_cmp)
            CPPUNIT_TEST(SkipList_RCU_SHB_cmpless)
            CPPUNIT_TEST(SkipList_RCU_SHB_less_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_xorshift_less)
            CPPUNIT_TEST(SkipList_RCU_SHB_xorshift_cmp)
            CPPUNIT_TEST(SkipList_RCU_SHB_xorshift_cmpless)
            CPPUNIT_TEST(SkipList_RCU_SHB_xorshift_less_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_xorshift_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_xorshift_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_turbopas_less)
            CPPUNIT_TEST(SkipList_RCU_SHB_turbopas_cmp)
            CPPUNIT_TEST(SkipList_RCU_SHB_turbopas_cmpless)
            CPPUNIT_TEST(SkipList_RCU_SHB_turbopas_less_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_turbopas_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_turbopas_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_michaelalloc_less)
            CPPUNIT_TEST(SkipList_RCU_SHB_michaelalloc_cmp)
            CPPUNIT_TEST(SkipList_RCU_SHB_michaelalloc_cmpless)
            CPPUNIT_TEST(SkipList_RCU_SHB_michaelalloc_less_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_michaelalloc_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_SHB_michaelalloc_cmpless_stat)

            CPPUNIT_TEST(SkipList_RCU_SHT_less)
            CPPUNIT_TEST(SkipList_RCU_SHT_cmp)
            CPPUNIT_TEST(SkipList_RCU_SHT_cmpless)
            CPPUNIT_TEST(SkipList_RCU_SHT_less_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_xorshift_less)
            CPPUNIT_TEST(SkipList_RCU_SHT_xorshift_cmp)
            CPPUNIT_TEST(SkipList_RCU_SHT_xorshift_cmpless)
            CPPUNIT_TEST(SkipList_RCU_SHT_xorshift_less_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_xorshift_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_xorshift_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_turbopas_less)
            CPPUNIT_TEST(SkipList_RCU_SHT_turbopas_cmp)
            CPPUNIT_TEST(SkipList_RCU_SHT_turbopas_cmpless)
            CPPUNIT_TEST(SkipList_RCU_SHT_turbopas_less_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_turbopas_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_turbopas_cmpless_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_michaelalloc_less)
            CPPUNIT_TEST(SkipList_RCU_SHT_michaelalloc_cmp)
            CPPUNIT_TEST(SkipList_RCU_SHT_michaelalloc_cmpless)
            CPPUNIT_TEST(SkipList_RCU_SHT_michaelalloc_less_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_michaelalloc_cmp_stat)
            CPPUNIT_TEST(SkipList_RCU_SHT_michaelalloc_cmpless_stat)

        CPPUNIT_TEST_SUITE_END()

    };
}
