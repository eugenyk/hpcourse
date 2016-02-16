//$$CDS-header$$

#ifndef __CDS_INTRUSIVE_ELLEN_BINTREE_IMPL_H
#define __CDS_INTRUSIVE_ELLEN_BINTREE_IMPL_H

#include <cds/intrusive/details/ellen_bintree_base.h>
#include <cds/opt/compare.h>
#include <cds/ref.h>
#include <cds/details/binary_functor_wrapper.h>
#include <cds/urcu/details/check_deadlock.h>
#include <cds/details/std/memory.h>

namespace cds { namespace intrusive {

    /// Ellen's et al binary search tree
    /** @ingroup cds_intrusive_map
        @ingroup cds_intrusive_tree
        @anchor cds_intrusive_EllenBinTree

        Source:
            - [2010] F.Ellen, P.Fatourou, E.Ruppert, F.van Breugel "Non-blocking Binary Search Tree"

        %EllenBinTree is an unbalanced leaf-oriented binary search tree that implements the <i>set</i>
        abstract data type. Nodes maintains child pointers but not parent pointers.
        Every internal node has exactly two children, and all data of type \p T currently in
        the tree are stored in the leaves. Internal nodes of the tree are used to direct \p find
        operation along the path to the correct leaf. The keys (of \p Key type) stored in internal nodes
        may or may not be in the set. \p Key type is a subset of \p T type.
        There should be exactly defined a key extracting functor for converting object of type \p T to
        object of type \p Key.

        Due to \p extract_min and \p extract_max member functions the \p %EllenBinTree can act as
        a <i>priority queue</i>. In this case you should provide unique compound key, for example,
        the priority value plus some uniformly distributed random value.

        @note In the current implementation we do not use helping technique described in original paper.
        So, the current implementation is near to fine-grained lock-based tree.
        Helping will be implemented in future release

        @warning Recall the tree is <b>unbalanced</b>. The complexity of operations is <tt>O(log N)</tt>
        for uniformly distributed random keys, but in worst case the complexity is <tt>O(N)</tt>.

        @note Do not include <tt><cds/intrusive/ellen_bintree_impl.h></tt> header file explicitly.
        There are header file for each GC type:
        - <tt><cds/intrusive/ellen_bintree_hp.h></tt> - for Hazard Pointer GC cds::gc::HP
        - <tt><cds/intrusive/ellen_bintree_ptb.h></tt> - for Pass-the-Buck GC cds::gc::PTB
        - <tt><cds/intrusive/ellen_bintree_rcu.h></tt> - for RCU GC
            (see \ref cds_intrusive_EllenBinTree_rcu "RCU-based EllenBinTree")

        <b>Template arguments</b> :
        - \p GC - garbage collector used, possible types are cds::gc::HP, cds::gc::PTB.
            Note that cds::gc::HRC is not supported.
        - \p Key - key type, a subset of \p T
        - \p T - type to be stored in tree's leaf nodes. The type must be based on ellen_bintree::node
            (for ellen_bintree::base_hook) or it must have a member of type ellen_bintree::node
            (for ellen_bintree::member_hook).
        - \p Traits - type traits. See ellen_bintree::type_traits for explanation.

        It is possible to declare option-based tree with cds::intrusive::ellen_bintree::make_traits metafunction
        instead of \p Traits template argument.
        Template argument list \p Options of cds::intrusive::ellen_bintree::make_traits metafunction are:
        - opt::hook - hook used. Possible values are: ellen_bintree::base_hook, ellen_bintree::member_hook, ellen_bintree::traits_hook.
            If the option is not specified, <tt>ellen_bintree::base_hook<></tt> is used.
        - ellen_bintree::key_extractor - key extracting functor, mandatory option. The functor has the following prototype:
            \code
                struct key_extractor {
                    void operator ()( Key& dest, T const& src ) ;
                };
            \endcode
            It should initialize \p dest key from \p src data. The functor is used to initialize internal nodes.
        - opt::compare - key compare functor. No default functor is provided.
            If the option is not specified, \p %opt::less is used.
        - opt::less - specifies binary predicate used for key compare. At least \p %opt::compare or \p %opt::less should be defined.
        - opt::disposer - the functor used for dispose removed nodes. Default is opt::v::empty_disposer. Due the nature
            of GC schema the disposer may be called asynchronously. The disposer is used only for leaf nodes.
        - opt::item_counter - the type of item counting feature. Default is \ref atomicity::empty_item_counter that means no item counting.
        - opt::memory_model - C++ memory ordering model. Can be opt::v::relaxed_ordering (relaxed memory model, the default)
            or opt::v::sequential_consistent (sequentially consisnent memory model).
        - ellen_bintree::update_desc_allocator - an allocator of \ref ellen_bintree::update_desc "update descriptors",
            default is CDS_DEFAULT_ALLOCATOR.
            Note that update descriptor is helping data structure with short lifetime and it is good candidate for pooling.
            The number of simultaneously existing descriptors is bounded and depends on the number of threads
            working with the tree and GC intenals.
            A bounded lock-free container like \p cds::container::VyukovMPMCCycleQueue is good candidate
            for the free-list of update descriptors, see cds::memory::vyukov_queue_pool free-list implementation.
            Also notice that size of update descriptor is constant and not dependent on the type of data
            stored in the tree so single free-list object can be used for all \p %EllenBinTree objects.
        - opt::node_allocator - the allocator used for internal nodes. Default is \ref CDS_DEFAULT_ALLOCATOR.
        - opt::stat - internal statistics. Available types: ellen_bintree::stat, ellen_bintree::empty_stat (the default)

        @anchor cds_intrusive_EllenBinTree_less
        <b>Predicate requirements</b>

        opt::less, opt::compare and other predicates using with member fuctions should accept at least parameters
        of type \p T and \p Key in any combination.
        For example, for \p Foo struct with \p std::string key field the appropiate \p less functor is:
        \code
        struct Foo: public cds::intrusive::ellen_bintree::node< ... >
        {
            std::string m_strKey ;
            ...
        } ;

        struct less {
            bool operator()( Foo const& v1, Foo const& v2 ) const
            { return v1.m_strKey < v2.m_strKey ; }

            bool operator()( Foo const& v, std::string const& s ) const
            { return v.m_strKey < s ; }

            bool operator()( std::string const& s, Foo const& v ) const
            { return s < v.m_strKey ; }

            // Support comparing std::string and char const *
            bool operator()( std::string const& s, char const * p ) const
            { return s.compare(p) < 0 ; }

            bool operator()( Foo const& v, char const * p ) const
            { return v.m_strKey.compare(p) < 0 ; }

            bool operator()( char const * p, std::string const& s ) const
            { return s.compare(p) > 0; }

            bool operator()( char const * p, Foo const& v ) const
            { return v.m_strKey.compare(p) > 0; }
        };
        \endcode
    */
    template < class GC,
        typename Key,
        typename T,
#ifdef CDS_DOXYGEN_INVOKED
        class Traits = ellen_bintree::type_traits
#else
        class Traits
#endif
    >
    class EllenBinTree
    {
    public:
        typedef GC      gc              ;   ///< Garbage collector used
        typedef Key     key_type        ;   ///< type of a key stored in internal nodes; key is a part of \p value_type
        typedef T       value_type      ;   ///< type of value stored in the binary tree
        typedef Traits  options         ;   ///< Traits template parameter

