//$$CDS-header$$

#ifndef __UNIT_STACK_TYPES_H
#define __UNIT_STACK_TYPES_H

#include <cds/container/treiber_stack.h>
#include <cds/container/michael_deque.h>

#include <cds/gc/hp.h>
#include <cds/gc/ptb.h>
#include <cds/gc/hrc.h>

namespace stack {

    namespace details {
        template <typename GC, typename T, CDS_DECL_OPTIONS7>
        class MichaelDequeL: public cds::container::MichaelDeque< GC, T, CDS_OPTIONS7>
        {
            typedef cds::container::MichaelDeque< GC, T, CDS_OPTIONS7> base_class ;
        public:
            MichaelDequeL( size_t nMaxItemCount )
                : base_class( (unsigned int) nMaxItemCount, 4 )
                {}

            bool push( T const& v )
            {
                return base_class::push_front( v )   ;
            }

            bool pop( T& v )
            {
                return base_class::pop_front( v )    ;
            }
        };

        template <typename GC, typename T, CDS_DECL_OPTIONS7>
        class MichaelDequeR: public cds::container::MichaelDeque< GC, T, CDS_OPTIONS7>
        {
            typedef cds::container::MichaelDeque< GC, T, CDS_OPTIONS7> base_class ;
        public:
            MichaelDequeR( size_t nMaxItemCount )
                : base_class( (unsigned int) nMaxItemCount, 4 )
            {}

            bool push( T const& v )
            {
                return base_class::push_back( v )   ;
            }

            bool pop( T& v )
            {
                return base_class::pop_back( v )    ;
            }
        };

    }

    template <typename T>
    struct Types {

    // TreiberStack
        typedef cds::container::TreiberStack< cds::gc::HP, T >       Treiber_HP  ;

        typedef cds::container::TreiberStack< cds::gc::HP, T
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
        >       Treiber_HP_relaxed  ;

        typedef cds::container::TreiberStack< cds::gc::HP, T
            ,cds::opt::stat<cds::intrusive::stack_stat<> >
        >       Treiber_HP_stat  ;

        typedef cds::container::TreiberStack< cds::gc::HRC, T >       Treiber_HRC ;

        typedef cds::container::TreiberStack< cds::gc::HRC, T
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
        > Treiber_HRC_relaxed ;

        typedef cds::container::TreiberStack< cds::gc::HRC, T
            ,cds::opt::stat<cds::intrusive::stack_stat<> >
        >       Treiber_HRC_stat  ;

        typedef cds::container::TreiberStack< cds::gc::PTB, T > Treiber_PTB ;

        typedef cds::container::TreiberStack< cds::gc::PTB, T
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
        > Treiber_PTB_relaxed ;

        typedef cds::container::TreiberStack< cds::gc::PTB, T
            ,cds::opt::stat<cds::intrusive::stack_stat<> >
        >       Treiber_PTB_stat  ;

        typedef cds::container::TreiberStack< cds::gc::HP, T
            ,cds::opt::back_off<cds::backoff::yield>
        > Treiber_HP_yield ;
        typedef cds::container::TreiberStack< cds::gc::HP, T
            ,cds::opt::back_off<cds::backoff::yield>
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
        > Treiber_HP_yield_relaxed  ;
        typedef cds::container::TreiberStack< cds::gc::HP, T
            ,cds::opt::back_off<cds::backoff::pause>
        > Treiber_HP_pause;
        typedef cds::container::TreiberStack< cds::gc::HP, T
            ,cds::opt::back_off<cds::backoff::pause>
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
        > Treiber_HP_pause_relaxed  ;
        typedef cds::container::TreiberStack< cds::gc::HP, T,
            cds::opt::back_off<
                cds::backoff::exponential<
                    cds::backoff::pause,
                    cds::backoff::yield
                >
            >
        >       Treiber_HP_exp  ;
        typedef cds::container::TreiberStack< cds::gc::HP, T
            ,cds::opt::back_off<
                cds::backoff::exponential<
                    cds::backoff::pause,
                    cds::backoff::yield
                >
            >
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
        > Treiber_HP_exp_relaxed  ;

