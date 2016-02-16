//$$CDS-header$$

#ifndef __CDS_CONTAINER_MSPRIORITY_QUEUE_H
#define __CDS_CONTAINER_MSPRIORITY_QUEUE_H

#include <cds/container/base.h>
#include <cds/lock/spinlock.h>
#include <cds/os/thread.h>
#include <cds/details/bit_reverse_counter.h>
#include <cds/intrusive/options.h>
#include <cds/opt/buffer.h>
#include <cds/opt/compare.h>
#include <cds/details/bounded_container.h>
#include <cds/details/aligned_type.h>

namespace cds { namespace container {

    /// MSPriorityQueue related definitions
    namespace mspriority_queue {

        /// Type traits for MSPriorityQueue
        struct type_traits {
            /// Storage type
            /**
                The storage type for the heap array. Default is cds::opt::v::dynamic_buffer is used.

                At MSPriorityQueue class instantiation, the \p buffer::rebind member metafunction
                is called to change type of values stored in the buffer. So, you may specify
                any type of values here.
            */
            typedef opt::v::dynamic_buffer<void *>  buffer      ;

            /// Priority compare functor
            /**
                No default functor is provided. If the option is not specified, the \p less is used.
            */
            typedef opt::none           compare     ;

            /// specifies binary predicate used for priority comparing.
            /**
                Default is \p std::less<T>.
            */
            typedef opt::none           less        ;

            /// Type of mutual-exclusion lock
            typedef lock::Spin          lock_type   ;

            /// Back-off strategy
            typedef backoff::yield      back_off    ;

            /// Swap policy
            /**
                The swap policy is used in heapify algorithm.
                Default is opt::v::default_swap_policy.
            */
            typedef cds::opt::v::default_swap_policy    swap_policy ;

            /// Move policy
            /**
                The move policy used in MSPriorityQueue::pop functions
                to move item's value.
                Default is opt::v::assignment_move_policy.
            */
            typedef cds::opt::v::assignment_move_policy  move_policy ;

            /// Disposer of removed item (only for intrusive queue)
            typedef cds::intrusive::opt::v::empty_disposer  disposer ;
        };

        /// Metafunction converting option list to traits
        /**
            This is a wrapper for <tt> cds::opt::make_options< type_traits, Options...> </tt>

            See \ref MSPriorityQueue, \ref type_traits, \ref cds::opt::make_options.
        */
        template <CDS_DECL_OPTIONS8>
        struct make_traits {
#   ifdef CDS_DOXYGEN_INVOKED
            typedef implementation_defined type ;   ///< Metafunction result
#   else
            typedef typename cds::opt::make_options<
                typename cds::opt::find_type_traits< type_traits, CDS_OPTIONS8 >::type
                ,CDS_OPTIONS8
            >::type   type ;
#   endif
        };

    }   // namespace mspriority_queue

    /// Michael & Scott array-based lock-based concurrent priority queue heap
    /** @ingroup cds_nonintrusive_priority_queue
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

        It is possible to declare option-based queue with cds::container::mspriority_queue::make_traits
        metafunction instead of \p Traits template argument.
        Template argument list \p Options of \p %cds::container::mspriority_queue::make_traits metafunction are:
        - opt::buffer - the buffer type for heap array. Possible type are: opt::v::static_buffer, opt::v::dynamic_buffer.
            Default is \p %opt::v::dynamic_buffer.
            At \p %MSPriorityQueue class instantiation, the \p buffer::rebind member metafunction
            is called to change type of values stored in the buffer. So, you may specify any type of values here.
        - opt::compare - priority compare functor. No default functor is provided.
            If the option is not specified, the opt::less is used.
        - opt::less - specifies binary predicate used for priority compare. Default is \p std::less<T>.
        - opt::lock_type - lock type. Default is cds::lock::Spin.
        - opt::back_off - back-off strategy. Default is cds::backoff::yield
        - opt::swap_policy - item swapping policy used to heapify. Default is opt::v::default_swap_policy
            that is a wrapper for <tt> std::swap<T> </tt> algorithm.
        - opt::move_policy - policy for moving item's value. Default is opt::v::assignment_move_policy.
            If the compiler supports move semantics it would be better to specify the move policy
            based on move semantics feature for type \p T.

    \par Usage

    */
    template <typename T, class Traits>
    class MSPriorityQueue: public cds::bounded_container
    {
    public:
        typedef T           value_type  ;   ///< Value type stored in the queue
        typedef Traits      options     ;   ///< Traits template parameter

#   ifdef CDS_DOXYGEN_INVOKED
        typedef implementation_defined key_comparator  ;    ///< priority comparing functor based on opt::compare and opt::less option setter.
#   else
        typedef typename opt::details::make_comparator< value_type, options >::type key_comparator  ;
#   endif

