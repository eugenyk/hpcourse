#ifndef node_h
#define node_h

#include "anchor.h"

struct Node {
    int data;
    
    std::atomic<Anchor> m_Links;
    
    Node()
    {
        m_Links.store(Anchor(), std::memory_order::memory_order_release);
    }
    
    explicit Node (Anchor const& a):m_Links()
    {
        m_Links.store(a, std::memory_order::memory_order_release);
    }
};

#endif /* node_h */