        typedef typename options::hook      hook        ;   ///< hook type
        typedef typename hook::node_type    node_type   ;   ///< node type
        typedef typename options::disposer  disposer    ;   ///< leaf node disposer

    protected:
        //@cond
        typedef ellen_bintree::base_node< gc >                      tree_node       ; ///< Base type of tree node
        typedef node_type                                           leaf_node       ; ///< Leaf node type
        typedef ellen_bintree::internal_node< key_type, leaf_node > internal_node   ; ///< Internal node type
        typedef ellen_bintree::update_desc< leaf_node, internal_node> update_desc   ; ///< Update descriptor
        typedef typename update_desc::update_ptr                    update_ptr      ; ///< Marked pointer to update descriptor
        //@endcond

    public:
#   ifdef CDS_DOXYGEN_INVOKED
        typedef implementation_defined key_comparator  ;    ///< key compare functor based on opt::compare and opt::less option setter.
        typedef typename get_node_traits< value_type, node_type, hook>::type node_traits ; ///< Node traits
#   else
        typedef typename opt::details::make_comparator< value_type, options >::type key_comparator ;
        struct node_traits: public get_node_traits< value_type, node_type, hook>::type
        {
            static internal_node const& to_internal_node( tree_node const& n )
            {
                assert( n.is_internal() ) ;
                return static_cast<internal_node const&>( n ) ;
            }

            static leaf_node const& to_leaf_node( tree_node const& n )
            {
                assert( n.is_leaf() ) ;
                return static_cast<leaf_node const&>( n ) ;
            }
        };
#   endif

        typedef typename options::item_counter  item_counter;   ///< Item counting policy used
        typedef typename options::memory_model  memory_model;   ///< Memory ordering. See cds::opt::memory_model option
        typedef typename options::stat          stat        ;   ///< internal statistics type
        typedef typename options::key_extractor         key_extractor   ;   ///< key extracting functor

        typedef typename options::node_allocator        node_allocator  ;   ///< Internal node allocator
        typedef typename options::update_desc_allocator update_desc_allocator ; ///< Update descriptor allocator

    protected:
        //@cond
        typedef ellen_bintree::details::compare< key_type, value_type, key_comparator, node_traits > node_compare ;

        typedef cds::details::Allocator< internal_node, node_allocator >        cxx_node_allocator   ;
        typedef cds::details::Allocator< update_desc, update_desc_allocator >   cxx_update_desc_allocator   ;

        struct search_result {
            typedef typename gc::template GuardArray<5> guard_array ;
            guard_array guards  ;

            enum guard_index {
                Guard_GrandParent,
                Guard_Parent,
                Guard_Leaf,
                Guard_updGrandParent,
                Guard_updParent
            };

            internal_node *     pGrandParent    ;
            internal_node *     pParent         ;
            leaf_node *         pLeaf           ;
            update_ptr          updParent       ;
            update_ptr          updGrandParent  ;
            bool                bRightLeaf      ; // true if pLeaf is right child of pParent, false otherwise
            bool                bRightParent    ; // true if pParent is right child of pGrandParent, false otherwise

            search_result()
                :pGrandParent( null_ptr<internal_node *>() )
                ,pParent( null_ptr<internal_node *>() )
                ,pLeaf( null_ptr<leaf_node *>() )
                ,bRightLeaf( false )
                ,bRightParent( false )
            {}
        };
        //@endcond

    protected:
        //@cond
        internal_node       m_Root          ;   ///< Tree root node (key= Infinite2)
        leaf_node           m_LeafInf1      ;   ///< Infinite leaf 1 (key= Infinite1)
        leaf_node           m_LeafInf2      ;   ///< Infinite leaf 2 (key= Infinite2)
        //@endcond

        item_counter        m_ItemCounter   ;   ///< item counter
        mutable stat        m_Stat          ;   ///< internal statistics

    protected:
        //@cond
        static void free_leaf_node( value_type * p )
        {
            disposer()( p ) ;
        }

        internal_node * alloc_internal_node() const
        {
            m_Stat.onInternalNodeCreated() ;
            internal_node * pNode = cxx_node_allocator().New() ;
            return pNode    ;
        }

        static void free_internal_node( internal_node * pNode )
        {
            cxx_node_allocator().Delete( pNode ) ;
        }

        struct internal_node_deleter {
            void operator()( internal_node * p) const
            {
                free_internal_node( p ) ;
            }
        };

        typedef std::unique_ptr< internal_node, internal_node_deleter>  unique_internal_node_ptr ;

        update_desc * alloc_update_desc() const
        {
            m_Stat.onUpdateDescCreated()    ;
            return cxx_update_desc_allocator().New() ;
        }

        static void free_update_desc( update_desc * pDesc )
        {
            cxx_update_desc_allocator().Delete( pDesc ) ;
        }

        void retire_node( tree_node * pNode ) const
        {
            if ( pNode->is_leaf() ) {
                assert( static_cast<leaf_node *>( pNode ) != &m_LeafInf1 ) ;
                assert( static_cast<leaf_node *>( pNode ) != &m_LeafInf2 ) ;

                gc::template retire( node_traits::to_value_ptr( static_cast<leaf_node *>( pNode )), free_leaf_node )   ;
            }
            else {
                assert( static_cast<internal_node *>( pNode ) != &m_Root ) ;
                m_Stat.onInternalNodeDeleted() ;

                gc::template retire( static_cast<internal_node *>( pNode ), free_internal_node )   ;
            }
        }

        void retire_update_desc( update_desc * p ) const
        {
            m_Stat.onUpdateDescDeleted() ;
            gc::template retire( p, free_update_desc )   ;
        }

        void make_empty_tree()
        {
            m_Root.infinite_key( 2 )    ;
            m_LeafInf1.infinite_key( 1 ) ;
            m_LeafInf2.infinite_key( 2 ) ;
            m_Root.m_pLeft.store( &m_LeafInf1, memory_model::memory_order_relaxed ) ;
            m_Root.m_pRight.store( &m_LeafInf2, memory_model::memory_order_release );
        }

#   ifndef CDS_CXX11_LAMBDA_SUPPORT
        struct trivial_equal_functor {
            template <typename Q>
            bool operator()( Q const& , leaf_node const& ) const
            {
                return true ;
            }
        };

        struct empty_insert_functor {
            void operator()( value_type& )
            {}
        };

#   endif

#   if !defined(CDS_CXX11_LAMBDA_SUPPORT) || (CDS_COMPILER == CDS_COMPILER_MSVC && CDS_COMPILER_VERSION == CDS_COMPILER_MSVC10)
        struct unlink_equal_functor {
            bool operator()( value_type const& v, leaf_node const& n ) const
            {
                return &v == node_traits::to_value_ptr( n ) ;
            }
        };
        struct empty_erase_functor  {
            void operator()( value_type const& )
            {}
        };
#   endif
        //@endcond

    public:
        /// Default constructor
        EllenBinTree()
        {
            static_assert( (!std::is_same< key_extractor, opt::none >::value), "The key extractor option must be specified" ) ;
            make_empty_tree()   ;
        }

        /// Clears the tree
        ~EllenBinTree()
        {
            clear() ;
        }

        /// Inserts new node
        /**
            The function inserts \p val in the tree if it does not contain
            an item with key equal to \p val.

            Returns \p true if \p val is placed into the set, \p false otherwise.
        */
        bool insert( value_type& val )
        {
#   ifdef CDS_CXX11_LAMBDA_SUPPORT
            return insert( val, []( value_type& ) {} ) ;
#   else
            return insert( val, empty_insert_functor() ) ;
#   endif
        }

