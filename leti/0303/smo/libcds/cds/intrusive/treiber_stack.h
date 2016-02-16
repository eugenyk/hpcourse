//$$CDS-header$$

#ifndef __CDS_INTRUSIVE_TREIBER_STACK_H
#define __CDS_INTRUSIVE_TREIBER_STACK_H

#include <cds/intrusive/single_link_struct.h>
#include <cds/ref.h>
#include <cds/intrusive/stack_stat.h>

#include <cds/details/std/type_traits.h>

namespace cds { namespace intrusive {

    /// Treiber stack
    /** @ingroup cds_intrusive_stack
        Intrusive implementation of well-known Treiber's stack algorithm:
        - R. K. Treiber. Systems programming: Coping with parallelism. Technical Report RJ 5118, IBM Almaden Research Center, April 1986.

        Template arguments:
        - \p GC - garbage collector type: gc::HP, gc::HRC, gc::PTB
        - \p T - type to be inserted into the stack
        - \p Options - options

        \p Options are:
        - opt::hook - hook used. Possible values are: single_link::base_hook, single_link::member_hook, single_link::traits_hook.
            If the option is not specified, <tt>single_link::base_hook<></tt> is used.
            For Gidenstam's gc::HRC, only single_link::base_hook is supported.
        - opt::back_off - back-off strategy used. If the option is not specified, the cds::backoff::empty is used.
        - opt::disposer - the functor used for dispose removed items. Default is opt::v::empty_disposer. This option is used only
            in \ref clear function.
        - opt::link_checker - the type of node's link fields checking. Default is \ref opt::debug_check_link.
            Note: for gc::HRC garbage collector, link checking policy is always selected as \ref opt::always_check_link.
        - opt::memory_model - C++ memory ordering model. Can be opt::v::relaxed_ordering (relaxed memory model, the default)
            or opt::v::sequential_consistent (sequentially consisnent memory model).
        - opt::item_counter - the type of item counting feature. Default is \ref atomicity::empty_item_counter
        - opt::stat - the type to gather internal statistics.
            Possible option value are: \ref intrusive::stack_stat, \ref intrusive::stack_dummy_stat, user-provided class that supports
            intrusive::stack_stat interface. Default is \ref intrusive::stack_dummy_stat.

        Garbage collecting schema \p GC must be consistent with the single_link::node GC.

        Be careful when you want destroy an item popped, see \ref cds_intrusive_item_destroying "Destroying items of intrusive containers".

        \par Examples

        Example of how to use \p single_link::base_hook.
        Your class that objects will be pushed on TreiberStack should be based on single_link::node class
        \code
        #include <cds/intrusive/stack/treiber_stack.h>
        #include <cds/gc/hp.h>

        namespace ci = cds::intrusive   ;
        typedef cds::gc::HP gc     ;

        struct myData: public ci::single_link::node< gc >
        {
            // ...
        } ;

        // Stack type
        typedef ci::TreiberStack< gc,
            myData,
            ci::opt::hook< ci::single_link::base_hook< gc > >
        > stack_t   ;
        \endcode

        Example of how to use base_hook with different tags.
        \code
        #include <cds/intrusive/stack/treiber_stack.h>
        #include <cds/gc/hp.h>

        namespace ci = cds::intrusive   ;
        typedef cds::gc::HP gc     ;

        // It is not necessary to declare complete type for tags
        struct tag1 ;
        struct tag2 ;

        struct myData
            : public ci::single_link::node< gc, tag1 >
            , public ci::single_link::node< gc, tag2 >
        {
            // ...
        } ;

        typedef ci::TreiberStack< gc, myData, ci::opt::hook< ci::single_link::base_hook< gc, tag1 > > stack1_t   ;
        typedef ci::TreiberStack< gc, myData, ci::opt::hook< ci::single_link::base_hook< gc, tag2 > > stack2_t   ;

        // You may add myData objects in the objects of type stack1_t and stack2_t independently
        void foo() {
            stack1_t    s1  ;
            stack2_t    s2  ;

            myData i1, i2   ;
            s1.push( i1 )   ;
            s2.push( i2 )   ;
            s2.push( i1 )   ;   // i1 is now contained in s1 and s2.

            myData * p      ;

            p = s1.pop()    ;   // pop i1 from s1
            p = s1.pop()    ;   // p == NULL, s1 is empty
            p = s2.pop()    ;   // pop i1 from s2
            p = s2.pop()    ;   // pop i2 from s2
            p = s2.pop()    ;   // p == NULL, s2 is empty
        }
        \endcode

        Example of how to use member_hook.
        Your class that will be pushed on TreiberStack should have a member of type single_link::node
        \code
        #include <cds/intrusive/stack/treiber_stack.h>
        #include <cds/gc/hp.h>
        #include <stddef.h>     // offsetof macro

        namespace ci = cds::intrusive   ;
        typedef cds::gc::HP gc     ;

        struct myData
        {
            // ...
            ci::single_link::node< gc >      member_hook_    ;
            // ...
        } ;

        typedef ci::TreiberStack< gc, myData,
            ci::opt::hook<
                ci::single_link::member_hook< offsetof(myData, member_hook_),
                gc
            >
        > stack_t   ;
        \endcode
    */
    template <typename GC, typename T, CDS_DECL_OPTIONS8>
    class TreiberStack
    {
        //@cond
        struct default_options
        {
            typedef cds::backoff::empty         back_off    ;
            typedef single_link::base_hook<>    hook        ;
            typedef opt::v::empty_disposer      disposer    ;
            typedef atomicity::empty_item_counter   item_counter;
            typedef opt::v::relaxed_ordering    memory_model;
            typedef stack_dummy_stat            stat        ;
            static const opt::link_check_type link_checker = opt::debug_check_link  ;
        };
        //@endcond

