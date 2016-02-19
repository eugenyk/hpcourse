#ifndef Anchor_h
#define Anchor_h

#include <libkern/OSAtomic.h>

#include <iostream>

struct Node {
    Node *right, *left;
    int data;
};

enum StatusType {
    STABLE = 1,
    RPUSH = 2,
    LPUSH =3
};

class Anchor {
public:
    
    long long prepareStatus(long long state, StatusType _status) {
        long long newState = state & (0xFFFFFFFFFFFFFF00);
        switch (_status) {
            case STABLE:
                newState = newState | 0b1;
                break;
            case RPUSH:
                newState = newState | 0b10;
                break;
            case LPUSH:
                newState = newState | 0b11;
                break;
        }
        return newState;
    }
    
    void preparePage(Node *address) {
        long long page = ((long long) address) & 0xFFFFFFFFFF000000;
        
        if (0 != this->page && page != this->page) {
            std::cout << "mismatch page!" << std::endl;
        }
        
        this->page = page;
    }
    
    long long incr(long long oldState) {
        unsigned char counter = (char) ((oldState & 0xFF00000000000000) >> 56);
        long long newState = 0xFFFFFFFFFFFFFF & oldState;
        counter++;
        newState |= (((long long)counter & 0xf) << 56);
        return newState;
    }
    
    
    long long prepareRight(long long oldState, Node *address) {
        long long newState;
        
        preparePage(address);
        
        oldState = oldState & 0xFFFFFFFF000000FF;
        newState = ((long long)address & (0xFFFFFF)) << 8;
        newState |= oldState;
        
        Node *tmp = getRight(newState);
        
        if (tmp != address){
            std::cout << "mismatch address!" << std::endl;
        }
        
        return newState;
    }
    
    Node* getRight(long long state) {
        Node *right = NULL;
        
        long long rightAddr = state & 0xFFFFFF00;
        
        if (!rightAddr) {
            return NULL;
        }
        
        right = (Node *) (this->page | (rightAddr >> 8));
        return right;
    }
    
    long long prepareLeft(long long oldState, Node *address) {
        long long newState;
        
        preparePage(address);
        
        newState = oldState & 0xFFFFFFFF;
        newState = ((long long)address & (0xFFFFFF)) << 32;
        newState |= oldState;
        
        return newState;
    }
    
    Node* getLeft(long long state) {
        Node *left = NULL;
        
        long long leftAddr = state & (0xFFFFFF << 32);
        
        if (!leftAddr) {
            return NULL;
        }
        
        left = (Node *) (this->page | (leftAddr >> 32));
        return left;
    }
    
    StatusType getStatus(long long state) {
        int status = state & 0b11;
        switch (status) {
            case 0b1:
                return STABLE;
            case 0b10:
                return RPUSH;
                break;
            case 0b11:
                return LPUSH;
                break;
        }
        return STABLE;
    }
    
public:
    long long page;
    /**
     * min -> max
     * 2 bytes for STATUS
     * 24 bytes for LEFT
     * 24 bytes for RIGHT
     */
    long long status;
    
    Anchor() {
        page = 0;
        status = 0;
        setStatus(STABLE);
    }
    
    bool setStatus(StatusType _status) {
        long long prevStatus, newStatus;

        prevStatus = this->status;
        newStatus = prepareStatus(this->status, _status);
        
        return OSAtomicCompareAndSwap64(prevStatus, newStatus, &this->status);
    }
    
    
    StatusType getStatus() {
        int status = this->status & 0b11;
        switch (status) {
            case 0b1:
                return STABLE;
            case 0b10:
                return RPUSH;
                break;
            case 0b11:
                return LPUSH;
                break;
        }
        return STABLE;
    }
    
    bool setLeft(Node *left) {
        long long prevStatus, newStatus;
        
        prevStatus = this->status;
        newStatus = prepareLeft(prevStatus, left);
            
        return OSAtomicCompareAndSwap64(prevStatus, newStatus, &this->status);
    }
    

    bool setRightAndStatus(Node *right, StatusType status) {
        long long prevStatus, newStatus;
        
        prevStatus = this->status;
        newStatus = prepareRight(prevStatus, right);
        newStatus = prepareStatus(newStatus, status);
        
        return OSAtomicCompareAndSwap64(prevStatus, newStatus, &this->status);
    }
    
    bool update(long long newStatus) {
        return OSAtomicCompareAndSwap64(this->status, newStatus, &this->status);
    }
    
    bool setRight(Node *right) {
        long long prevStatus, newStatus;
        
        prevStatus = this->status;
        newStatus = prepareRight(prevStatus, right);
        
        return OSAtomicCompareAndSwap64(prevStatus, newStatus, &this->status);
    }
    
    bool setLeftRight(Node *left, Node *right) {
        long long prevStatus, newStatus;
        
        prevStatus = this->status;
        newStatus = prepareRight(prevStatus, right);
        newStatus = prepareLeft(newStatus, left);
        
        return OSAtomicCompareAndSwap64(prevStatus, newStatus, &this->status);
    }
    
    
    
//    bool setRightAndStatus(Node *right, StatusType status) {
//        long long prevStatus, newStatus;
//        
//        this->page = ((long long) right) & 0xFFFFFFFF00000000;
//        
//        prevStatus = this->status;
//        newStatus = prevStatus & 0xFFFFFFFF00000000;
//        newStatus = ((long long)right & (0xFFFFFF)) << 2;
//        
//        switch (status) {
//            case STABLE:
//                newStatus = newStatus | 0b1;
//                break;
//            case RPUSH:
//                newStatus = newStatus | 0b10;
//                break;
//            case LPUSH:
//                newStatus = newStatus | 0b11;
//                break;
//        }
//        
//        if (OSAtomicCompareAndSwap64(prevStatus, newStatus, &this->status)) {
//            return true;
//        }
//        
//        return false; //wtf?
//    }
    
    Node* getRight() {
        Node *right = NULL;
        
        long long status = this->status;
        long long rightAddr = status & 0xFFFFFF00;
        
        if (!rightAddr) {
            return NULL;
        }
        
        right = (Node *) (this->page | (rightAddr >> 2));
        return right;
    }
    
    Node* getLeft() {
        Node *left = NULL;
        
        long long status = this->status;
        long long leftAddr = status & (0xFFFFFF << 26);
        
        if (!leftAddr) {
            return NULL;
        }
        
        left = (Node *) (this->page | (leftAddr >> 26));
        return left;
    }
};

#endif /* Anchor_h */