        typedef cds::container::TreiberStack< cds::gc::HP, T
            ,cds::opt::back_off<cds::backoff::yield>
        >  Treiber_HRC_yield  ;
        typedef cds::container::TreiberStack< cds::gc::HRC, T
            ,cds::opt::back_off<cds::backoff::yield>
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
        >  Treiber_HRC_yield_relaxed  ;
        typedef cds::container::TreiberStack< cds::gc::HRC, T
            ,cds::opt::back_off<cds::backoff::pause>
        > Treiber_HRC_pause ;
        typedef cds::container::TreiberStack< cds::gc::HRC, T
            ,cds::opt::back_off<cds::backoff::pause>
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
        > Treiber_HRC_pause_relaxed  ;
        typedef cds::container::TreiberStack< cds::gc::HRC, T
            ,cds::opt::back_off<
                cds::backoff::exponential<
                    cds::backoff::pause,
                    cds::backoff::yield
                >
            >
        > Treiber_HRC_exp  ;
        typedef cds::container::TreiberStack< cds::gc::HRC, T
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
            ,cds::opt::back_off<
                cds::backoff::exponential<
                    cds::backoff::pause,
                    cds::backoff::yield
                >
            >
        > Treiber_HRC_exp_relaxed  ;

        typedef cds::container::TreiberStack< cds::gc::PTB, T
            ,cds::opt::back_off<cds::backoff::yield>
        >  Treiber_PTB_yield  ;
        typedef cds::container::TreiberStack< cds::gc::PTB, T
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
            ,cds::opt::back_off<cds::backoff::yield>
        >  Treiber_PTB_yield_relaxed  ;
        typedef cds::container::TreiberStack< cds::gc::PTB, T
            ,cds::opt::back_off<cds::backoff::pause>
        > Treiber_PTB_pause  ;
        typedef cds::container::TreiberStack< cds::gc::PTB, T
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
            ,cds::opt::back_off<cds::backoff::pause>
        > Treiber_PTB_pause_relaxed  ;
        typedef cds::container::TreiberStack< cds::gc::PTB, T,
            cds::opt::back_off<
                cds::backoff::exponential<
                    cds::backoff::pause,
                    cds::backoff::yield
                >
            >
        >       Treiber_PTB_exp  ;
        typedef cds::container::TreiberStack< cds::gc::PTB, T
            ,cds::opt::back_off<
                cds::backoff::exponential<
                    cds::backoff::pause,
                    cds::backoff::yield
                >
            >
            ,cds::opt::memory_model<cds::opt::v::relaxed_ordering>
        > Treiber_PTB_exp_relaxed  ;

    // MichaelDeque, left side
        typedef details::MichaelDequeL< cds::gc::HP, T> MichaelDequeL_HP ;
        typedef details::MichaelDequeL< cds::gc::HP, T
            ,cds::opt::memory_model<cds::opt::v::sequential_consistent>
        > MichaelDequeL_HP_seqcst ;

        typedef details::MichaelDequeL< cds::gc::HP, T
            ,cds::opt::item_counter< cds::atomicity::item_counter >
        > MichaelDequeL_HP_ic ;

        typedef details::MichaelDequeL< cds::gc::HP, T
            ,cds::opt::back_off<
                cds::backoff::exponential<
                    cds::backoff::pause,
                    cds::backoff::yield
                >
            >
        > MichaelDequeL_HP_exp ;

        typedef details::MichaelDequeL< cds::gc::HP, T
            ,cds::opt::back_off< cds::backoff::yield >
        > MichaelDequeL_HP_yield ;

        typedef details::MichaelDequeL< cds::gc::HP, T
            ,cds::opt::stat<cds::intrusive::deque_stat<> >
        > MichaelDequeL_HP_stat ;