        typedef typename options::lock_type lock_type       ;   ///< heap's size lock type
        typedef typename options::back_off  back_off        ;   ///< Back-off strategy
        typedef typename options::swap_policy   swap_policy ;   ///< Swap policy for type \p T
        typedef typename options::move_policy   move_policy ;   ///< Move policy for type \p T

    protected:
        //@cond
        typedef cds::OS::ThreadId   tag_type    ;

        enum tag_value {
            Available   = -1,
            Empty       = 0
        };

        typedef unsigned char value_placeholder_item[ sizeof(value_type) ] ;
        typedef typename cds::details::aligned_type< value_placeholder_item, alignof( value_type ) >::type value_placeholder ;
        //@endcond

        /// Heap item type
        struct node {
            value_placeholder   m_Value ;   ///< Value placeholder
            tag_type volatile   m_nTag  ;   ///< A tag
            mutable lock_type   m_Lock  ;   ///< Node-level lock

            /// Creates empty node
            node()
                : m_nTag( tag_type(Empty) )
            {}

            /// Get node value
            value_type& value()
            {
                return reinterpret_cast<value_type&>( m_Value ) ;
            }

            /// Get node value
            value_type const& value() const
            {
                return reinterpret_cast< value_type const&>( m_Value ) ;
            }

            /// Returns value placeholder (for placement new)
            void * placeholder()
            {
                return &(m_Value[0]) ;
            }

            /// Destructs value
            void destruct()
            {
                reinterpret_cast<value_type *>( &m_Value )->~value_type() ;
            }

            /// Lock the node
            void lock()
            {
                m_Lock.lock()   ;
            }

            /// Unlock the node
            void unlock()
            {
                m_Lock.unlock() ;
            }
        };

    protected:
        //@cond
#   ifndef CDS_CXX11_LAMBDA_SUPPORT
        struct empty_cleaner
        {
            void operator()( value_type const& ) const
            {}
        };

        template <typename Func>
        struct clear_wrapper
        {
            Func& func ;
            clear_wrapper( Func f ): func(f) {}

            void operator()( int, value_type& src ) const
            {
                func( src ) ;
            }
        };
#   endif
        //@endcond

    public:
        typedef typename options::buffer::template rebind<node>::other   buffer_type ;   ///< Heap array buffer type

        //@cond
        typedef cds::bitop::bit_reverse_counter<>           item_counter_type   ;
        typedef typename item_counter_type::counter_type    counter_type        ;
        //@endcond

    protected:
        item_counter_type   m_ItemCounter   ;   ///< Item counter
        mutable lock_type   m_Lock          ;   ///< Heap's size lock
        buffer_type         m_Heap          ;   ///< Heap array

    public:
        /// Constructs empty priority queue
        /**
            For cds::opt::v::static_buffer the \p nCapacity parameter is ignored.
        */
        MSPriorityQueue( size_t nCapacity )
            : m_Heap( nCapacity )
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
            Otherwise, the function inserts the copy of \p val into the heap
            and returns \p true.

