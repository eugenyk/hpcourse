//
// main.cpp
// cas-shared-deque
//
// CAS-Based Lock-Free Algorithm
// for Shared Deques
//
// Created by Dmitry Sereda on 02.02.16.
// Copyright © 2016 Dmitry Sereda. All rights reserved.

//

#include <iostream>
#include <libkern/OSAtomic.h>

#include <thread>



enum StatusType {
    STABLE = 1,
    RPUSH = 2,
    LPUSH =3
};

struct Node {
    Node *right, *left;
    int data;
};

struct Anchor {
    Node *right = NULL, *left = NULL;
    StatusType status = STABLE;
};

Anchor* anchor;

bool cas( volatile int64_t *ptr, int64_t old_val, int64_t new_val) {
    // bool
    //__OSX_AVAILABLE_STARTING(__MAC_10_4, __IPHONE_2_0)
    // bool OSAtomicCompareAndSwap64( int64_t __oldValue, int64_t __newValue, volatile int64_t *__theValue );
    bool b = OSAtomicCompareAndSwap64(old_val, new_val, ptr);;
    return b;
    
}


void StabilizeLeft(Anchor *_anchor){
    Node *prev = _anchor->left->right;
    Node *prevnext = prev->left;
    
    
    if (_anchor->left != anchor->left || _anchor->right != anchor->right || _anchor->status != anchor->status) return;
    
    if (prevnext != _anchor->left) {
        if (_anchor->left != anchor->left || _anchor->right != anchor->right || _anchor->status != anchor->status) return;
        if (cas((int64_t*)&prev->left, (int64_t) prevnext, (int64_t) _anchor->left)) return;
    }
    Anchor *newAnchor = (Anchor*)::operator new(sizeof(Anchor));
    newAnchor->left = anchor->left;
    newAnchor->right = anchor->right;
    newAnchor->status = STABLE;
    cas((int64_t*)&anchor, (int64_t) anchor, (int64_t) newAnchor);
}

void StabilizeRight(Anchor *_anchor){
    Node *prev = _anchor->right->left;
    
    //if (_anchor->left != anchor->left || _anchor->right != anchor->right || _anchor->status != anchor->status)
    if (((int64_t)_anchor->left xor (int64_t)_anchor->right) != ((int64_t)anchor->left xor (int64_t)anchor->right))
        return;
    
    Node *prevnext = prev->right;
    
    if (prevnext != _anchor->right) {
        if (((int64_t)_anchor->left xor (int64_t)_anchor->right) != ((int64_t)anchor->left xor (int64_t)anchor->right))
            return;
        
        if (cas((int64_t*)&prev->right, (int64_t) prevnext, (int64_t) _anchor->right))
            return;
    }
    Anchor *newAnchor = (Anchor*)::operator new(sizeof(Anchor));
    newAnchor->left = _anchor->left;
    newAnchor->right = _anchor->right;
    newAnchor->status = STABLE;
    
    cas((int64_t*)&anchor, (int64_t) anchor, (int64_t) newAnchor);
}

void Stabilize(Anchor *_anchor){
    if (RPUSH == _anchor->status)
        StabilizeRight(_anchor);
    else
    {
        StabilizeLeft(_anchor);
    }
}

void PushRight(int data)
{
    Node* node = (Node*)::operator new(sizeof(Node));
    node->left = NULL;
    node->right = NULL;
    node->data = data;
    
    while(1) {
        Anchor* newAnchor = (Anchor*)::operator new(sizeof(Anchor));
        newAnchor->status = anchor->status;
        newAnchor->left = anchor->left;
        newAnchor->right = anchor->right;
              
        if (NULL == newAnchor->right) {
            
            newAnchor->left = node;
            newAnchor->right = node;
            
            if (cas((int64_t*)&anchor, (int64_t) anchor, (int64_t) newAnchor)){
                return;
            }
        } else if (STABLE == newAnchor->status) {
            newAnchor->right = node;
            newAnchor->status = RPUSH;
            node->left = anchor->right;
            if (cas((int64_t*)&anchor, (int64_t) anchor, (int64_t) newAnchor)) {
                StabilizeRight(newAnchor);
                return;
            }
        } else {
            Stabilize(newAnchor);
        }
        
        
        //::operator delete(anchor);
    }
}

void PushLeft(int data)
{
    Node* node = (Node*)::operator new(sizeof(Node));
    node->left = NULL;
    node->right = NULL;
    node->data = data;
    
    while(1) {
        Anchor* newAnchor = (Anchor*)::operator new(sizeof(Anchor));
        newAnchor->status = anchor->status;
        newAnchor->left = anchor->left;
        newAnchor->right = anchor->right;
        
        if (NULL == newAnchor->left) {
            
            newAnchor->left = node;
            newAnchor->right = node;
            
            if (cas((int64_t*)&anchor, (int64_t) anchor, (int64_t) newAnchor)){
                return;
            }
        } else if (STABLE == newAnchor->status) {
            newAnchor->left = node;
            newAnchor->status = LPUSH;
            node->right = anchor->left;
            if (cas((int64_t*)&anchor, (int64_t) anchor, (int64_t) newAnchor)) {
                StabilizeLeft(newAnchor);
                return;
            }
        } else {
            Stabilize(newAnchor);
        }
        
        
        
    }
    
}