        /// Inserts new node
        /**
            This function is intended for derived non-intrusive containers.

            The function allows to split creating of new item into two part:
            - create item with key only
            - insert new item into the tree
            - if inserting is success, calls  \p f functor to initialize value-field of \p val.

            The functor signature is:
            \code
                void func( value_type& val ) ;
            \endcode
            where \p val is the item inserted. User-defined functor \p f should guarantee that during changing
            \p val no any other changes could be made on this tree's item by concurrent threads.
            The user-defined functor is called only if the inserting is success and may be passed by reference
            using <tt>boost::ref</tt>
        */
        template <typename Func>
        bool insert( value_type& val, Func f )
        {
            unique_internal_node_ptr pNewInternal ;

            search_result res ;
            for ( ;; ) {
                if ( search( res, val, node_compare() )) {
                    if ( pNewInternal.get() )
                        m_Stat.onInternalNodeDeleted() ;    // unique_internal_node_ptr deletes internal node
                    m_Stat.onInsertFailed() ;
                    return false    ;
                }

                if ( res.updParent.bits() != update_desc::Clean )
                    help( res.updParent ) ;
                else {

                    if ( !pNewInternal.get() )
                        pNewInternal.reset( alloc_internal_node() );

                    if ( try_insert( val, pNewInternal.get(), res )) {
                        cds::unref(f)( val )    ;
                        pNewInternal.release()  ;   // internal node is linked into the tree and should not be deleted
                        break;
                    }
                }

                m_Stat.onInsertRetry()  ;
            }

            ++m_ItemCounter ;
            m_Stat.onInsertSuccess()    ;
            return true ;
        }

        /// Ensures that the \p val exists in the tree
        /**
            The operation performs inserting or changing data with lock-free manner.

            If the item \p val is not found in the tree, then \p val is inserted into the tree.
            Otherwise, the functor \p func is called with item found.
            The functor signature is:
            \code
                void func( bool bNew, value_type& item, value_type& val ) ;
            \endcode
            with arguments:
            - \p bNew - \p true if the item has been inserted, \p false otherwise
            - \p item - item of the tree
            - \p val - argument \p val passed into the \p ensure function
            If new item has been inserted (i.e. \p bNew is \p true) then \p item and \p val arguments
            refer to the same thing.

            The functor can change non-key fields of the \p item; however, \p func must guarantee
            that during changing no any other modifications could be made on this item by concurrent threads.

            You can pass \p func argument by value or by reference using <tt>boost::ref</tt> or cds::ref.

            Returns std::pair<bool, bool> where \p first is \p true if operation is successfull,
            \p second is \p true if new item has been added or \p false if the item with \p key
            already is in the tree.
        */
        template <typename Func>
        std::pair<bool, bool> ensure( value_type& val, Func func )
        {
            unique_internal_node_ptr pNewInternal ;

            search_result res ;
            for ( ;; ) {
                if ( search( res, val, node_compare() )) {
                    cds::unref(func)( false, *node_traits::to_value_ptr( res.pLeaf ), val ) ;
                    if ( pNewInternal.get() )
                        m_Stat.onInternalNodeDeleted() ;    // unique_internal_node_ptr deletes internal node
                    m_Stat.onEnsureExist() ;
                    return std::make_pair( true, false ) ;
                }

                if ( res.updParent.bits() != update_desc::Clean )
                    help( res.updParent ) ;
                else {

                    if ( !pNewInternal.get() )
                        pNewInternal.reset( alloc_internal_node() );

                    if ( try_insert( val, pNewInternal.get(), res )) {
                        cds::unref(func)( true, val, val )    ;
                        pNewInternal.release()  ;   // internal node is linked into the tree and should not be deleted
                        break;
                    }
                }
                m_Stat.onEnsureRetry()  ;
            }

            ++m_ItemCounter ;
            m_Stat.onEnsureNew()    ;
            return std::make_pair( true, true ) ;
        }

        /// Unlinks the item \p val from the tree
        /**
            The function searches the item \p val in the tree and unlink it from the tree
            if it is found and is equal to \p val.

            Difference between \ref erase and \p unlink functions: \p erase finds <i>a key</i>
            and deletes the item found. \p unlink finds an item by key and deletes it
            only if \p val is an item of the tree, i.e. the pointer to item found
            is equal to <tt> &val </tt>.

            The \ref disposer specified in \p Traits class template parameter is called
            by garbage collector \p GC asynchronously.

            The function returns \p true if success and \p false otherwise.
        */
        bool unlink( value_type& val )
        {
#       if defined(CDS_CXX11_LAMBDA_SUPPORT) && !(CDS_COMPILER == CDS_COMPILER_MSVC && CDS_COMPILER_VERSION == CDS_COMPILER_MSVC10)
            // vc10 generates an error for the lambda - it sees cds::intrusive::node_traits but not class-defined node_traits
            return erase_( val, node_compare(),
                []( value_type const& v, leaf_node const& n ) -> bool { return &v == node_traits::to_value_ptr( n ); },
                [](value_type const&) {} )  ;
#       else
            return erase_( val, node_compare(), unlink_equal_functor(), empty_erase_functor() )  ;
#       endif
        }

        /// Deletes the item from the tree
        /** \anchor cds_intrusive_EllenBinTree_erase
            The function searches an item with key equal to \p val in the tree,
            unlinks it from the tree, and returns \p true.
            If the item with key equal to \p val is not found the function return \p false.

            Note the hash functor should accept a parameter of type \p Q that can be not the same as \p value_type.
        */
        template <typename Q>
        bool erase( const Q& val )
        {
#       ifdef CDS_CXX11_LAMBDA_SUPPORT
            return erase_( val, node_compare(),
                []( Q const&, leaf_node const& ) -> bool { return true; },
                [](value_type const&) {} )  ;
#       else
            return erase_( val, node_compare(), trivial_equal_functor(), empty_erase_functor() )  ;
#       endif
        }

        /// Delete the item from the tree with comparing functor \p pred
        /**
            The function is an analog of \ref cds_intrusive_EllenBinTree_erase "erase(Q const&)"
            but \p pred predicate is used for key comparing.
            \p Less has the interface like \p std::less and should meet \ref cds_intrusive_EllenBinTree_less
            "Predicate requirements".
            \p pred must imply the same element order as the comparator used for building the tree.
        */
        template <typename Q, typename Less>
        bool erase_with( const Q& val, Less pred )
        {
            typedef ellen_bintree::details::compare<
                key_type,
                value_type,
                opt::details::make_comparator_from_less<Less>,
                node_traits
            > compare_functor ;

#       ifdef CDS_CXX11_LAMBDA_SUPPORT
            return erase_( val, compare_functor(),
                []( Q const&, leaf_node const& ) -> bool { return true; },
                [](value_type const&) {} )  ;
#       else
            return erase_( val, compare_functor(), trivial_equal_functor(), empty_erase_functor() )  ;
#       endif
        }