            The function use copy constructor to create new heap item from \p val.
        */
        bool push( value_type const& val )
        {
            tag_type const curId = cds::OS::getCurrentThreadId()  ;

            // Insert new item at bottom of the heap
            m_Lock.lock()   ;
            if ( m_ItemCounter.value() >= capacity() ) {
                m_Lock.unlock() ;
                return false    ;
            }

            counter_type i = m_ItemCounter.inc()    ;
            assert( i < m_Heap.capacity() ) ;

            node& refNode = m_Heap[i]   ;
            refNode.lock()  ;
            m_Lock.unlock() ;
            new ( refNode.placeholder() ) value_type( val )     ;
            refNode.m_nTag = curId  ;
            refNode.unlock()        ;

            // Move item towards top of the heap while it has higher priority than parent
            heapify_after_push( i, curId ) ;

            return true ;
        }

#ifdef CDS_EMPLACE_SUPPORT
        /// Inserts a item into priority queue
        /**
            If the priority queue is full, the function returns \p false,
            no item has been added.
            Otherwise, the function inserts a new item created from \p args arguments
            into the heap and returns \p true.

            The function is available only for compilers supporting variable template
            and move semantics C++11 feature.
        */
        template <typename... Args>
        bool emplace( Args&&... args )
        {
            tag_type const curId = cds::OS::getCurrentThreadId()  ;

            // Insert new item at bottom of the heap
            m_Lock.lock()   ;
            if ( m_ItemCounter.value() >= capacity() ) {
                m_Lock.unlock() ;
                return false    ;
            }

            counter_type i = m_ItemCounter.inc()    ;
            assert( i < m_Heap.capacity() ) ;

            node& refNode = m_Heap[i]   ;
            refNode.lock()  ;
            m_Lock.unlock() ;
            new ( refNode.placeholder() ) value_type( std::forward<Args>(args)... )     ;
            refNode.m_nTag = curId  ;
            refNode.unlock()        ;

            // Move item towards top of the heap while it has higher priority than parent
            heapify_after_push( i, curId ) ;

            return true ;
        }
#endif

        /// Extracts item with high priority
        /**
            If the priority queue is empty, the function returns \p false.
            Otherwise, it returns \p true and \p dest contains the copy of extracted item.
            The item is deleted from the heap.

            The function uses \ref move_policy to move extracted value from the heap's top
            to \p dest.
        */
        bool pop( value_type& dest )
        {
            return pop_with( dest, move_policy() ) ;
        }

        /// Extracts item with high priority
        /**
            If the priority queue is empty, the function returns \p false.
            Otherwise, it returns \p true and \p dest contains the copy of extracted item.
            The item is deleted from the heap.

            The function uses \p MoveFunc \p f to move extracted value from the heap's top
            to \p dest. The interface of \p MoveFunc is:
            \code
            struct move_funcor {
                void operator()( Q& dest, T& src ) ;
            };
            \endcode
        */
        template <typename Q, typename MoveFunc>
        bool pop_with( Q& dest, MoveFunc f )
        {
            m_Lock.lock()   ;
            if ( m_ItemCounter.value() == 0 ) {
                m_Lock.unlock() ;
                return false    ;
            }
            counter_type nBottom = m_ItemCounter.reversed_value() ;
            assert( nBottom <= capacity() ) ;
            m_ItemCounter.dec() ;

            node& refBottom = m_Heap[ nBottom ]   ;
            refBottom.lock()  ;
            m_Lock.unlock() ;

            refBottom.m_nTag = tag_type(Empty)    ;

            if ( nBottom == 1 ) {
                // The heap becomes empty
                f( dest, refBottom.value() ) ;
                refBottom.destruct()    ;
                refBottom.unlock()      ;
                return true             ;
            }

            node& refTop = m_Heap[ 1 ]  ;
            refTop.lock() ;
            f( dest, refTop.value() )   ;

            if ( refTop.m_nTag == tag_type(Empty) ) {
                refBottom.destruct()    ;
                refBottom.unlock()      ;
                refTop.unlock()         ;
                return true             ;
            }

            move_policy()( refTop.value(), refBottom.value() ) ;
            refBottom.destruct()    ;
            refBottom.unlock()      ;
            refTop.m_nTag = tag_type(Available)     ;

            // refTop will be unlocked inside heapify_after_pop
            heapify_after_pop( 1, &refTop ) ;

            return true ;
        }

        /// Clears the queue (not atomic)
        /**
            This function is no atomic, but thread-safe
        */
        void clear()
        {
#       ifdef CDS_CXX11_LAMBDA_SUPPORT
            clear_with( []( value_type const& ) {} ) ;
#       else
            clear_with( empty_cleaner() ) ;
#       endif
        }