    public:
        //@cond
        typedef typename opt::make_options<
            typename cds::opt::find_type_traits< default_options, CDS_OPTIONS8 >::type
            ,CDS_OPTIONS8
        >::type   options ;
        //@endcond

    public:
        /// Rebind template arguments
        template <typename GC2, typename T2, CDS_DECL_OTHER_OPTIONS8>
        struct rebind {
            typedef TreiberStack< GC2, T2, CDS_OTHER_OPTIONS8> other   ;   ///< Rebinding result
        };

    public:
        typedef T  value_type   ;   ///< type of value stored in the stack
        typedef typename options::hook      hook        ;   ///< hook type
        typedef typename hook::node_type    node_type   ;   ///< node type
        typedef typename options::disposer  disposer    ;   ///< disposer used
        typedef typename get_node_traits< value_type, node_type, hook>::type node_traits ;    ///< node traits
        typedef typename single_link::get_link_checker< node_type, options::link_checker >::type link_checker   ;   ///< link checker
        typedef typename options::memory_model  memory_model      ;   ///< Memory ordering. See cds::opt::memory_model option
        typedef typename options::item_counter item_counter ;   ///< Item counting policy used
        typedef typename options::stat      stat        ;   ///< Internal statistics policy used

    public:
        typedef GC  gc          ;   ///< Garbage collector
        typedef typename options::back_off  back_off    ;   ///< back-off strategy

    protected:
        typename node_type::atomic_node_ptr m_Top       ;   ///< Top of the stack
        item_counter        m_ItemCounter   ;   ///< Item counter
        stat                m_stat          ;   ///< Internal statistics

        //@cond
        typedef intrusive::node_to_value<TreiberStack> node_to_value    ;
        //@endcond

    protected:
        //@cond
        void clear_links( node_type * pNode ) CDS_NOEXCEPT
        {
            pNode->m_pNext.store( null_ptr<node_type *>(), memory_model::memory_order_relaxed ) ;
        }
        //@endcond

    public:
        /// Constructs empty stack
        TreiberStack()
            : m_Top(null_ptr<node_type *>())
        {
            // GC and node_type::gc must be the same
            static_assert(( std::is_same<gc, typename node_type::gc>::value ), "GC and node_type::gc must be the same")    ;

            // For cds::gc::HRC, only base_hook is allowed
            static_assert((
                std::conditional<
                    std::is_same<gc, cds::gc::HRC>::value,
                    std::is_same< typename hook::hook_type, opt::base_hook_tag >,
                    boost::true_type
                >::type::value
            ), "For cds::gc::HRC, only base_hook is allowed") ;
        }

        /// Destructor calls \ref cds_intrusive_TreiberStack_clear "clear" member function
        ~TreiberStack()
        {
            clear() ;
        }

        /// Push the item \p val on the stack
        /**
            No copying is made since it is intrusive stack.
        */
        bool push( value_type& val )
        {
            node_type * pNew = node_traits::to_node_ptr( val )  ;
            link_checker::is_empty( pNew ) ;
            back_off bkoff ;

            m_stat.onPush() ;

            node_type * t = m_Top.load(memory_model::memory_order_relaxed) ;
            while ( true ) {
                pNew->m_pNext.store( t, memory_model::memory_order_relaxed )  ;
                if ( m_Top.compare_exchange_weak( t, pNew, memory_model::memory_order_release, CDS_ATOMIC::memory_order_relaxed )) {     // #1 sync-with #2
                    ++m_ItemCounter ;
                    return true     ;
                }
                m_stat.onPushRace() ;
                bkoff()    ;
            }
        }

