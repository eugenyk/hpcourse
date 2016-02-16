//$$CDS-header$$

#ifndef __CDS_INTRUSIVE_QUEUE_STAT_H
#define __CDS_INTRUSIVE_QUEUE_STAT_H

#include <cds/cxx11_atomic.h>

namespace cds { namespace intrusive {

    /// Queue internal statistics. May be used for debugging or profiling
    /** @ingroup cds_intrusive_helper
        Template argument \p Counter defines type of counter.
        Default is cds::atomicity::event_counter, that is weak, i.e. it is not guaranteed
        strict event counting.
        You may use stronger type of counter like as cds::atomicity::item_counter,
        or even integral type, for example, \p int.
    */
    template <typename Counter = cds::atomicity::event_counter >
    struct queue_stat
    {
        typedef Counter     counter_type    ;   ///< Counter type

        counter_type m_EnqueueCount      ;  ///< Enqueue call count
        counter_type m_DequeueCount      ;  ///< Dequeue call count
        counter_type m_EnqueueRace       ;  ///< Count of enqueue race conditions encountered
        counter_type m_DequeueRace       ;  ///< Count of dequeue race conditions encountered
        counter_type m_AdvanceTailError  ;  ///< Count of "advance tail failed" events
        counter_type m_BadTail           ;  ///< Count of events "Tail is not pointed to the last item in the queue"

        /// Register enqueue call
        void onEnqueue()                { ++m_EnqueueCount; }
        /// Register dequeue call
        void onDequeue()                { ++m_DequeueCount; }
        /// Register enqueue race event
        void onEnqueueRace()            { ++m_EnqueueRace; }
        /// Register dequeue race event
        void onDequeueRace()            { ++m_DequeueRace; }
        /// Register "advance tail failed" event
        void onAdvanceTailFailed()      { ++m_AdvanceTailError; }
        /// Register event "Tail is not pointed to last item in the queue"
        void onBadTail()                { ++m_BadTail; }
    };

    /// Dummy queue statistics - no counting is performed. Support interface like \ref queue_stat
    /** @ingroup cds_intrusive_helper
    */
    struct queue_dummy_stat
    {
        //@cond
        void onEnqueue()                {}
        void onDequeue()                {}
        void onEnqueueRace()            {}
        void onDequeueRace()            {}
        void onAdvanceTailFailed()      {}
        void onBadTail()                {}
        //@endcond
    };


}}  // namespace cds::intrusive


#endif  // #ifndef __CDS_INTRUSIVE_QUEUE_STAT_H
