//$$CDS-header$$

#ifndef __UNIT_PQUEUE_TYPES_H
#define __UNIT_PQUEUE_TYPES_H

#include <cds/container/mspriority_queue.h>

#include "pqueue/std_pqueue.h"
#include "pqueue/ellen_bintree_pqueue.h"

#include <vector>
#include <deque>
#include <cds/details/std/mutex.h>
#include <cds/lock/spinlock.h>

namespace pqueue {
    namespace cc = cds::container   ;
    namespace co = cds::opt         ;

    template <typename Value>
    struct Types
    {
        static size_t const c_nBoundedCapacity = 1024 * 1024 * 16 ;

        typedef std::less<Value>    less ;

        struct cmp {
            int operator()( Value const& v1, Value const& v2 ) const
            {
                return less()( v1, v2 ) ? -1 : less()( v2, v1 ) ? 1 : 0 ;
            }
        };



        // MSPriorityQueue
        typedef cc::MSPriorityQueue< Value,
            typename cc::mspriority_queue::make_traits<
                co::buffer< co::v::static_buffer< char, c_nBoundedCapacity > >
            >::type
        > MSPriorityQueue_static_less  ;

        typedef cc::MSPriorityQueue< Value,
            typename cc::mspriority_queue::make_traits<
                co::buffer< co::v::static_buffer< char, c_nBoundedCapacity > >
                ,co::compare< cmp >
            >::type
        > MSPriorityQueue_static_cmp  ;

        typedef cc::MSPriorityQueue< Value,
            typename cc::mspriority_queue::make_traits<
                co::buffer< co::v::static_buffer< char, c_nBoundedCapacity > >
                ,co::lock_type<cds_std::mutex>
            >::type
        > MSPriorityQueue_static_mutex  ;

        typedef cc::MSPriorityQueue< Value,
            typename cc::mspriority_queue::make_traits<
                co::buffer< co::v::dynamic_buffer< char > >
            >::type
        > MSPriorityQueue_dyn_less  ;

        typedef cc::MSPriorityQueue< Value,
            typename cc::mspriority_queue::make_traits<
                co::buffer< co::v::dynamic_buffer< char > >
                ,co::compare< cmp >
            >::type
        > MSPriorityQueue_dyn_cmp  ;

        typedef cc::MSPriorityQueue< Value,
            typename cc::mspriority_queue::make_traits<
                co::buffer< co::v::dynamic_buffer< char > >
                ,co::lock_type<cds_std::mutex>
            >::type
        > MSPriorityQueue_dyn_mutex  ;


        // Priority queue based on EllenBinTreeSet
        typedef EllenBinTreePQueue< cds::gc::HP, typename Value::key_type, Value,
            typename cc::ellen_bintree::make_set_traits<
                cc::ellen_bintree::key_extractor< typename Value::key_extractor >
                ,cc::opt::less< std::less<Value> >
            >::type
        > EllenBinTree_HP   ;

        typedef EllenBinTreePQueue< cds::gc::PTB, typename Value::key_type, Value,
            typename cc::ellen_bintree::make_set_traits<
                cc::ellen_bintree::key_extractor< typename Value::key_extractor >
                ,cc::opt::less< std::less<Value> >
            >::type
        > EllenBinTree_PTB   ;

        typedef EllenBinTreePQueue< cds::urcu::gc< cds::urcu::general_instant<> >, typename Value::key_type, Value,
            typename cc::ellen_bintree::make_set_traits<
                cc::ellen_bintree::key_extractor< typename Value::key_extractor >
                ,cc::opt::less< std::less<Value> >
            >::type
        > EllenBinTree_RCU_gpi   ;

        typedef EllenBinTreePQueue< cds::urcu::gc< cds::urcu::general_buffered<> >, typename Value::key_type, Value,
            typename cc::ellen_bintree::make_set_traits<
                cc::ellen_bintree::key_extractor< typename Value::key_extractor >
                ,cc::opt::less< std::less<Value> >
            >::type
        > EllenBinTree_RCU_gpb   ;

        typedef EllenBinTreePQueue< cds::urcu::gc< cds::urcu::general_threaded<> >, typename Value::key_type, Value,
            typename cc::ellen_bintree::make_set_traits<
                cc::ellen_bintree::key_extractor< typename Value::key_extractor >
                ,cc::opt::less< std::less<Value> >
            >::type
        > EllenBinTree_RCU_gpt   ;

#ifdef CDS_URCU_SIGNAL_HANDLING_ENABLED
        typedef EllenBinTreePQueue< cds::urcu::gc< cds::urcu::signal_buffered<> >, typename Value::key_type, Value,
            typename cc::ellen_bintree::make_set_traits<
                cc::ellen_bintree::key_extractor< typename Value::key_extractor >
                ,cc::opt::less< std::less<Value> >
            >::type
        > EllenBinTree_RCU_shb   ;

        typedef EllenBinTreePQueue< cds::urcu::gc< cds::urcu::signal_threaded<> >, typename Value::key_type, Value,
            typename cc::ellen_bintree::make_set_traits<
                cc::ellen_bintree::key_extractor< typename Value::key_extractor >
                ,cc::opt::less< std::less<Value> >
            >::type
        > EllenBinTree_RCU_sht   ;
#endif
        /// Standard priority_queue
        typedef StdPQueue< Value, std::vector<Value>, cds::lock::Spin > StdPQueue_vector_spin   ;
        typedef StdPQueue< Value, std::vector<Value>, cds_std::mutex >  StdPQueue_vector_mutex  ;
        typedef StdPQueue< Value, std::deque<Value>,  cds::lock::Spin > StdPQueue_deque_spin    ;
        typedef StdPQueue< Value, std::deque<Value>,  cds_std::mutex >  StdPQueue_deque_mutex   ;
    };

}   // namespace pqueue

#endif // #ifndef __UNIT_PQUEUE_TYPES_H
