//$$CDS-header$$

#ifndef __CDS_CONTAINER_TREIBER_STACK_H
#define __CDS_CONTAINER_TREIBER_STACK_H

#include <cds/intrusive/treiber_stack.h>
#include <cds/container/base.h>
#include <cds/details/std/memory.h>

namespace cds { namespace container {

    //@cond
    namespace details {

        template <typename GC, typename T, CDS_DECL_OPTIONS6>
        struct make_treiber_stack
        {
            typedef T value_type    ;

            struct default_options {
                typedef cds::backoff::empty         back_off    ;
                typedef CDS_DEFAULT_ALLOCATOR       allocator   ;
                typedef cds::opt::v::relaxed_ordering       memory_model;
                typedef cds::atomicity::empty_item_counter  item_counter;
                typedef cds::intrusive::stack_dummy_stat    stat        ;
            };

            typedef typename cds::opt::make_options<
                typename cds::opt::find_type_traits< default_options, CDS_OPTIONS6 >::type
                ,CDS_OPTIONS6
            >::type   options ;

            typedef GC gc  ;
            typedef typename options::memory_model memory_model;

            struct node_type: public cds::intrusive::single_link::node< gc >
            {
                value_type  m_value ;

                node_type( const value_type& val )
                    : m_value( val )
                {}
#           ifdef CDS_EMPLACE_SUPPORT
                template <typename... Args>
                node_type( Args&&... args )
                    : m_value( std::forward<Args>(args)...)
                {}
#           else
                node_type()
                {}
#           endif
            };

            typedef typename options::allocator::template rebind<node_type>::other allocator_type   ;
            typedef cds::details::Allocator< node_type, allocator_type >           cxx_allocator    ;

            struct node_deallocator
            {
                void operator ()( node_type * pNode )
                {
                    cxx_allocator().Delete( pNode ) ;
                }
            }   ;

            typedef intrusive::TreiberStack<
                gc
                ,node_type
                ,intrusive::opt::hook<
                    intrusive::single_link::base_hook< cds::opt::gc<gc> >
                >
                ,cds::opt::back_off< typename options::back_off >
                ,cds::intrusive::opt::disposer< node_deallocator >
                ,cds::opt::memory_model< memory_model >
                ,cds::opt::item_counter< typename options::item_counter >
                ,cds::opt::stat< typename options::stat >
            >   type    ;
        };
    } // namespace details
    //@endcond

