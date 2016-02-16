//$$CDS-header$$

#ifndef __CDS_INTRUSIVE_MSPRIORITY_QUEUE_H
#define __CDS_INTRUSIVE_MSPRIORITY_QUEUE_H

#include <cds/container/mspriority_queue.h>

namespace cds { namespace intrusive {

    /// MSPriorityQueue related definitions
    namespace mspriority_queue {
#ifdef CDS_DOXYGEN_INVOKED
        /// Type traits for MSPriorityQueue, synonym for cds::container::mspriority_queue::type_traits
        struct type_traits: public cds::container::mspriority_queue::type_traits
        {};

        /// Metafunction converting option list to traits struct, synonym for cds::container::mspriority_queue::make_traits
        template <CDS_DECL_OPTIONS7>
        struct make_traits: public cds::container::mspriority_queue::make_traits<CDS_OPTIONS7>
        {};
#else
        using cds::container::mspriority_queue::type_traits ;
        using cds::container::mspriority_queue::make_traits ;
#endif
        //@cond
        template <typename T, typename Traits>
        struct make_queue
        {
            typedef T       value_type  ;
            typedef Traits  type_traits ;

            typedef typename opt::details::make_comparator< value_type, type_traits >::type key_comparator  ;

            // Comparator for T *
            struct compare_wrapper: private key_comparator
            {
                typedef key_comparator  base_class  ;

                int operator()( value_type const* p1, value_type const* p2 ) const
                {
                    return base_class::operator()( *p1, *p2 ) ;
                }
                int operator()( value_type* p1, value_type* p2 ) const
                {
                    return base_class::operator()( *p1, *p2 ) ;
                }

                template <typename Q>
                int operator()( value_type const* p, Q const& q ) const
                {
                    return base_class::operator()( *p, q ) ;
                }
                template <typename Q>
                int operator()( value_type* p, Q const& q ) const
                {
                    return base_class::operator()( *p, q ) ;
                }

                template <typename Q>
                int operator()( Q const& q, value_type const* p ) const
                {
                    return base_class::operator()( q, *p ) ;
                }
                template <typename Q>
                int operator()( Q const& q, value_type* p ) const
                {
                    return base_class::operator()( q, *p ) ;
                }

                template <typename Q, typename U>
                int operator()( Q const& q, U const& u ) const
                {
                    return base_class::operator()( q, u ) ;
                }
            };

            // Traits for T *
            struct traits: public type_traits
            {
                typedef compare_wrapper                     compare     ;
                typedef cds::opt::v::default_swap_policy    swap_policy ;
                typedef cds::opt::v::assignment_move_policy move_policy ;
            };

            // Result of metafunction
            typedef cds::container::MSPriorityQueue< value_type*, traits >  type ;
        };
        //@endcond
    } // namespace mspriority_queue

