#ifndef michael_deque_h
#define michael_deque_h

#include <atomic>

#include "node.h"
#include "anchor.h"

static const unsigned int c_nIndexMask    = ((unsigned int)(0 - 1)) >> 1 ;
static const unsigned int c_nFlagMask     = ((unsigned int)(1)) << (sizeof(unsigned int) * 8 - 1) ;
static const unsigned int c_nEmptyIndex   = 0   ;

class MichaelDeque
{
private:
    std::atomic<Anchor> m_Anchor;
    enum anchor_status {
        Stable,
        RPush,
        LPush
    };
    
    static anchor_status status (Anchor const& a);
    static unsigned int index( unsigned int i );
    
    void stabilize(Anchor& a);
    void stabilize_front(Anchor &a);
    void stabilize_back(Anchor &a);
public:
    
    /**
     * For tests only
     */
    Node* get_right() {
        Anchor a = m_Anchor.load(std::memory_order::memory_order_acquire);
        return (Node*) index(a.idxRight);
    }
    
    /**
     * For tests only
     */
    Node* get_left() {
        Anchor a = m_Anchor.load(std::memory_order::memory_order_acquire);
        return (Node*) index(a.idxLeft);
    }
    
    void push_back( int& data);
    void push_front( int& data);
    int pop_back();
    int pop_front();
};

#endif /* michael_deque_h */