    /// Treiber's stack algorithm
    /** @ingroup cds_nonintrusive_stack
        It is non-intrusive version of Treiber's stack algorithm based on intrusive implementation
        intrusive::TreiberStack.

        Template arguments:
        - \p GC - garbage collector type: gc::HP, gc::HRC, gc::PTB
        - \p T - type stored in the stack. It should be default-constructible, copy-constructible, assignable type.
        - \p Options - options

        Available \p Options:
        - opt::allocator - allocator (like \p std::allocator). Default is \ref CDS_DEFAULT_ALLOCATOR
        - opt::back_off - back-off strategy used. If the option is not specified, the cds::backoff::empty is used
        - opt::memory_model - C++ memory ordering model. Can be opt::v::relaxed_ordering (relaxed memory model, the default)
            or opt::v::sequential_consistent (sequentially consisnent memory model).
        - opt::item_counter - the type of item counting feature. Default is \ref atomicity::empty_item_counter
        - opt::stat - the type to gather internal statistics.
            Possible option value are: \ref intrusive::stack_stat, \ref intrusive::stack_dummy_stat, user-provided class that supports
            intrusive::stack_stat interface. Default is \ref intrusive::stack_dummy_stat.
    */
    template < typename GC, typename T, CDS_DECL_OPTIONS6 >
    class TreiberStack
        : public
#ifdef CDS_DOXYGEN_INVOKED
        intrusive::TreiberStack< GC, cds::intrusive::single_link::node< T >, Options... >
#else
        details::make_treiber_stack< GC, T, CDS_OPTIONS6 >::type
#endif
    {
        //@cond
        typedef details::make_treiber_stack< GC, T, CDS_OPTIONS6 > options   ;
        typedef typename options::type base_class ;
        //@endcond

    public:
        /// Rebind template arguments
        template <typename GC2, typename T2, CDS_DECL_OTHER_OPTIONS6>
        struct rebind {
            typedef TreiberStack< GC2, T2, CDS_OTHER_OPTIONS6> other   ;   ///< Rebinding result
        };

    public:
        typedef T value_type ; ///< Value type stored in the stack
        typedef typename base_class::gc gc                      ;   ///< Garbage collector used
        typedef typename base_class::back_off  back_off         ;   ///< Back-off strategy used
        typedef typename options::allocator_type allocator_type ;   ///< Allocator type used for allocate/deallocate the nodes
        typedef typename options::memory_model  memory_model    ;   ///< Memory ordering. See cds::opt::memory_order option
        typedef typename base_class::stat       stat            ;   ///< Internal statistics policy used

    protected:
        typedef typename options::node_type  node_type   ;   ///< stack node type (derived from intrusive::single_link::node)

        //@cond
        typedef typename options::cxx_allocator     cxx_allocator   ;
        typedef typename options::node_deallocator  node_deallocator;   // deallocate node
        //@endcond

    protected:
        ///@cond
        static node_type * alloc_node( const value_type& val )
        {
            return cxx_allocator().New( val )    ;
        }
#   ifdef CDS_EMPLACE_SUPPORT
        template <typename... Args>
        static node_type * alloc_node_move( Args&&... args )
        {
            return cxx_allocator().MoveNew( std::forward<Args>( args )... ) ;
        }
#   endif

        static void free_node( node_type * p )
        {
            node_deallocator()( p ) ;
        }
        static void retire_node( node_type * p )
        {
            gc::template retire<typename base_class::disposer>( p )   ;
        }

        struct node_disposer {
            void operator()( node_type * pNode )
            {
                free_node( pNode )  ;
            }
        };
        typedef std::unique_ptr< node_type, node_disposer >     scoped_node_ptr ;
        //@endcond

    public:
        /// Constructs empty stack
        TreiberStack()
        {}

        /// Clears the stack on destruction
        ~TreiberStack()
        {}

        /// Push the item \p val on the stack
        bool push( const value_type& val )
        {
            scoped_node_ptr p( alloc_node(val)) ;
            if ( base_class::push( *p )) {
                p.release() ;
                return true ;
            }
            return false ;
        }

#   ifdef CDS_EMPLACE_SUPPORT
        /// Pushes data of type \ref value_type constructed with <tt>std::forward<Args>(args)...</tt>
        /**
            This function is available only for compiler that supports
            variadic template and move semantics
        */
        template <typename... Args>
        bool emplace( Args&&... args )
        {
            scoped_node_ptr p( alloc_node_move( std::forward<Args>(args)...)) ;
            if ( base_class::push( *p )) {
                p.release() ;
                return true ;
            }
            return false ;
        }
#   endif

        /// Pop an item from the stack
        /**
            The value of popped item is stored in \p val.
            On success functions returns \p true, \p val contains value popped from the stack.
            If stack is empty the function returns \p false, \p val is unchanged.
        */
        bool pop( value_type& val )
        {
            node_type * p = base_class::pop()   ;
            if ( !p )
                return false    ;

            val = p->m_value    ;
            retire_node( p )    ;

            return true ;
        }

        /// Check if stack is empty
        bool empty() const
        {
            return base_class::empty()  ;
        }

        /// Clear the stack
        void clear()
        {
            base_class::clear() ;
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
            return base_class::size() ;
        }

        /// Returns reference to internal statistics
        stat const& statistics() const
        {
            return base_class::statistics() ;
        }

    };


}}  // namespace cds::container


#endif // #ifndef __CDS_CONTAINER_TREIBER_STACK_H