        /// Deletes the item from the tree
        /** \anchor cds_intrusive_EllenBinTree_erase_func
            The function searches an item with key equal to \p val in the tree,
            call \p f functor with item found, unlinks it from the tree, and returns \p true.
            The \ref disposer specified in \p Traits class template parameter is called
            by garbage collector \p GC asynchronously.

            The \p Func interface is
            \code
            struct functor {
                void operator()( value_type const& item ) ;
            } ;
            \endcode
            The functor can be passed by reference with <tt>boost:ref</tt>

            If the item with key equal to \p val is not found the function return \p false.

            Note the hash functor should accept a parameter of type \p Q that can be not the same as \p value_type.
        */
        template <typename Q, typename Func>
        bool erase( Q const& val, Func f )
        {
#       ifdef CDS_CXX11_LAMBDA_SUPPORT
            return erase_( val, node_compare(),
                []( Q const&, leaf_node const& ) -> bool { return true; },
                f )  ;
#       else
            return erase_( val, node_compare(), trivial_equal_functor(), f )  ;
#       endif
        }

        /// Delete the item from the tree with comparing functor \p pred
        /**
            The function is an analog of \ref cds_intrusive_EllenBinTree_erase_func "erase(Q const&, Func)"
            but \p pred predicate is used for key comparing.
            \p Less has the interface like \p std::less and should meet \ref cds_intrusive_EllenBinTree_less
            "Predicate requirements".
            \p pred must imply the same element order as the comparator used for building the tree.
        */
        template <typename Q, typename Less, typename Func>
        bool erase_with( Q const& val, Less pred, Func f )
        {
            typedef ellen_bintree::details::compare<
                key_type,
                value_type,
                opt::details::make_comparator_from_less<Less>,
                node_traits
            > compare_functor ;

#       ifdef CDS_CXX11_LAMBDA_SUPPORT
            return erase_( val, compare_functor(),
                []( Q const&, leaf_node const& ) -> bool { return true; },
                f )  ;
#       else
            return erase_( val, compare_functor(), trivial_equal_functor(), f )  ;
#       endif
        }

        /// Extracts an item with minimal key from the tree
        /**
            The function searches an item with minimal key, unlinks it, and returns pointer to an item found.
            If the tree is empty the function returns \p NULL.

            @note Due the concurrent nature of the tree, the function extracts <i>nearly</i> minimum key.
            It means that the function gets leftmost leaf of the tree and tries to unlink it.
            During unlinking, a concurrent thread may insert an item with key less than leftmost item's key.
            So, the function returns the item with minimum key at the moment of tree traversing.

            The function does not call the disposer for the item found.
            You should explicitly call \p GC::retire member function to free the item.
        */
        value_type * extract_min()
        {
            update_desc * pOp = null_ptr<update_desc *>() ;
            search_result res ;

            for ( ;; ) {
                if ( !search_min( res )) {
                    // Tree is empty
                    if ( pOp )
                        retire_update_desc( pOp ) ;
                    m_Stat.onExtractMinFailed() ;
                    return null_ptr<value_type *>() ;
                }

                if ( res.updGrandParent.bits() != update_desc::Clean )
                    help( res.updGrandParent ) ;
                else if ( res.updParent.bits() != update_desc::Clean )
                    help( res.updParent ) ;
                else {
                    if ( !pOp )
                        pOp = alloc_update_desc()   ;
                    if ( check_delete_precondition( res ) ) {
                        pOp->dInfo.pGrandParent = res.pGrandParent  ;
                        pOp->dInfo.pParent = res.pParent    ;
                        pOp->dInfo.pLeaf = res.pLeaf        ;
                        pOp->dInfo.bDisposeLeaf = false     ;
                        pOp->pUpdate = res.updParent        ;
                        pOp->dInfo.bRightParent = res.bRightParent;
                        pOp->dInfo.bRightLeaf = res.bRightLeaf      ;

                        res.pGrandParent->m_pUpdate.store( update_ptr( pOp, update_desc::DFlag ), memory_model::memory_order_release ) ;

                        if ( help_delete( pOp ))
                            break;
                        pOp = null_ptr<update_desc *>() ;
                    }
                }

                m_Stat.onExtractMinRetry() ;
            }

            --m_ItemCounter     ;
            m_Stat.onExtractMinSuccess() ;
            return node_traits::to_value_ptr( res.pLeaf ) ;
        }

        /// Extracts an item with maximal key from the tree
        /**
            The function searches an item with maximal key, unlinks it, and returns pointer to an item found.
            If the tree is empty the function returns \p NULL.

            @note Due the concurrent nature of the tree, the function extracts <i>nearly</i> maximal key.
            It means that the function gets rightmost leaf of the tree and tries to unlink it.
            During unlinking, a concurrent thread may insert an item with key great than leftmost item's key.
            So, the function returns the item with maximum key at the moment of tree traversing.

            The function does not call the disposer for the item found.
            You should explicitly call \p GC::retire member function to free the item.
        */
        value_type * extract_max()
        {
            update_desc * pOp = null_ptr<update_desc *>() ;
            search_result res ;

            for ( ;; ) {
                if ( !search_max( res )) {
                    // Tree is empty
                    if ( pOp )
                        retire_update_desc( pOp ) ;
                    m_Stat.onExtractMaxFailed() ;
                    return null_ptr<value_type *>() ;
                }

                if ( res.updGrandParent.bits() != update_desc::Clean )
                    help( res.updGrandParent ) ;
                else if ( res.updParent.bits() != update_desc::Clean )
                    help( res.updParent ) ;
                else {
                    if ( !pOp )
                        pOp = alloc_update_desc() ;
                    if ( check_delete_precondition( res ) ) {
                        pOp->dInfo.pGrandParent = res.pGrandParent  ;
                        pOp->dInfo.pParent = res.pParent    ;
                        pOp->dInfo.pLeaf = res.pLeaf        ;
                        pOp->dInfo.bDisposeLeaf = false     ;
                        pOp->pUpdate = res.updParent        ;
                        pOp->dInfo.bRightParent = res.bRightParent;
                        pOp->dInfo.bRightLeaf = res.bRightLeaf      ;

                        res.pGrandParent->m_pUpdate.store( update_ptr( pOp, update_desc::DFlag ), memory_model::memory_order_release ) ;
                        if ( help_delete( pOp ))
                            break;
                        pOp = null_ptr<update_desc *>() ;
                    }
                }

                m_Stat.onExtractMaxRetry() ;
            }

            --m_ItemCounter     ;
            m_Stat.onExtractMaxSuccess() ;
            return node_traits::to_value_ptr( res.pLeaf ) ;
        }

        /// Extracts an item from the tree
        /** \anchor cds_intrusive_EllenBinTree_extract
            The function searches an item with key equal to \p val in the tree,
            unlinks it, and returns pointer to an item found.
            If the item with the key equal to \p val is not found the function returns \p NULL.

            The function does not call the disposer for the item found.
            You should explicitly call \p GC::retire member function to free the item.
        */
        template <typename Q>
        value_type * extract( Q const& val )
        {
            return extract_( val, node_compare() ) ;
        }

        /// Extracts an item from the set using \p pred for searching
        /**
            The function is an analog of \ref cds_intrusive_EllenBinTree_extract "extract(Q const&)"
            but \p cmp is used for key compare.
            \p Less has the interface like \p std::less and should meet \ref cds_intrusive_EllenBinTree_less
            "Predicate requirements".
            \p pred must imply the same element order as the comparator used for building the set.
        */
        template <typename Q, typename Less>
        value_type * extract_with( Q const& val, Less pred )
        {
            typedef ellen_bintree::details::compare<
                key_type,
                value_type,
                opt::details::make_comparator_from_less<Less>,
                node_traits
            > compare_functor ;

            return extract_( val, compare_functor() ) ;
        }


