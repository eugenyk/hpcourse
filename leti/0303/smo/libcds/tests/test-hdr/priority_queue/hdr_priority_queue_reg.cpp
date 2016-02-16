//$$CDS-header$$

#include "priority_queue/hdr_intrusive_pqueue.h"
#include "priority_queue/hdr_pqueue.h"

namespace priority_queue {

    size_t intrusive_pqueue::disposer::m_nCallCount = 0 ;
    size_t intrusive_pqueue::another_disposer::m_nCallCount = 0 ;

    size_t pqueue::disposer::m_nCallCount = 0 ;

} // namespace priority_queue


CPPUNIT_TEST_SUITE_REGISTRATION_(priority_queue::IntrusivePQueueHdrTest, s_IntrusivePQueueHdrTest)      ;
CPPUNIT_TEST_SUITE_REGISTRATION_(priority_queue::PQueueHdrTest, s_PQueueHdrTest)      ;
