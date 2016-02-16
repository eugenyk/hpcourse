//$$CDS-header$$

#ifndef __CDS_INTRUSIVE_STACK_STAT_H
#define __CDS_INTRUSIVE_STACK_STAT_H

#include <cds/cxx11_atomic.h>

namespace cds { namespace intrusive {

    /// Stack internal statistics. May be used for debugging or profiling
    /** @ingroup cds_intrusive_helper
        Template argument \p Counter defines type of counter.
        Default is cds::atomicity::event_counter, that is weak, i.e. it is not guaranteed
        strict event counting.
        You may use stronger type of counter like as cds::atomicity::item_counter,
        or even integral type, for example, \p int.
    */
    template <typename Counter = cds::atomicity::event_counter >
    struct stack_stat
    {
        typedef Counter     counter_type    ;   ///< Counter type

        counter_type m_PushCount        ;  ///< Push call count
        counter_type m_PopCount         ;  ///< Pop call count
        counter_type m_PushRace         ;  ///< Count of push race conditions encountered
        counter_type m_PopRace          ;  ///< Count of pop race conditions encountered

        /// Register push call
        void onPush()               { ++m_PushCount; }
        /// Register pop call
        void onPop()                { ++m_PopCount; }
        /// Register push race event
        void onPushRace()           { ++m_PushRace; }
        /// Register pop race event
        void onPopRace()            { ++m_PopRace; }
    };

    /// Dummy stack statistics - no counting is performed. Support interface like \ref stack_stat
    /** @ingroup cds_intrusive_helper
    */
    struct stack_dummy_stat
    {
        //@cond
        void onPush()       {}
        void onPop()        {}
        void onPushRace()   {}
        void onPopRace()    {}
        //@endcond
    };


}}  // namespace cds::intrusive


#endif  // #ifndef __CDS_INTRUSIVE_STACK_STAT_H