        /// Finds the key \p val
        /** @anchor cds_intrusive_EllenBinTree_find_val
            The function searches the item with key equal to \p val
            and returns \p true if it is found, and \p false otherwise.

            Note the hash functor specified for class \p Traits template parameter
            should accept a parameter of type \p Q that can be not the same as \p value_type.
        */
        template <typename Q>
        bool find( Q const& val )
        {
            search_result    res ;
            if ( search( res, val, node_compare() )) {
                m_Stat.onFindSuccess() ;
                return true ;
            }

            m_Stat.onFindFailed() ;
            return false ;
        }

        /// Finds the key \p val with comparing functor \p pred
        /**
            The function is an analog of \ref cds_intrusive_EllenBinTree_find_val "find(Q const&)"
            but \p pred is used for key compare.
            \p Less functor has the interface like \p std::less and should meet \ref cds_intrusive_EllenBinTree_less
            "Predicate requirements".
            \p pred must imply the same element order as the comparator used for building the tree.
            \p pred should accept arguments of type \p Q, \p key_type, \p value_type in any combination.
        */
        template <typename Q, typename Less>
        bool find_with( Q const& val, Less pred )
        {
            typedef ellen_bintree::details::compare<
                key_type,
                value_type,
                opt::details::make_comparator_from_less<Less>,
                node_traits
            > compare_functor ;

            search_result    res ;
            if ( search( res, val, compare_functor() )) {
                m_Stat.onFindSuccess() ;
                return true ;
            }
            m_Stat.onFindFailed() ;
            return false ;
        }

        /// Finds the key \p val
        /** @anchor cds_intrusive_EllenBinTree_find_func
            The function searches the item with key equal to \p val and calls the functor \p f for item found.
            The interface of \p Func functor is:
            \code
            struct functor {
                void operator()( value_type& item, Q& val ) ;
            };
            \endcode
            where \p item is the item found, \p val is the <tt>find</tt> function argument.

            You can pass \p f argument by value or by reference using <tt>boost::ref</tt> or cds::ref.

            The functor can change non-key fields of \p item. Note that the functor is only guarantee
            that \p item cannot be disposed during functor is executing.
            The functor does not serialize simultaneous access to the tree \p item. If such access is
            possible you must provide your own synchronization schema on item level to exclude unsafe item modifications.

            The \p val argument is non-const since it can be used as \p f functor destination i.e., the functor
            can modify both arguments.

            The function returns \p true if \p val is found, \p false otherwise.
        */
        template <typename Q, typename Func>
        bool find( Q& val, Func f )
        {
            return find_( val, f ) ;
        }

        /// Finds the key \p val with comparing functor \p pred
        /**
            The function is an analog of \ref cds_intrusive_EllenBinTree_find_func "find(Q&, Func)"
            but \p cmp is used for key comparison.
            \p Less functor has the interface like \p std::less and should meet \ref cds_intrusive_EllenBinTree_less
            "Predicate requirements".
            \p cmp must imply the same element order as the comparator used for building the tree.
        */
        template <typename Q, typename Less, typename Func>
        bool find_with( Q& val, Less pred, Func f )
        {
            return find_with_( val, pred, f ) ;
        }

        /// Finds the key \p val
        /** @anchor cds_intrusive_EllenBinTree_find_cfunc
            The function searches the item with key equal to \p val and calls the functor \p f for item found.
            The interface of \p Func functor is:
            \code
            struct functor {
                void operator()( value_type& item, Q const& val ) ;
            };
            \endcode
            where \p item is the item found, \p val is the <tt>find</tt> function argument.

            You can pass \p f argument by value or by reference using <tt>boost::ref</tt> or cds::ref.

            The functor can change non-key fields of \p item. Note that the functor is only guarantee
            that \p item cannot be disposed during functor is executing.
            The functor does not serialize simultaneous access to the tree \p item. If such access is
            possible you must provide your own synchronization schema on item level to exclude unsafe item modifications.

            The function returns \p true if \p val is found, \p false otherwise.
        */
        template <typename Q, typename Func>
        bool find( Q const& val, Func f )
        {
            return find_( val, f ) ;
        }

        /// Finds the key \p val with comparing functor \p pred
        /**
            The function is an analog of \ref cds_intrusive_EllenBinTree_find_cfunc "find(Q const&, Func)"
            but \p cmp is used for key compare.
            \p Less functor has the interface like \p std::less and should meet \ref cds_intrusive_EllenBinTree_less
            "Predicate requirements".
            \p cmp must imply the same element order as the comparator used for building the tree.
        */
        template <typename Q, typename Less, typename Func>
        bool find_with( Q const& val, Less pred, Func f )
        {
            return find_with_( val, pred, f ) ;
        }

        /// Checks if the tree is empty
        bool empty() const
        {
            return m_Root.m_pLeft.load( memory_model::memory_order_relaxed )->is_leaf() ;
        }

        /// Clears the tree (non-atomic)
        /**
            The function unlink all items from the tree.
            The function is not atomic, thus, in multi-threaded environment with parallel insertions
            this sequence
            \code
            set.clear() ;
            assert( set.empty() ) ;
            \endcode
            the assertion could be raised.

            For each leaf the \ref disposer will be called after unlinking.
        */
        void clear()
        {
            value_type * p ;
            while ( (p = extract_min()) != null_ptr<value_type *>() )
                gc::retire( p, free_leaf_node ) ;
        }

        /// Returns item count in the tree
        /**
            Only leaf nodes containing user data are counted.

            The value returned depends on item counter type provided by \p Traits template parameter.
            If it is atomicity::empty_item_counter this function always returns 0.
            Therefore, the function is not suitable for checking the tree emptiness, use \ref empty
            member function for this purpose.
        */
        size_t size() const
        {
            return m_ItemCounter    ;
        }

        /// Returns const reference to internal statistics
        stat const& statistics() const
        {
            return m_Stat ;
        }

        /// Checks internal consistency (not atomic, not thread-safe)
        /**
            The debugging function to check internal consistency of the tree.
        */
        bool check_consistency() const
        {
            return check_consistency( &m_Root ) ;
        }

    protected:
        //@cond

        bool check_consistency( internal_node const * pRoot ) const
        {
            tree_node * pLeft  = pRoot->m_pLeft.load( CDS_ATOMIC::memory_order_relaxed ) ;
            tree_node * pRight = pRoot->m_pRight.load( CDS_ATOMIC::memory_order_relaxed );
            assert( pLeft ) ;
            assert( pRight ) ;

            if ( node_compare()( *pLeft, *pRoot ) < 0
                && node_compare()( *pRoot, *pRight ) <= 0
                && node_compare()( *pLeft, *pRight ) < 0 )
            {
                bool bRet = true ;
                if ( pLeft->is_internal() )
                    bRet = check_consistency( static_cast<internal_node *>( pLeft ) ) ;
                assert( bRet )  ;

                if ( bRet && pRight->is_internal() )
                    bRet = bRet && check_consistency( static_cast<internal_node *>( pRight )) ;
                assert( bRet ) ;

                return bRet ;
            }
            return false ;
        }