    /// Michael & Scott array-based lock-based concurrent priority queue heap
    /** @ingroup cds_intrusive_priority_queue
        Source:
            - [1996] G.Hunt, M.Michael, S. Parthasarathy, M.Scott
                "An efficient algorithm for concurrent priority queue heaps"

        \p %MSPriorityQueue augments the standard array-based heap data structure with
        a mutual-exclusion lock on the heap's size and locks on each node in the heap.
        Each node also has a tag that indicates whether
        it is empty, valid, or in a transient state due to an update to the heap
        by an inserting thread.
        The algorithm allows concurrent insertions and deletions in opposite directions,
        without risking deadlock and without the need for special server threads.
        It also uses a "bit-reversal" technique to scatter accesses across the fringe
        of the tree to reduce contention.
        On large heaps the algorithm achieves significant performance improvements
        over serialized single-lock algorithm, for various insertion/deletion
        workloads. For small heaps it still performs well, but not as well as
        single-lock algorithm.

        Template parameters:
        - \p T - type to be stored in the list. The priority is a part of \p T type.
        - \p Traits - type traits. See mspriority_queue::type_traits for explanation.

        It is possible to declare option-based queue with cds::intrusive::mspriority_queue::make_traits
        metafunction instead of \p Traits template argument.
        Template argument list \p Options of \p %cds::intrusive::mspriority_queue::make_traits metafunction are:
        - opt::buffer - the buffer type for heap array. Possible type are: opt::v::static_buffer, opt::v::dynamic_buffer.
            Default is \p %opt::v::dynamic_buffer.
            At \p %MSPriorityQueue class instantiation, the \p buffer::rebind member metafunction
            is called to change type of values stored in the buffer. So, you can specify any type of values here.
        - opt::compare - priority compare functor. No default functor is provided.
            If the option is not specified, the opt::less is used.
        - opt::less - specifies binary predicate used for priority compare. Default is \p std::less<T>.
        - opt::lock_type - lock type. Default is cds::lock::Spin.
        - opt::back_off - back-off strategy. Default is cds::backoff::yield
        - intrusive::opt::disposer - the functor used for dispose removed items. Default is opt::v::empty_disposer.
            Used in \ref clear member function.

    \par Usage

    */
    template <typename T, class Traits>
    class MSPriorityQueue
#ifdef CDS_DOXYGEN_INVOKED
        : protected cds::container::MSPriorityQueue< T*, Traits >
#else
        : protected mspriority_queue::make_queue< T, Traits>::type
#endif
    {
        //@cond
        typedef mspriority_queue::make_queue< T, Traits>    maker       ;
        typedef typename maker::type                        base_class  ;
        //@endcond

    public:
        typedef T           value_type  ;   ///< Value type stored in the queue
        typedef Traits      options     ;   ///< Traits template parameter

        typedef typename base_class::lock_type  lock_type ; ///< heap's size lock type
        typedef typename base_class::back_off   back_off  ; ///< Back-off strategy
        typedef typename options::disposer      disposer  ; ///< Disposer type (used in \ref clear member function)

    protected:
        //@cond
#   ifndef CDS_CXX11_LAMBDA_SUPPORT
        struct clear_wrapper {
            void operator()( value_type * p )
            {
                disposer()( p ) ;
            }
        };

        template <typename Func>
        struct clear_wrapper2 {
            Func& disp ;

            clear_wrapper2( Func& f ): disp(f) {}
            void operator()( value_type * p )
            {
                disp( p ) ;
            }
        };
#   endif
        //@endcond

    public:
        /// Constructs empty priority queue
        /**
            For cds::opt::v::static_buffer the \p nCapacity parameter is ignored.
        */
        MSPriorityQueue( size_t nCapacity )
            : base_class( nCapacity )
        {}

        /// Clears priority queue and destructs the object
        ~MSPriorityQueue()
        {
            clear() ;
        }

        /// Inserts a item into priority queue
        /**
            If the priority queue is full, the function returns \p false,
            no item has been added.
            Otherwise, the function inserts pointer to \p val into the heap
            and returns \p true.
        */
        bool push( value_type& val )
        {
            return base_class::push( &val ) ;
        }

        /// Extracts item with high priority
        /**
            If the priority queue is empty, the function returns \p NULL.
            Otherwise, it returns pointer to extracted item.
            The item is deleted from the heap.
        */
        value_type * pop()
        {
            value_type * p  ;
            return base_class::pop( p ) ? p : null_ptr<value_type *>() ;
        }

        /// Clears the queue (not atomic)
        /**
            This function is no atomic, but thread-safe.

            For each removed item the disposer is called.
        */
        void clear()
        {
#       ifdef CDS_CXX11_LAMBDA_SUPPORT
            base_class::clear_with( []( value_type * p ) { disposer()( p ); } ) ;
#       else
            base_class::clear_with( clear_wrapper() ) ;
#       endif
        }

        /// Clears the queue (not atomic)
        /**
            This function is no atomic, but thread-safe.

            For each removed item the disposer \p disp is called.
            \p Disposer interface is:
            \code
            struct my_disposer
            {
                void operator()( value_type * p ) ;
            };
            \endcode
            A lambda function or a function pointer can be used as \p disp.
        */
        template <typename Disposer>
        void clear_with( Disposer disp )
        {
#       ifdef CDS_CXX11_LAMBDA_SUPPORT
            base_class::clear_with( [&disp]( value_type * p ) { disp( p ); } ) ;
#       else
            base_class::clear_with( clear_wrapper2<Disposer>( disp ) ) ;
#       endif
        }

        /// Checks is the priority queue is empty
        bool empty() const
        {
            return base_class::empty() ;
        }

        /// Checks if the priority queue is full
        bool full() const
        {
            return base_class::full() ;
        }

        /// Returns current size of priority queue
        size_t size() const
        {
            return base_class::size() ;
        }

        /// Return capacity of the priority queue
        size_t capacity() const
        {
            return base_class::capacity() ;
        }
    };

}} // namespace cds::intrusive

#endif // #ifndef __CDS_INTRUSIVE_MSPRIORITY_QUEUE_H