        /// Pop an item from the stack
        /**
            If stack is empty, returns \p NULL.
            The disposer is <b>not</b> called for popped item.
            See \ref cds_intrusive_item_destroying "Destroying items of intrusive containers".
        */
        value_type * pop()
        {
            back_off bkoff ;
            typename gc::Guard  guard   ;

            m_stat.onPop()  ;

            while ( true ) {
                node_type * t = guard.protect( m_Top, node_to_value() ) ;
                if ( t == null_ptr<node_type *>() )
                    return null_ptr<value_type *>() ;    // stack is empty

                node_type * pNext = t->m_pNext.load(memory_model::memory_order_relaxed)  ;
                if ( m_Top.compare_exchange_weak( t, pNext, memory_model::memory_order_acquire, CDS_ATOMIC::memory_order_relaxed )) {              // #2
                    clear_links( t )    ;
                    --m_ItemCounter     ;
                    return node_traits::to_value_ptr( *t )   ;
                }

                m_stat.onPopRace()  ;
                bkoff()             ;
            }
        }

        /// Check if stack is empty
        bool empty() const
        {
            /*
                http://www.manning-sandbox.com/thread.jspa?threadID=46245&tstart=0

                My question to Anthony Williams:

                Suppose, we want to add function empty() for stack class:
                bool empty() const
                {
                    return head.load() == NULL ;
                }

                What is right memory ordering for empty()?..
                The solution:
                bool empty() const
                {
                    // sync with pop CAS
                    std::atomic_thread_fence<std::memory_order_release>() ;

                    // sync with push CAS
                    return head.load(std::memory_order_acquire) == NULL ;
                }

                is right or very naive?.
                Is there a better empty() implementation?

                A.Williams:
                empty() can just be implemented as a check of head, with memory_order_relaxed.
                This matches the load(memory_order_relaxed) for the initial check for an empty queue in pop().
                You won't see changes immediately, but we don't want to force serialization.
                Incidentally, the release fence in your sample code won't do anything useful.
                A release fence provides synchronization if there is a subsequent store that is loaded by a load-acquire,
                or a load followed by an acquire fence. Since your code only has a load following the release fence,
                this doesn't help.
                If you want guaranteed visibility, you can use an RMW operation.
                In this case, compare_exchange_strong() checking for NULL and replacing it with NULL would do,
                but this will add overhead by forcing the visibility.
                It is generally better use a relaxed load, let the processors propagate
                the changes in their own time, and do something else whilst you're waiting.
            */
            return m_Top.load(memory_model::memory_order_relaxed) == null_ptr<node_type *>()    ;
        }

        /// Clear the stack
        /** @anchor cds_intrusive_TreiberStack_clear
            For each removed item the disposer is called.

            <b>Caution</b>
            It is possible that after <tt>clear()</tt> the <tt>empty()</tt> returns \p false
            if some other thread pushes an item into the stack during \p clear works
        */
        void clear()
        {
            back_off bkoff ;
            typename gc::Guard guard    ;
            node_type * pTop    ;
            while ( true ) {
                pTop = guard.protect( m_Top, node_to_value() )   ;
                if ( pTop == null_ptr<node_type *>() )
                    return  ;
                if ( m_Top.compare_exchange_weak( pTop, null_ptr<node_type *>(), memory_model::memory_order_acq_rel, CDS_ATOMIC::memory_order_relaxed )) {    // sync-with #1 and #2
                    m_ItemCounter.reset() ;
                    break   ;
                }
                bkoff()     ;
            }

            while( pTop ) {
                node_type * p = pTop    ;
                pTop = p->m_pNext.load(memory_model::memory_order_relaxed)   ;
                clear_links( p )  ;
                gc::template retire<disposer>( node_traits::to_value_ptr( *p ) )  ;
            }
        }

        /// Returns stack's item count
        /**
            The value returned depends on opt::item_counter option. For atomicity::empty_item_counter,
            this function always returns 0.

            <b>Warning</b>: even if you use real item counter and it returns 0, this fact is not mean that the stack
            is empty. To check emptyness use \ref empty() method.
        */
        size_t    size() const
        {
            return m_ItemCounter.value()    ;
        }

        /// Returns reference to internal statistics
        stat const& statistics() const
        {
            return m_stat ;
        }

    };

}} // namespace cds::intrusive

#endif  // #ifndef __CDS_INTRUSIVE_TREIBER_STACK_H