        tree_node * search_protect_node( search_result& res, CDS_ATOMIC::atomic<tree_node *>& pNode )
        {
            tree_node * p ;
            tree_node * pn = pNode.load( memory_model::memory_order_relaxed ) ;
            do {
                p = pn ;
                if ( p->is_internal() )
                    res.guards.assign( search_result::Guard_Leaf, static_cast<internal_node *>( p )) ;
                else
                    res.guards.assign( search_result::Guard_Leaf, node_traits::to_value_ptr( static_cast<leaf_node *>( p ) )) ;
            } while ( p != ( pn = pNode.load( memory_model::memory_order_acquire ))) ;
            return p ;
        }

        template <typename KeyValue, typename Compare>
        bool search( search_result& res, KeyValue const& key, Compare cmp )
        {
            internal_node * pParent         ;
            internal_node * pGrandParent    ;
            update_ptr      updParent       ;
            update_ptr      updGrandParent  ;
            bool bRightLeaf = false         ;
            bool bRightParent = false       ;

            int nCmp = 0;

        retry:
            pParent = null_ptr< internal_node *>() ;
            pGrandParent = null_ptr<internal_node *>()   ;
            tree_node *     pLeaf = const_cast<internal_node *>( &m_Root ) ;
            while ( pLeaf->is_internal() ) {
                res.guards.copy( search_result::Guard_GrandParent, search_result::Guard_Parent ) ;
                pGrandParent = pParent          ;
                res.guards.copy( search_result::Guard_Parent, search_result::Guard_Leaf ) ;
                pParent = static_cast<internal_node *>( pLeaf ) ;
                bRightParent = bRightLeaf       ;
                res.guards.copy( search_result::Guard_updGrandParent, search_result::Guard_updParent ) ;
                updGrandParent = updParent      ;

                {
                    update_ptr upd( pParent->m_pUpdate.load( memory_model::memory_order_relaxed ) ) ;
                    do {
                        updParent = upd ;
                        res.guards.assign( search_result::Guard_updParent, upd )    ;
                    } while ( updParent != (upd = pParent->m_pUpdate.load( memory_model::memory_order_acquire )) ) ;
                }

                if ( updParent.bits() != update_desc::Clean ) {
                    // Help and retry
                    help( updParent )   ;
                    goto retry          ;
                }

                nCmp = cmp( key, *pParent ) ;
                bRightLeaf = nCmp >= 0      ;

                pLeaf = search_protect_node( res, nCmp < 0 ? pParent->m_pLeft : pParent->m_pRight ) ;
            }

            assert( pLeaf->is_leaf() )  ;
            nCmp = cmp( key, *static_cast<leaf_node *>(pLeaf) ) ;

            res.pGrandParent    = pGrandParent  ;
            res.pParent         = pParent       ;
            res.pLeaf           = static_cast<leaf_node *>( pLeaf ) ;
            res.updParent       = updParent     ;
            res.updGrandParent  = updGrandParent;
            res.bRightParent    = bRightParent  ;
            res.bRightLeaf      = bRightLeaf    ;

            return nCmp == 0    ;
        }

        bool search_min( search_result& res )
        {
            internal_node * pParent         ;
            internal_node * pGrandParent    ;
            update_ptr      updParent       ;
            update_ptr      updGrandParent  ;

        retry:
            pParent = null_ptr< internal_node *>() ;
            pGrandParent = null_ptr<internal_node *>()   ;
            tree_node *     pLeaf = const_cast<internal_node *>( &m_Root ) ;
            while ( pLeaf->is_internal() ) {
                res.guards.copy( search_result::Guard_GrandParent, search_result::Guard_Parent ) ;
                pGrandParent = pParent          ;
                res.guards.copy( search_result::Guard_Parent, search_result::Guard_Leaf ) ;
                pParent = static_cast<internal_node *>( pLeaf ) ;
                res.guards.copy( search_result::Guard_updGrandParent, search_result::Guard_updParent ) ;
                updGrandParent = updParent ;

                {
                    update_ptr upd( pParent->m_pUpdate.load( memory_model::memory_order_relaxed ) ) ;
                    do {
                        updParent = upd ;
                        res.guards.assign( search_result::Guard_updParent, upd )    ;
                    } while ( updParent != (upd = pParent->m_pUpdate.load( memory_model::memory_order_acquire )) ) ;
                }

                if ( updParent.bits() != update_desc::Clean ) {
                    // Help and retry
                    help( updParent )   ;
                    goto retry          ;
                }

                pLeaf = search_protect_node( res, pParent->m_pLeft ) ;
            }

            if ( pLeaf->infinite_key())
                return false ;

            res.pGrandParent    = pGrandParent  ;
            res.pParent         = pParent       ;
            assert( pLeaf->is_leaf() )  ;
            res.pLeaf           = static_cast<leaf_node *>( pLeaf ) ;
            res.updParent       = updParent     ;
            res.updGrandParent  = updGrandParent;
            res.bRightParent    = false         ;
            res.bRightLeaf      = false         ;

            return true ;
        }

        bool search_max( search_result& res )
        {
            internal_node * pParent         ;
            internal_node * pGrandParent    ;
            update_ptr      updParent       ;
            update_ptr      updGrandParent  ;
            bool bRightLeaf = false     ;
            bool bRightParent = false   ;

        retry:
            pParent = null_ptr< internal_node *>()      ;
            pGrandParent = null_ptr<internal_node *>()   ;
            tree_node *     pLeaf = const_cast<internal_node *>( &m_Root ) ;
            while ( pLeaf->is_internal() ) {
                res.guards.copy( search_result::Guard_GrandParent, search_result::Guard_Parent ) ;
                pGrandParent = pParent          ;
                res.guards.copy( search_result::Guard_Parent, search_result::Guard_Leaf ) ;
                pParent = static_cast<internal_node *>( pLeaf ) ;
                bRightParent = bRightLeaf       ;
                res.guards.copy( search_result::Guard_updGrandParent, search_result::Guard_updParent ) ;
                updGrandParent = updParent      ;

                {
                    update_ptr upd( pParent->m_pUpdate.load( memory_model::memory_order_relaxed ) ) ;
                    do {
                        updParent = upd ;
                        res.guards.assign( search_result::Guard_updParent, upd )    ;
                    } while ( updParent != (upd = pParent->m_pUpdate.load( memory_model::memory_order_acquire )) ) ;
                }

                if ( updParent.bits() != update_desc::Clean ) {
                    // Help and retry
                    help( updParent )   ;
                    goto retry          ;
                }

                if ( pParent->infinite_key()) {
                    pLeaf = search_protect_node( res, pParent->m_pLeft ) ;
                    bRightLeaf = false ;
                }
                else {
                    pLeaf = search_protect_node( res, pParent->m_pRight ) ;
                    bRightLeaf = true   ;
                }
            }

            if ( pLeaf->infinite_key())
                return false ;

            res.pGrandParent    = pGrandParent  ;
            res.pParent         = pParent       ;
            assert( pLeaf->is_leaf() )  ;
            res.pLeaf           = static_cast<leaf_node *>( pLeaf ) ;
            res.updParent       = updParent     ;
            res.updGrandParent  = updGrandParent    ;
            res.bRightParent    = bRightParent  ;
            res.bRightLeaf      = bRightLeaf    ;

            return true ;
        }

