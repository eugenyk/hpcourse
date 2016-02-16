//$$CDS-header$$

#ifndef __CDS_OPT_BUFFER_H
#define __CDS_OPT_BUFFER_H

#include <cds/details/defs.h>
#include <cds/user_setup/allocator.h>
#include <cds/details/allocator.h>
#include <cds/int_algo.h>

namespace cds { namespace opt {

    /// [type-option] Option setter for user-provided plain buffer
    /**
        This option is used by some container as a random access array for storing
        container's item; for example, a bounded queue may use
        this option to define underlying buffer implementation.

        The template parameter \p Type should be rebindable.

        Implementations:
            - opt::v::static_buffer
            - opt::v::dynamic_buffer
    */
    template <typename Type>
    struct buffer {
        //@cond
        template <typename Base> struct pack: public Base
        {
            typedef Type buffer ;
        };
        //@endcond
    };

    namespace v {

        /// Static buffer (\ref opt::buffer option)
        /**
            One of available type for opt::buffer type-option.

            This buffer maintains static array. No dynamic memory allocation performed.

            \par Template parameters:
                \li \p T - item type the buffer stores
                \li \p Capacity - the capacity of buffer. The value must be power of two.
        */
        template <typename T, size_t Capacity>
        class static_buffer
        {
        public:
            typedef T   value_type  ;   ///< value type
            static const size_t c_nCapacity = Capacity ;    ///< Capacity

            /// Rebind buffer for other template parameters
            template <typename Q, size_t Capacity2 = c_nCapacity>
            struct rebind {
                typedef static_buffer<Q, Capacity2> other   ;   ///< Rebind result type
            };
        private:
            //@cond
            value_type  m_buffer[c_nCapacity]  ;
            //@endcond
        public:
            /// Construct static buffer
            static_buffer()
            {
                // Capacity must be power of 2
                static_assert( (c_nCapacity & (c_nCapacity - 1)) == 0, "Capacity must be power of two" ) ;
            }
            /// Construct buffer of given capacity
            /**
                This ctor ignores \p nCapacity argument. The capacity of static buffer
                is defined by template argument \p Capacity
            */
            static_buffer( size_t nCapacity )
            {
                // Capacity must be power of 2
                static_assert( (c_nCapacity & (c_nCapacity - 1)) == 0,  "Capacity must be power of two") ;
                //assert( c_nCapacity == nCapacity )  ;
            }

            /// Get item \p i
            value_type& operator []( size_t i )
            {
                assert( i < capacity() )    ;
                return m_buffer[i]          ;
            }

            /// Get item \p i, const version
            const value_type& operator []( size_t i ) const
            {
                assert( i < capacity() )    ;
                return m_buffer[i]          ;
            }

            /// Returns buffer capacity
            CDS_CONSTEXPR size_t capacity() const CDS_NOEXCEPT
            {
                return c_nCapacity  ;
            }

            /// Zeroize the buffer
            void zeroize()
            {
                memset( m_buffer, 0, capacity() * sizeof(m_buffer[0]) ) ;
            }

            /// Returns pointer to buffer array
            value_type * buffer()
            {
                return m_buffer ;
            }

            /// Returns pointer to buffer array (const version)
            value_type * buffer() const
            {
                return m_buffer ;
            }

        private:
            //@cond
            // non-copyable
            static_buffer(const static_buffer&) ;
            void operator =(const static_buffer&);
            //@endcond
        };


        /// Dynamically allocated buffer
        /**
            One of available opt::buffer type-option.

            This buffer maintains dynamically allocated array.
            Allocation is performed at construction time.

            \par Template parameters:
                \li \p T - item type storing in the buffer
                \li \p Alloc - an allocator used for allocating internal buffer (\p std::allocator interface)
        */
        template <typename T, class Alloc = CDS_DEFAULT_ALLOCATOR>
        class dynamic_buffer
        {
        public:
            typedef T   value_type  ;   ///< Value type

            /// Rebind buffer for other template parameters
            template <typename Q>
            struct rebind {
                typedef dynamic_buffer<Q, Alloc> other   ;  ///< Rebinding result type
            };

            //@cond
            typedef cds::details::Allocator<value_type, Alloc>   allocator_type  ;
            //@endcond

        private:
            //@cond
            value_type *    m_buffer    ;
            size_t const    m_nCapacity ;
            //@endcond
        public:
            /// Allocates dynamic buffer of given \p nCapacity
            /**
                \p nCapacity must be power of two.
            */
            dynamic_buffer( size_t nCapacity )
                : m_nCapacity( beans::ceil2(nCapacity) )
            {
                assert( m_nCapacity >= 2 )    ;
                // Capacity must be power of 2
                assert( (m_nCapacity & (m_nCapacity - 1)) == 0 ) ;

                allocator_type a    ;
                m_buffer = a.NewArray( m_nCapacity )  ;
            }

            /// Destroys dynamically allocated buffer
            ~dynamic_buffer()
            {
                allocator_type a    ;
                a.Delete( m_buffer, m_nCapacity )   ;
            }

            /// Get item \p i
            value_type& operator []( size_t i )
            {
                assert( i < capacity() )    ;
                return m_buffer[i]          ;
            }

            /// Get item \p i, const version
            const value_type& operator []( size_t i ) const
            {
                assert( i < capacity() )    ;
                return m_buffer[i]          ;
            }

            /// Returns buffer capacity
            size_t capacity() const CDS_NOEXCEPT
            {
                return m_nCapacity  ;
            }

            /// Zeroize the buffer
            void zeroize()
            {
                memset( m_buffer, 0, capacity() * sizeof(m_buffer[0]) ) ;
            }

            /// Returns pointer to buffer array
            value_type * buffer()
            {
                return m_buffer ;
            }

            /// Returns pointer to buffer array (const version)
            value_type * buffer() const
            {
                return m_buffer ;
            }

        private:
            //@cond
            // non-copyable
            dynamic_buffer(const dynamic_buffer&) ;
            void operator =(const dynamic_buffer&);
            //@endcond
        };

    }   // namespace v

}}  // namespace cds::opt

#endif // #ifndef __CDS_OPT_BUFFER_H
