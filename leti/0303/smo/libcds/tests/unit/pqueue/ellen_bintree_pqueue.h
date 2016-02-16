//$$CDS-header$$

#ifndef __CDSUNIT_STD_ELLEN_BINTREE_PQUEUE_H
#define __CDSUNIT_STD_ELLEN_BINTREE_PQUEUE_H

#include <cds/container/ellen_bintree_set_hp.h>
#include <cds/container/ellen_bintree_set_ptb.h>
#include <cds/urcu/general_instant.h>
#include <cds/urcu/general_buffered.h>
#include <cds/urcu/general_threaded.h>
#include <cds/urcu/signal_buffered.h>
#include <cds/urcu/signal_threaded.h>
#include <cds/container/ellen_bintree_set_rcu.h>

namespace pqueue {

    template <typename GC, typename Key, typename T, typename Traits>
    class EllenBinTreePQueue: protected cds::container::EllenBinTreeSet< GC, Key, T, Traits >
    {
        typedef cds::container::EllenBinTreeSet< GC, Key, T, Traits > base_class ;
        typedef T value_type ;

    public:
        bool push( value_type const& val )
        {
            return base_class::insert( val );
        }

        bool pop( value_type& dest )
        {
            return base_class::extract_max( dest );
        }

        template <typename Q, typename MoveFunc>
        bool pop_with( Q& dest, MoveFunc f )
        {
            return base_class::extract_max( dest, f );
        }

        void clear()
        {
            base_class::clear();
        }

        template <typename Func>
        void clear_with( Func f )
        {
            base_class::clear();
        }

        bool empty() const
        {
            return base_class::empty();
        }

        size_t size() const
        {
            return base_class::size();
        }
    };

} // namespace pqueue


#endif // #ifndef __CDSUNIT_STD_ELLEN_BINTREE_PQUEUE_H