int PopRight() {
    Anchor *newAnchor = (Anchor*) ::operator new(sizeof(Anchor));
    int _return = -1;

    while(1) {
        newAnchor->left = anchor->left;
        newAnchor->right = anchor->right;
        newAnchor->status = anchor->status;
        
        if (NULL == newAnchor->right)
            return -1;
        
        if (newAnchor->left == newAnchor->right) {
            if (newAnchor->left)
                _return = newAnchor->left->data;
            newAnchor->left = NULL;
            newAnchor->right = NULL;
            newAnchor->status = anchor->status;
            if (cas((int64_t*)&anchor, (int64_t) anchor, (int64_t) newAnchor)) {
                break;
            }
        } else if (STABLE == newAnchor->status) {
            Node *node = newAnchor->right;
            Node *prev = node->left;
            newAnchor->right = prev;
            if (cas((int64_t*)&anchor, (int64_t) anchor, (int64_t) newAnchor)) {
                _return = node->data;
                break;
            }
        } else {
            Stabilize(anchor);
        }
    }
    
    return _return;
}

int PopLeft() {
    Anchor *newAnchor = (Anchor*) ::operator new(sizeof(Anchor));
    int _return = -1;
    
    while(1) {
        newAnchor->left = anchor->left;
        newAnchor->right = anchor->right;
        newAnchor->status = anchor->status;
        
        if (NULL == newAnchor->left)
            return -1;
        
        if (newAnchor->left == newAnchor->right) {
            if (newAnchor->left)
                _return = newAnchor->left->data;
            newAnchor->left = NULL;
            newAnchor->right = NULL;
            newAnchor->status = anchor->status;
            if (cas((int64_t*)&anchor, (int64_t) anchor, (int64_t) newAnchor)) {
                break;
            }
        } else if (STABLE == newAnchor->status) {
            Node *node = newAnchor->left;
            Node *prev = node->right;
            newAnchor->left = prev;
            if (cas((int64_t*)&anchor, (int64_t) anchor, (int64_t) newAnchor)) {
                _return = node->data;
                break;
            }
        } else {
            Stabilize(anchor);
        }
    }
    
    return _return;
}

int i, i2;

std::mutex g_lock;

void Pusher(int th_id, int size){
    for (int j = 0 ; j < size; j++) {
//        g_lock.lock();
//        if (rand() % 2 == 1)
            PushRight((th_id * 100) + j);
//        else
//            PushLeft((th_id * 100) + j);
//        g_lock.u1nlock();
    }
}

int main(){
    anchor = (Anchor*)::operator new(sizeof(Anchor));
    anchor->left = NULL;
    anchor->right = NULL;
    anchor->status = STABLE;
    int size = 10;
    
    std::thread thr1(Pusher, 1, size);
    std::thread thr2(Pusher, 2, size);
    std::thread thr3(Pusher, 3, size);
    std::thread thr4(Pusher, 4, size);
    std::thread thr5(Pusher, 5, size);
    std::thread thr6(Pusher, 6, size);
    std::thread thr7(Pusher, 7, size);
    std::thread thr8(Pusher, 0, size);
    thr1.join();
    thr2.join();
    thr3.join();
    thr4.join();
    thr5.join();
    thr6.join();
    thr7.join();
    thr8.join();
    
    int matrix[8*size];
    for (int i = 0; i < 8*size; i++) {
        matrix[i] = 0;
    }
    
//    std::cout << std::endl << "left -> right: " << std::endl;
    Node *a = anchor->left;
    a = anchor->right;
    int n,m;
    while (a && a != a->left) {
//        std::cout << a->data << ":";
        n = a->data % 100;
        m = a->data / 100;
        matrix[m*size+n] = 1;
        a = a->left;
    }
    
    for (m = 0; m < 8; m++) {
        for (n = 0; n < size; n++) {
            if (!matrix[m*size+n]) {
                std::cout << "wrong at: "<< m << "x" << n << std::endl;
            }
        }
     }
}

//int _main(int argc, const char * argv[]) {
//    anchor = (Anchor*)::operator new(sizeof(Anchor));
//    anchor->left = NULL;
//    anchor->right = NULL;
//    anchor->status = STABLE;
//    
//    fork();
//    fork();
//    
//    for (i = 0 ; i < 10; i++) {
//        PushRight(i);
////         if (rand() % 2 == 1)
////         PushRight(i);
////         else
////         PushLeft(i);
//    }
//
//    
//    std::cout << std::endl << "left -> right: " << std::endl;
//    
//
//    Node *a = anchor->left;
//    while (a && a != a->right) {
//        std::cout << a->data << ":";
//        a = a->right;
//    }
//    
//    std::cout << std::endl << "left -> right: " << std::endl;
//    
//    a = anchor->right;
//    while (a && a != a->left) {
//        std::cout << a->data << ":";
//        a = a->left;
//    }
//    
//    std::cout << std::endl ;
//    
//    std::cout << std::endl << "pop right: " << std::endl;
//    
//    while (1) {
//        i = PopRight();
//        if (-1 == i)
//            break;
//        std::cout << i << ":" ;
//    }
//    
//    for (i = 0 ; i < 10; i++) {
//        PushRight(i);
//        //         if (rand() % 2 == 1)
//        //         PushRight(i);
//        //         else
//        //         PushLeft(i);
//    }
//    
//    std::cout << std::endl ;
//    
//    std::cout << std::endl << "left -> right: " << std::endl;
//    
//    a = anchor->right;
//    while (a && a != a->left) {
//        std::cout << a->data << ":";
//        a = a->left;
//    }
//    
//    std::cout << std::endl << "pop left: " << std::endl;
//    
//    while (1) {
//        i = PopLeft();
//        if (-1 == i)
//            break;
//        std::cout << i << ":" ;
//    }
//    
//    std::cout << std::endl ;
//}