        typedef details::MichaelDequeL< cds::gc::PTB, T> MichaelDequeL_PTB ;
        typedef details::MichaelDequeL< cds::gc::PTB, T
            ,cds::opt::memory_model<cds::opt::v::sequential_consistent>
        > MichaelDequeL_PTB_seqcst ;

        typedef details::MichaelDequeL< cds::gc::PTB, T
            ,cds::opt::item_counter< cds::atomicity::item_counter >
        > MichaelDequeL_PTB_ic ;

        typedef details::MichaelDequeL< cds::gc::PTB, T
            ,cds::opt::back_off<
            cds::backoff::exponential<
            cds::backoff::pause,
            cds::backoff::yield
            >
            >
        > MichaelDequeL_PTB_exp ;

        typedef details::MichaelDequeL< cds::gc::PTB, T
            ,cds::opt::back_off< cds::backoff::yield >
        > MichaelDequeL_PTB_yield ;

        typedef details::MichaelDequeL< cds::gc::PTB, T
            ,cds::opt::stat<cds::intrusive::michael_deque::stat<> >
        > MichaelDequeL_PTB_stat ;


    // MichaelDeque, right side
        typedef details::MichaelDequeR< cds::gc::HP, T> MichaelDequeR_HP ;
        typedef details::MichaelDequeR< cds::gc::HP, T
            ,cds::opt::memory_model<cds::opt::v::sequential_consistent>
        > MichaelDequeR_HP_seqcst ;

        typedef details::MichaelDequeR< cds::gc::HP, T
            ,cds::opt::item_counter< cds::atomicity::item_counter >
        > MichaelDequeR_HP_ic ;

        typedef details::MichaelDequeR< cds::gc::HP, T
            ,cds::opt::back_off<
            cds::backoff::exponential<
            cds::backoff::pause,
            cds::backoff::yield
            >
            >
        > MichaelDequeR_HP_exp ;

        typedef details::MichaelDequeR< cds::gc::HP, T
            ,cds::opt::back_off< cds::backoff::yield >
        > MichaelDequeR_HP_yield ;

        typedef details::MichaelDequeR< cds::gc::HP, T
            ,cds::opt::stat< cds::intrusive::michael_deque::stat<> >
        > MichaelDequeR_HP_stat ;

        typedef details::MichaelDequeR< cds::gc::PTB, T> MichaelDequeR_PTB ;
        typedef details::MichaelDequeR< cds::gc::PTB, T
            ,cds::opt::memory_model<cds::opt::v::sequential_consistent>
        > MichaelDequeR_PTB_seqcst ;

        typedef details::MichaelDequeR< cds::gc::PTB, T
            ,cds::opt::item_counter< cds::atomicity::item_counter >
        > MichaelDequeR_PTB_ic ;

        typedef details::MichaelDequeR< cds::gc::PTB, T
            ,cds::opt::back_off<
                cds::backoff::exponential<
                    cds::backoff::pause,
                    cds::backoff::yield
                >
            >
        > MichaelDequeR_PTB_exp ;

        typedef details::MichaelDequeR< cds::gc::PTB, T
            ,cds::opt::stat< cds::intrusive::deque_stat<> >
        > MichaelDequeR_PTB_stat ;

        typedef details::MichaelDequeR< cds::gc::PTB, T
            ,cds::opt::back_off< cds::backoff::yield >
        > MichaelDequeR_PTB_yield ;

    };
} // namespace stack

namespace std {
    inline ostream& operator <<( ostream& o, cds::intrusive::stack_stat<> const& s )
    {
        return o << "\tStatistics:\n"
            << "\t\t           Push: " << s.m_PushCount.get()   << "\n"
            << "\t\t            Pop: " << s.m_PopCount.get()    << "\n"
            << "\t\tPush contention: " << s.m_PushRace.get()    << "\n"
            << "\t\t Pop contention: " << s.m_PopRace.get()     << "\n"
            ;
    }

    inline ostream& operator <<( ostream& o, cds::intrusive::stack_dummy_stat const& s )
    {
        return o ;
    }

}

#endif // #ifndef __UNIT_STACK_TYPES_H
