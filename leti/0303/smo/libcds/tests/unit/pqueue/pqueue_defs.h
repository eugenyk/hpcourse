//$$CDS-header$$

#ifndef _CDSUNIT_PQUEUE_PQUEUE_DEFS_H
#define _CDSUNIT_PQUEUE_PQUEUE_DEFS_H

// MSPriorityQueue
#define CDSUNIT_DECLARE_MSPriorityQueue \
    TEST_BOUNDED(MSPriorityQueue_static_less)   \
    TEST_BOUNDED(MSPriorityQueue_static_cmp)    \
    TEST_BOUNDED(MSPriorityQueue_static_mutex)  \
    TEST_BOUNDED(MSPriorityQueue_dyn_less)      \
    TEST_BOUNDED(MSPriorityQueue_dyn_cmp)       \
    TEST_BOUNDED(MSPriorityQueue_dyn_mutex)
#define CDSUNIT_TEST_MSPriorityQueue    \
    CPPUNIT_TEST(MSPriorityQueue_static_less)   \
    CPPUNIT_TEST(MSPriorityQueue_static_cmp)    \
    CPPUNIT_TEST(MSPriorityQueue_static_mutex)  \
    CPPUNIT_TEST(MSPriorityQueue_dyn_less)      \
    CPPUNIT_TEST(MSPriorityQueue_dyn_cmp)       \
    CPPUNIT_TEST(MSPriorityQueue_dyn_mutex)


// EllenBinTree
#ifdef CDS_URCU_SIGNAL_HANDLING_ENABLED
#   define CDSUNIT_DECLARE_EllenBinTree_RCU_signal  \
        TEST_CASE(EllenBinTree_RCU_shb) \
        TEST_CASE(EllenBinTree_RCU_sht)
#   define CDSUNIT_TEST_EllenBinTree_RCU_signal \
        CPPUNIT_TEST(EllenBinTree_RCU_shb)  \
        CPPUNIT_TEST(EllenBinTree_RCU_sht)
#else
#   define CDSUNIT_DECLARE_EllenBinTree_RCU_signal
#   define CDSUNIT_TEST_EllenBinTree_RCU_signal
#endif

#define CDSUNIT_DECLARE_EllenBinTree    \
    TEST_CASE(EllenBinTree_HP)          \
    TEST_CASE(EllenBinTree_PTB)         \
    TEST_CASE(EllenBinTree_RCU_gpi)     \
    TEST_CASE(EllenBinTree_RCU_gpb)     \
    TEST_CASE(EllenBinTree_RCU_gpt)     \
    CDSUNIT_DECLARE_EllenBinTree_RCU_signal
#define CDSUNIT_TEST_EllenBinTree       \
    CPPUNIT_TEST(EllenBinTree_HP)       \
    CPPUNIT_TEST(EllenBinTree_PTB)      \
    CPPUNIT_TEST(EllenBinTree_RCU_gpi)  \
    CPPUNIT_TEST(EllenBinTree_RCU_gpb)  \
    CPPUNIT_TEST(EllenBinTree_RCU_gpt)  \
    CDSUNIT_TEST_EllenBinTree_RCU_signal


// Std::priority_queue
#define CDSUNIT_DECLARE_StdPQueue       \
    TEST_CASE(StdPQueue_vector_spin)    \
    TEST_CASE(StdPQueue_vector_mutex)   \
    TEST_CASE(StdPQueue_deque_spin)     \
    TEST_CASE(StdPQueue_deque_mutex)
#define CDUNIT_TEST_StdPQueue           \
    CPPUNIT_TEST(StdPQueue_vector_spin) \
    CPPUNIT_TEST(StdPQueue_vector_mutex)\
    CPPUNIT_TEST(StdPQueue_deque_spin)  \
    CPPUNIT_TEST(StdPQueue_deque_mutex)


#endif // #ifndef _CDSUNIT_PQUEUE_PQUEUE_DEFS_H
