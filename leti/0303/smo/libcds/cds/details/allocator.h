//$$CDS-header$$

#ifndef __CDS_DETAILS_ALLOCATOR_H
#define __CDS_DETAILS_ALLOCATOR_H

/*
    Allocator class for the library. Supports allocating and constructing of objects

    Editions:
        2008.03.08    Maxim.Khiszinsky    Created
*/

#include <cds/details/defs.h>
#include <cds/user_setup/allocator.h>
#include <memory>
#include <boost/type_traits/has_trivial_destructor.hpp>

namespace cds {
    namespace details {

        /// Extends \p std::allocator interface to provide semantics like operator \p new and \p delete
        /**
            The class is the wrapper around underlying \p Alloc class. \p Alloc provides the
            interface defined in C++ standard.
        */
        template <typename T, class Alloc = CDS_DEFAULT_ALLOCATOR >
        class Allocator: public Alloc::template rebind<T>::other
        {
        public:
            /// Underlying allocator type
            typedef typename Alloc::template rebind<T>::other   allocator_type   ;

#       ifdef CDS_CXX11_VARIADIC_TEMPLATE_SUPPORT
            /// Analogue of operator new T(\p src... )
            template <typename... S>
            T *  New( S const&... src )
            {
                return Construct( allocator_type::allocate(1), src... ) ;
            }
#       else
            //@cond
            /// Analogue of operator new T
            T *  New()
            {
                return Construct( allocator_type::allocate(1) ) ;
            }

            /// Analogue of operator new T(\p src )
            template <typename S>
            T *  New( S const& src )
            {
                return Construct( allocator_type::allocate(1), src ) ;
            }

            /// Analogue of operator new T( \p s1, \p s2 )
            template <typename S1, typename S2>
            T *  New( S1 const& s1, S2 const& s2 )
            {
                return Construct( allocator_type::allocate(1), s1, s2 ) ;
            }

            /// Analogue of operator new T( \p s1, \p s2, \p s3 )
            template <typename S1, typename S2, typename S3>
            T *  New( S1 const& s1, S2 const& s2, S3 const& s3 )
            {
                return Construct( allocator_type::allocate(1), s1, s2, s3 ) ;
            }
            //@endcond
#       endif

#       ifdef CDS_EMPLACE_SUPPORT
            /// Analogue of <tt>operator new T( std::forward<Args>(args)... )</tt> (move semantics)
            /**
                This function is available only for compiler that supports
                variadic template and move semantics
            */
            template <typename... Args>
            T * MoveNew( Args&&... args )
            {
                return MoveConstruct( allocator_type::allocate(1), std::forward<Args>(args)... ) ;
            }
#       endif


            /// Analogue of operator new T[\p nCount ]
            T * NewArray( size_t nCount )
            {
                T * p = allocator_type::allocate( nCount )  ;
                for ( size_t i = 0; i < nCount; ++i )
                    Construct( p + i )  ;
                return p    ;
            }

            /// Analogue of operator new T[\p nCount ].
            /**
                Each item of array of type T is initialized by parameter \p src: T( src )
            */
            template <typename S>
            T * NewArray( size_t nCount, S const& src )
            {
                T * p = allocator_type::allocate( nCount )  ;
                for ( size_t i = 0; i < nCount; ++i )
                    Construct( p + i, src )  ;
                return p    ;
            }

            /// Analogue of operator delete
            void Delete( T * p )
            {
                allocator_type::destroy( p )    ;
                allocator_type::deallocate( p, 1 )    ;
            }

            /// Analogue of operator delete []
            void Delete( T * p, size_t nCount )
            {
                 for ( size_t i = 0; i < nCount; ++i )
                     allocator_type::destroy( p + i )    ;
                allocator_type::deallocate( p, nCount )    ;
            }

#       ifdef CDS_CXX11_VARIADIC_TEMPLATE_SUPPORT
#       if CDS_COMPILER == CDS_COMPILER_INTEL
            //@cond
            T * Construct( void * p )
            {
                return new( p ) T ;
            }
            //@endcond
#       endif
            /// Analogue of placement operator new( \p p ) T( src... )
            template <typename... S>
            T * Construct( void * p, S const&... src )
            {
                return new( p ) T( src... )    ;
            }
#       else
            //@cond
            /// Analogue of placement operator new( \p p ) T
            T * Construct( void * p )
            {
                return new( p ) T ;
            }


            /// Analogue of placement operator new( \p p ) T( \p src )
            template <typename S>
            T * Construct( void * p, S const& src )
            {
                return new( p ) T( src )    ;
            }

            /// Analogue of placement operator new( \p p ) T( \p s1, \p s2 )
            template <typename S1, typename S2>
            T *  Construct( void * p, S1 const& s1, S2 const& s2 )
            {
                return new( p ) T( s1, s2 ) ;
            }

            /// Analogue of placement operator new( \p p ) T( \p s1, \p s2, \p s3 )
            template <typename S1, typename S2, typename S3>
            T *  Construct( void * p, S1 const& s1, S2 const& s2, S3 const& s3 )
            {
                return new( p ) T( s1, s2, s3 ) ;
            }
            //@endcond
#       endif

#       ifdef CDS_EMPLACE_SUPPORT
            /// Analogue of placement <tt>operator new( p ) T( std::forward<Args>(args)... )</tt>
            /**
                This function is available only for compiler that supports
                variadic template and move semantics
            */
            template <typename... Args>
            T * MoveConstruct( void * p, Args&&... args )
            {
                return new( p ) T( std::forward<Args>(args)... )    ;
            }
#       endif

            /// Rebinds allocator to other type \p Q instead of \p T
            template <typename Q>
            struct rebind {
                typedef Allocator< Q, typename Alloc::template rebind<Q>::other >    other ; ///< Rebinding result
            };
        };

        //@cond
        namespace {
            template <class T>
            static inline void impl_call_dtor(T* p, boost::false_type const&)
            {
                p->T::~T()  ;
            }

            template <class T>
            static inline void impl_call_dtor(T* p, boost::true_type const&)
            {}
        }
        //@endcond

        /// Helper function to call destructor of type T
        /**
            This function is empty for the type T that has trivial destructor.
        */
        template <class T>
        static inline void call_dtor( T* p )
        {
            impl_call_dtor( p, ::boost::has_trivial_destructor<T>() );
        }


        /// Deferral removing of the object of type \p T. Helper class
        template <typename T, typename Alloc = CDS_DEFAULT_ALLOCATOR>
        struct deferral_deleter {
            typedef T           type            ; ///< Type
            typedef Alloc       allocator_type  ; ///< Allocator for removing

            /// Frees the object \p p
            /**
                Caveats: this function uses temporary object of type \ref cds::details::Allocator to free the node \p p.
                So, the node allocator should be stateless. It is standard requirement for \p std::allocator class objects.

                Do not use this function directly.
            */
            static void free( T * p )
            {
                Allocator<T, Alloc> a   ;
                a.Delete( p )   ;
            }
        };

    }    // namespace details
}    // namespace cds

#endif    // #ifndef __CDS_DETAILS_ALLOCATOR_H