        /// Clears the queue (not atomic)
        /**
            This function is no atomic, but thread-safe.

            For each item removed the functor \p f is called.
            \p Func interface is:
            \code
                struct clear_functor
                {
                    void operator()( value_type& item ) ;
                };
            \endcode
            A lambda function or a function pointer can be used as \p f.
        */
        template <typename Func>
        void clear_with( Func f )
        {
            int dummy ;
            while ( !empty() ) {
#       ifdef CDS_CXX11_LAMBDA_SUPPORT
                pop_with( dummy, [&f]( int, value_type& src ) { f( src ); } ) ;
#       else
                pop_with( dummy, clear_wrapper<Func>(f) ) ;
#       endif
            }
        }

        /// Checks is the priority queue is empty
        bool empty() const
        {
            return size() == 0  ;
        }

        /// Checks if the priority queue is full
        bool full() const
        {
            return size() == capacity() ;
        }

        /// Returns current size of priority queue
        size_t size() const
        {
            m_Lock.lock()   ;
            size_t nSize = (size_t) m_ItemCounter.value()    ;
            m_Lock.unlock() ;
            return nSize    ;
        }

        /// Return capacity of the priority queue
        size_t capacity() const
        {
            // m_Heap[0] is not used
            return m_Heap.capacity() - 1 ;
        }

    protected:
        //@cond

        void heapify_after_push( counter_type i, tag_type curId )
        {
            key_comparator  cmp     ;
            back_off        bkoff   ;

            // Move item towards top of the heap while it has higher priority than parent
            while ( i > 1 ) {
                bool bProgress = true ;
                counter_type nParent = i / 2 ;
                node& refParent = m_Heap[nParent]   ;
                node& refItem = m_Heap[i]           ;
                refParent.lock()    ;
                refItem.lock()      ;

                if ( refParent.m_nTag == tag_type(Available) && refItem.m_nTag == curId ) {
                    if ( cmp( refItem.value(), refParent.value() ) > 0 ) {
                        std::swap( refItem.m_nTag, refParent.m_nTag ) ;
                        swap_policy()( refItem.value(), refParent.value() ) ;
                        i = nParent ;
                    }
                    else {
                        refItem.m_nTag = tag_type(Available)    ;
                        i = 0   ;
                    }
                }
                else if ( refParent.m_nTag == tag_type(Empty) )
                    i = 0 ;
                else if ( refItem.m_nTag != curId )
                    i = nParent ;
                else
                    bProgress = false ;

                refItem.unlock()    ;
                refParent.unlock()  ;

                if ( !bProgress )
                    bkoff() ;
                else
                    bkoff.reset()   ;
            }

            if ( i == 1 ) {
                node& refItem = m_Heap[i]   ;
                refItem.lock()      ;
                if ( refItem.m_nTag == curId )
                    refItem.m_nTag = tag_type(Available) ;
                refItem.unlock()    ;
            }
        }

        void heapify_after_pop( counter_type nParent, node * pParent )
        {
            key_comparator cmp      ;

            while ( nParent < m_Heap.capacity() / 2 ) {
                counter_type nLeft = nParent * 2      ;
                counter_type nRight = nLeft + 1 ;
                node& refLeft = m_Heap[nLeft]   ;
                node& refRight = m_Heap[nRight] ;
                refLeft.lock()  ;
                refRight.lock() ;

                counter_type nChild ;
                node * pChild       ;
                if ( refLeft.m_nTag == tag_type(Empty) ) {
                    refRight.unlock()   ;
                    refLeft.unlock()    ;
                    break;
                }
                else if ( refRight.m_nTag == tag_type(Empty) || cmp( refLeft.value(), refRight.value() ) > 0 ) {
                    refRight.unlock()   ;
                    nChild = nLeft      ;
                    pChild = &refLeft   ;
                }
                else {
                    refLeft.unlock()    ;
                    nChild = nRight     ;
                    pChild = &refRight  ;
                }

                // If child has higher priority that parent then swap
                // Otherwise stop
                if ( cmp( pChild->value(), pParent->value() ) > 0 ) {
                    std::swap( pParent->m_nTag, pChild->m_nTag ) ;
                    swap_policy()( pParent->value(), pChild->value() ) ;
                    pParent->unlock()   ;
                    nParent = nChild    ;
                    pParent = pChild    ;
                }
                else {
                    pChild->unlock()    ;
                    break;
                }
            }
            pParent->unlock()   ;
        }
        //@endcond
    };

}} // namespace cds::container

#endif // #ifndef __CDS_CONTAINER_MSPRIORITY_QUEUE_H
