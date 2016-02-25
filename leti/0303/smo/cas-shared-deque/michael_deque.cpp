#include "michael_deque.h"

MichaelDeque::anchor_status MichaelDeque::status (Anchor const& a) {
    if ( a.idxLeft & c_nFlagMask )
        return LPush ;
    if ( a.idxRight & c_nFlagMask )
        return RPush ;
    return Stable ;
}

unsigned int MichaelDeque::index( unsigned int i )
{
    return i & c_nIndexMask ;
}

void MichaelDeque::stabilize(Anchor& a)
{
    switch ( status(a)) {
        case LPush:
            stabilize_front(a)   ;
            break;
        case RPush:
            stabilize_back(a)  ;
            break;
        default:
            break;
    }
}

void MichaelDeque::stabilize_front(Anchor &a)
{
    Node *pLeft, *pRight;
    
    unsigned int const idxLeft  = index( a.idxLeft ) ;
    unsigned int const idxRight = index( a.idxRight ) ;
    
    pLeft = (Node *) idxLeft;
    pRight = (Node *) idxRight;
    
    if ( m_Anchor.load( std::memory_order::memory_order_acquire ) != a )
        return ;
    
    unsigned int idxPrev = index(pLeft->m_Links.load(std::memory_order::memory_order_relaxed).idxRight);
    Node *pPrev = (Node *) idxPrev;
    
    if ( m_Anchor.load( std::memory_order::memory_order_acquire ) != a )
        return ;
    
    Anchor prevLinks = pPrev->m_Links.load(std::memory_order::memory_order_acquire);
    if (index(prevLinks.idxLeft) != idxLeft) {
        if (m_Anchor.load(std::memory_order::memory_order_acquire) != a)
            return;
        
        if (!pPrev->m_Links.compare_exchange_strong(prevLinks, Anchor(idxLeft, prevLinks.idxRight), std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed))
            return;
    }
    
    m_Anchor.compare_exchange_weak(a, Anchor(idxLeft, idxRight), std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed);
}

void MichaelDeque::stabilize_back(Anchor &a)
{
    Node *pLeft, *pRight;
    
    unsigned int const idxLeft  = index( a.idxLeft ) ;
    unsigned int const idxRight = index( a.idxRight ) ;
    
    pLeft = (Node *) idxLeft;
    pRight = (Node *) idxRight;
    
    if ( m_Anchor.load( std::memory_order::memory_order_acquire ) != a )
        return ;
    
    unsigned int idxPrev = index(pRight->m_Links.load(std::memory_order::memory_order_relaxed).idxLeft);
    Node *pPrev = (Node *) idxPrev;
    
    if ( m_Anchor.load( std::memory_order::memory_order_acquire ) != a )
        return ;
    
    Anchor prevLinks = pPrev->m_Links.load(std::memory_order::memory_order_acquire);
    
    
    if (index(prevLinks.idxRight) != idxRight) {
        if (m_Anchor.load(std::memory_order::memory_order_acquire) != a)
            return;
        
        if (!pPrev->m_Links.compare_exchange_strong(prevLinks, Anchor(prevLinks.idxLeft, idxRight), std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed))
            return;
    }
    
    m_Anchor.compare_exchange_weak(a, Anchor(idxLeft, idxRight), std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed);
}

void MichaelDeque::push_back( int& data) {
    
    Node* node = new Node();
    node->data = data;
    
    while (1) {
        Anchor a = m_Anchor.load(std::memory_order::memory_order_acquire);
        if (a.idxRight == c_nEmptyIndex) {
            if (m_Anchor.compare_exchange_weak(a, Anchor((int) node | c_nEmptyIndex, (int) node | c_nEmptyIndex), std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed))
                break;
        } else if ( Stable == status(a)) {
            node->m_Links.store(Anchor(a.idxRight, c_nEmptyIndex), std::memory_order::memory_order_release);
            Anchor aNew(a.idxLeft, (int) node | c_nFlagMask);
            if (m_Anchor.compare_exchange_weak(a, aNew, std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed)) {
                stabilize_back(aNew);
                break;
            }
        } else {
            stabilize(a);
        }
    }
}

void MichaelDeque::push_front( int& data) {
    
    Node* node = new Node();
    node->data = data;
    
    while (1) {
        Anchor a = m_Anchor.load(std::memory_order::memory_order_acquire);
        if (a.idxLeft == c_nEmptyIndex) {
            if (m_Anchor.compare_exchange_weak(a, Anchor((int) node | c_nEmptyIndex, (int) node | c_nEmptyIndex), std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed))
                break;
        } else if ( Stable == status(a)) {
            node->m_Links.store(Anchor(c_nEmptyIndex ,a.idxLeft), std::memory_order::memory_order_release);
            Anchor aNew((int) node | c_nFlagMask, a.idxRight);
            if (m_Anchor.compare_exchange_weak(a, aNew, std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed)) {
                stabilize_front(aNew);
                break;
            }
        } else {
            stabilize(a);
        }
    }
}

int MichaelDeque::pop_back() {
    
    Anchor a;
    
    while (true) {
        a = m_Anchor.load(std::memory_order::memory_order_acquire);
        
        if (c_nEmptyIndex == a.idxRight) {
            return -1;
        }
        
        if (a.idxRight == a.idxLeft) {
            if (m_Anchor.compare_exchange_weak(a, Anchor(c_nEmptyIndex, c_nEmptyIndex), std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed))
                break;
        } else if (Stable == status(a)) {
            unsigned int idxLeft = index(a.idxLeft), idxRight = index(a.idxRight);
            Node *pLeft, *pRight;
            pLeft = (Node*) idxLeft;
            pRight = (Node*) idxRight;
            
            if (m_Anchor.load(std::memory_order::memory_order_acquire) != a) {
                continue;
            }
            
            unsigned int nPrev = pRight->m_Links.load(std::memory_order::memory_order_acquire).idxLeft;
            if (m_Anchor.compare_exchange_weak(a, Anchor(a.idxLeft, nPrev), std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed))
                //return pRight->data;
                break;
        } else {
            stabilize(a);
        }
    }
    
    if (a.idxRight != c_nEmptyIndex){
        Node *res = (Node*) index(a.idxRight);
        return res->data;
    }
    
    return -1;
}

int MichaelDeque::pop_front() {
    
    Anchor a;
    
    while (true) {
        a = m_Anchor.load(std::memory_order::memory_order_acquire);
        
        if (c_nEmptyIndex == a.idxLeft) {
            return -1;
        }
        
        if (a.idxRight == a.idxLeft) {
            if (m_Anchor.compare_exchange_weak(a, Anchor(c_nEmptyIndex, c_nEmptyIndex), std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed))
                break;
        } else if (Stable == status(a)) {
            unsigned int idxLeft = index(a.idxLeft), idxRight = index(a.idxRight);
            Node *pLeft, *pRight;
            pLeft = (Node*) idxLeft;
            pRight = (Node*) idxRight;
            
            if (m_Anchor.load(std::memory_order::memory_order_acquire) != a) {
                continue;
            }
            
            unsigned int nPrev = pLeft->m_Links.load(std::memory_order::memory_order_acquire).idxRight;
            if (m_Anchor.compare_exchange_weak(a, Anchor(nPrev, a.idxRight), std::memory_order::memory_order_release, std::memory_order::memory_order_relaxed))
                //return pRight->data;
                break;
        } else {
            stabilize(a);
        }
    }
    
    if (a.idxLeft != c_nEmptyIndex){
        Node *res = (Node*) index(a.idxLeft);
        return res->data;
    }
    
    return -1;
}