        void help( update_ptr pUpdate )
        {
            // pUpdate must be guarded!

            if ( pUpdate.all() != update_desc::locked_desc() ) {
                // Helping is disabled since it leads to a strange problems
                // like double deallocation of node. The cause is obscure
                /*
                switch ( pUpdate.bits() ) {
                    case update_desc::IFlag:
                        m_Stat.onHelpInsert() ;
                        help_insert( pUpdate.ptr() ) ;
                        break;
                    case update_desc::DFlag:
                        m_Stat.onHelpDelete() ;
                        help_delete( pUpdate.ptr() ) ;
                        break;
                    case update_desc::Mark:
                        m_Stat.onHelpMark() ;
                        help_marked( pUpdate.ptr() ) ;
                        break;
                }
                */
            }
            else
                m_Stat.onNodeLocked() ;
        }

        void help_insert( update_desc * pOp )
        {
            // pOp must be guarded

            tree_node * pLeaf = static_cast<tree_node *>( pOp->iInfo.pLeaf ) ;
            if ( pOp->iInfo.bRightLeaf ) {
                pOp->iInfo.pParent->m_pRight.compare_exchange_strong( pLeaf, static_cast<tree_node *>( pOp->iInfo.pNew ),
                    memory_model::memory_order_relaxed, CDS_ATOMIC::memory_order_relaxed );
            }
            else {
                pOp->iInfo.pParent->m_pLeft.compare_exchange_strong( pLeaf, static_cast<tree_node *>( pOp->iInfo.pNew ),
                    memory_model::memory_order_relaxed, CDS_ATOMIC::memory_order_relaxed );
            }

            update_ptr cur( pOp, update_desc::IFlag ) ;
            if ( pOp->iInfo.pParent->m_pUpdate.compare_exchange_strong( cur, update_ptr( pOp ),
                memory_model::memory_order_release, CDS_ATOMIC::memory_order_relaxed ))
            {
                retire_update_desc( pOp )   ;
            }
        }

        bool check_delete_precondition( search_result& res )
        {
            // precondition: all member of res must be guarded

            assert( res.pGrandParent != null_ptr<internal_node *>() ) ;

            if ( res.pGrandParent->m_pUpdate.compare_exchange_strong( res.updGrandParent, update_ptr(update_desc::locked_desc()),
                memory_model::memory_order_acquire, CDS_ATOMIC::memory_order_relaxed ))
            {
                if ( static_cast<internal_node *>( res.bRightParent
                    ? res.pGrandParent->m_pRight.load(memory_model::memory_order_relaxed)
                    : res.pGrandParent->m_pLeft.load(memory_model::memory_order_relaxed) )
                    == res.pParent )
                {
                    if ( res.pParent->m_pUpdate.compare_exchange_strong( res.updParent, update_ptr(update_desc::locked_desc()),
                        memory_model::memory_order_acquire, CDS_ATOMIC::memory_order_relaxed ))
                    {
                        if ( static_cast<leaf_node *>(res.bRightLeaf
                            ? res.pParent->m_pRight.load(memory_model::memory_order_relaxed)
                            : res.pParent->m_pLeft.load(memory_model::memory_order_relaxed) )
                            == res.pLeaf )
                        {
                            return true ;
                        }
                        res.pParent->m_pUpdate.store( res.updParent, memory_model::memory_order_release ) ;
                    }
                }
                res.pGrandParent->m_pUpdate.store( res.updGrandParent, memory_model::memory_order_release ) ;
            }
            return false ;
        }

        bool help_delete( update_desc * pOp )
        {
            // precondition: pOp must be guarded

            update_ptr pUpdate( update_desc::locked_desc() ) ;
            update_ptr pMark( pOp, update_desc::Mark ) ;
            if ( pOp->dInfo.pParent->m_pUpdate.compare_exchange_strong( pUpdate, pMark,
                memory_model::memory_order_acquire, CDS_ATOMIC::memory_order_relaxed )
                || pUpdate == pMark )
            {
                help_marked( pOp ) ;
                return true ;
            }
            else {
                // Undo grandparent dInfo
                update_ptr pDel( pOp, update_desc::DFlag ) ;
                if ( pOp->dInfo.pGrandParent->m_pUpdate.compare_exchange_strong( pDel, update_ptr( pOp ),
                    memory_model::memory_order_release, CDS_ATOMIC::memory_order_relaxed ))
                {
                    retire_update_desc( pOp )   ;
                }

                help( pUpdate ) ;
                return false ;
            }
        }

        void help_marked( update_desc * pOp )
        {
            // precondition: pOp must be guarded

            bool bSuccess ;
            tree_node * pParent = pOp->dInfo.pParent ;

            typename gc::Guard guard ;
            tree_node * pOpposite    ;
            if ( pOp->dInfo.bRightLeaf ) {
                tree_node * p = pOp->dInfo.pParent->m_pLeft.load( memory_model::memory_order_relaxed ) ;
                do {
                    pOpposite = p ;
                    if ( p->is_internal() )
                        guard.assign( static_cast<internal_node *>(p) ) ;
                    else
                        guard.assign( node_traits::to_value_ptr( static_cast<leaf_node *>(p))) ;
                } while ( pOpposite != ( p = pOp->dInfo.pParent->m_pLeft.load( memory_model::memory_order_acquire )) ) ;
            }
            else {
                tree_node * p = pOp->dInfo.pParent->m_pRight.load( memory_model::memory_order_relaxed ) ;
                do {
                    pOpposite = p ;
                    if ( p->is_internal() )
                        guard.assign( static_cast<internal_node *>(p) ) ;
                    else
                        guard.assign( node_traits::to_value_ptr( static_cast<leaf_node *>(p))) ;
                } while ( pOpposite != ( p = pOp->dInfo.pParent->m_pRight.load( memory_model::memory_order_acquire )) ) ;
            }

            if ( pOp->dInfo.bRightParent ) {
                bSuccess = pOp->dInfo.pGrandParent->m_pRight.compare_exchange_strong( pParent, pOpposite,
                    memory_model::memory_order_relaxed, CDS_ATOMIC::memory_order_relaxed ) ;
            }
            else {
                bSuccess = pOp->dInfo.pGrandParent->m_pLeft.compare_exchange_strong( pParent, pOpposite,
                    memory_model::memory_order_relaxed, CDS_ATOMIC::memory_order_relaxed ) ;
            }

            if ( bSuccess ) {
                // Retire pOp->dInfo.pParent
                retire_node( pOp->dInfo.pParent )   ;

                // For extract operations the leaf should NOT be disposed
                if ( pOp->dInfo.bDisposeLeaf )
                    retire_node( pOp->dInfo.pLeaf )   ;
            }

            update_ptr upd( pOp, update_desc::DFlag ) ;
            if ( pOp->dInfo.pGrandParent->m_pUpdate.compare_exchange_strong( upd, update_ptr( pOp ),
                memory_model::memory_order_release, CDS_ATOMIC::memory_order_relaxed ))
            {
                retire_update_desc( pOp )   ;
            }
        }

        bool try_insert( value_type& val, internal_node * pNewInternal, search_result& res )
        {
            typename gc::Guard guard ;
            update_desc * pOp = alloc_update_desc()   ;
            guard.assign( pOp ) ;

            leaf_node * pNewLeaf = node_traits::to_node_ptr( val ) ;

            int nCmp = node_compare()( val, *res.pLeaf ) ;
            if ( nCmp < 0 ) {
                if ( res.pGrandParent ) {
                    assert( !res.pLeaf->infinite_key() ) ;
                    pNewInternal->infinite_key( 0 ) ;
                    key_extractor()( pNewInternal->m_Key, *node_traits::to_value_ptr( res.pLeaf ) ) ;
                }
                else {
                    assert( res.pLeaf->infinite_key() == tree_node::key_infinite1 ) ;
                    pNewInternal->infinite_key( 1 ) ;
                }
                pNewInternal->m_pLeft.store( static_cast<tree_node *>(pNewLeaf), memory_model::memory_order_relaxed ) ;
                pNewInternal->m_pRight.store( static_cast<tree_node *>(res.pLeaf), memory_model::memory_order_release ) ;
            }
            else {
                assert( !res.pLeaf->is_internal() ) ;
                pNewInternal->infinite_key( 0 ) ;

                key_extractor()( pNewInternal->m_Key, val ) ;
                pNewInternal->m_pLeft.store( static_cast<tree_node *>(res.pLeaf), memory_model::memory_order_relaxed ) ;
                pNewInternal->m_pRight.store( static_cast<tree_node *>(pNewLeaf), memory_model::memory_order_release ) ;
                assert( !res.pLeaf->infinite_key()) ;
            }

            pOp->iInfo.pParent = res.pParent    ;
            pOp->iInfo.pNew = pNewInternal      ;
            pOp->iInfo.pLeaf = res.pLeaf        ;
            pOp->iInfo.bRightLeaf = res.bRightLeaf  ;

            assert( res.updParent.bits() == update_desc::Clean ) ;
            if ( res.pParent->m_pUpdate.compare_exchange_strong( res.updParent, update_ptr( update_desc::locked_desc() ),
                memory_model::memory_order_acquire, CDS_ATOMIC::memory_order_relaxed ))
            {
                // check search result
                if ( ( pOp->iInfo.bRightLeaf
                    ? pOp->iInfo.pParent->m_pRight.load( memory_model::memory_order_relaxed )
                    : pOp->iInfo.pParent->m_pLeft.load( memory_model::memory_order_relaxed ) ) == res.pLeaf )
                {
                    res.pParent->m_pUpdate.store( update_ptr( pOp, update_desc::IFlag ), memory_model::memory_order_release ) ;

                    // do insert
                    help_insert( pOp ) ;

                    // pOp has been retired in help_insert
                    return true ;
                }
                res.pParent->m_pUpdate.store( res.updParent, memory_model::memory_order_release ) ;
            }
            else {
                // res.updParent has been actualized by CAS
                help( res.updParent )        ;
            }
            retire_update_desc( pOp )    ;
            return false ;
        }

        template <typename Q, typename Compare, typename Equal, typename Func>
        bool erase_( Q const& val, Compare cmp, Equal eq, Func f )
        {
            update_desc * pOp = null_ptr<update_desc *>() ;
            search_result res ;

            for ( ;; ) {
                if ( !search( res, val, cmp ) || !eq( val, *res.pLeaf ) ) {
                    if ( pOp )
                        retire_update_desc( pOp ) ;
                    m_Stat.onEraseFailed() ;
                    return false ;
                }

                if ( res.updGrandParent.bits() != update_desc::Clean )
                    help( res.updGrandParent ) ;
                else if ( res.updParent.bits() != update_desc::Clean )
                    help( res.updParent ) ;
                else {
                    if ( !pOp )
                        pOp = alloc_update_desc() ;
                    if ( check_delete_precondition( res ) ) {
                        pOp->dInfo.pGrandParent = res.pGrandParent  ;
                        pOp->dInfo.pParent = res.pParent    ;
                        pOp->dInfo.pLeaf = res.pLeaf        ;
                        pOp->dInfo.bDisposeLeaf = true      ;
                        pOp->pUpdate = res.updParent        ;
                        pOp->dInfo.bRightParent = res.bRightParent  ;
                        pOp->dInfo.bRightLeaf = res.bRightLeaf      ;

                        res.pGrandParent->m_pUpdate.store( update_ptr( pOp, update_desc::DFlag ), memory_model::memory_order_release ) ;

                        if ( help_delete( pOp )) {
                            // res.pLeaf is not deleted yet since res.pLeaf is guarded
                            cds::unref(f)( *node_traits::to_value_ptr( res.pLeaf )) ;
                            break;
                        }
                        pOp = null_ptr<update_desc *>() ;
                    }
                }

                m_Stat.onEraseRetry() ;
            }

            --m_ItemCounter ;
            m_Stat.onEraseSuccess() ;
            return true ;
        }

        template <typename Q, typename Compare>
        value_type * extract_( Q const& val, Compare cmp )
        {
            update_desc * pOp = null_ptr<update_desc *>() ;
            search_result res ;

            for ( ;; ) {
                if ( !search( res, val, cmp ) ) {
                    if ( pOp )
                        retire_update_desc( pOp ) ;
                    m_Stat.onExtractFailed() ;
                    return null_ptr<value_type *>() ;
                }

                if ( res.updGrandParent.bits() != update_desc::Clean )
                    help( res.updGrandParent ) ;
                else if ( res.updParent.bits() != update_desc::Clean )
                    help( res.updParent ) ;
                else {
                    if ( !pOp )
                        pOp = alloc_update_desc() ;
                    if ( check_delete_precondition( res )) {
                        pOp->dInfo.pGrandParent = res.pGrandParent  ;
                        pOp->dInfo.pParent = res.pParent    ;
                        pOp->dInfo.pLeaf = res.pLeaf        ;
                        pOp->dInfo.bDisposeLeaf = false     ;
                        pOp->pUpdate = res.updParent        ;
                        pOp->dInfo.bRightParent = res.bRightParent;
                        pOp->dInfo.bRightLeaf = res.bRightLeaf      ;

                        res.pGrandParent->m_pUpdate.store( update_ptr( pOp, update_desc::DFlag), memory_model::memory_order_release ) ;
                        if ( help_delete( pOp ))
                            break;
                        pOp = null_ptr<update_desc *>() ;
                    }
                }

                m_Stat.onExtractRetry() ;
            }

            --m_ItemCounter ;
            m_Stat.onExtractSuccess() ;
            return node_traits::to_value_ptr( res.pLeaf ) ;
        }

        template <typename Q, typename Func>
        bool find_( Q& val, Func f )
        {
            search_result    res ;
            if ( search( res, val, node_compare() )) {
                assert( res.pLeaf ) ;
                cds::unref(f)( *node_traits::to_value_ptr( res.pLeaf ), val ) ;

                m_Stat.onFindSuccess() ;
                return true ;
            }

            m_Stat.onFindFailed() ;
            return false ;
        }

        template <typename Q, typename Less, typename Func>
        bool find_with_( Q& val, Less pred, Func f )
        {
            typedef ellen_bintree::details::compare<
                key_type,
                value_type,
                opt::details::make_comparator_from_less<Less>,
                node_traits
            > compare_functor ;

            search_result    res ;
            if ( search( res, val, compare_functor() )) {
                assert( res.pLeaf ) ;
                cds::unref(f)( *node_traits::to_value_ptr( res.pLeaf ), val ) ;

                m_Stat.onFindSuccess() ;
                return true ;
            }

            m_Stat.onFindFailed() ;
            return false ;
        }

        //@endcond
    };

}} // namespace cds::intrusive

#endif // #ifndef __CDS_INTRUSIVE_ELLEN_BINTREE_IMPL_H
