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
#include <vector>

#include "Anchor.h"


#define THREADS 3
#define SIZE 10

int amatrix[THREADS * SIZE];


Anchor *anchor;
std::atomic<long long> anchorState;

void PushRight(int data);
void PushLeft(int data);
void Stabilize(long long newAnchorState);
void StabilizeRight(long long newAnchorState);
void StabilizeLeft(long long newAnchorState);

bool cas ( long long __oldValue, long long __newValue, volatile std::atomic<long long> *__theValue ) {
    return OSAtomicCompareAndSwap64((long long)__oldValue, (long long)__newValue, (long long*)__theValue);
}

void PushRight(int data) {
    Node *node = (Node *) ::operator new(sizeof(Node));
    node->data = data;
    
    long long newAnchorState;
    while(1) {
        newAnchorState = anchorState;
        if (NULL == anchor->getRight(newAnchorState)) {
            newAnchorState = anchor->prepareRight(newAnchorState, node);
            newAnchorState = anchor->prepareLeft(newAnchorState, node);
            newAnchorState = anchor->incr(newAnchorState);
            
            if (cas((long long)anchorState, newAnchorState, &anchorState)) {
                return;
            }
        } else if (STABLE == anchor->getStatus(newAnchorState)) {
            node->left = anchor->getRight(newAnchorState);
            newAnchorState = anchor->prepareRight(newAnchorState, node);
            newAnchorState = anchor->prepareStatus(newAnchorState, RPUSH);
            newAnchorState = anchor->incr(newAnchorState);
            if (cas(anchorState, newAnchorState, &anchorState)) {
                StabilizeRight(newAnchorState);
                return;
            }
        } else {
            Stabilize(newAnchorState);
        }
    }
}

void Stabilize(long long newAnchorState) {
    if (RPUSH == anchor->getStatus(newAnchorState)) {
        StabilizeRight(newAnchorState);
    } else {
        StabilizeLeft(newAnchorState);
    }
}



void StabilizeRight(long long newAnchorState) {
    Node *prev = anchor->getRight(newAnchorState)->left;
    
    if (anchorState != newAnchorState)
        return;
    
    Node *prevnext = prev->right;
    
    if (prevnext != anchor->getRight(newAnchorState)) {
        if (anchorState != newAnchorState)
            return;
        if (OSAtomicCompareAndSwap64((long long)prevnext, (long long)anchor->getRight(newAnchorState), (long long*)&prev->right))
            return;
    }
    newAnchorState = anchor->prepareStatus(newAnchorState, STABLE);
    newAnchorState = anchor->incr(newAnchorState);
    cas(anchorState, newAnchorState, &anchorState);
}

void StabilizeLeft(long long newAnchorState) {
    Node *prev = anchor->getLeft(newAnchorState)->right;
    
    if (anchorState != newAnchorState)
        return;
    
    Node *prevnext = prev->left;
    if (prevnext != anchor->getRight(newAnchorState)) {
        if (anchorState != newAnchorState)
            return;
        if (OSAtomicCompareAndSwap64((long long)prevnext, (long long)anchor->getLeft(newAnchorState), (long long*)&prev->left))
            return;
    }
    newAnchorState = anchor->prepareStatus(newAnchorState, STABLE);
    newAnchorState = anchor->incr(newAnchorState);
    cas(anchorState, newAnchorState, &anchorState);
}



void Pusher(int thread_id, int size) {
    for (int i = 0; i < size; i++) {
        PushRight(thread_id * SIZE + i);
    }
}



int main() {
    anchor = new Anchor();
    anchorState = anchor->status;
    anchorState = anchor->prepareStatus(anchorState, STABLE);
    
    int matrix[THREADS * SIZE];
    
    for (int i = 0; i < THREADS * SIZE; i++) {
        matrix[i] = 0;
        amatrix[i] = 0;
    }
    
    std::thread threads[THREADS];
    
    for (int i = 0 ; i < THREADS; i++) {
        threads[i] = std::thread(Pusher, i, SIZE);
    }
    
    for (int i = 0 ; i < THREADS; i++) {
        threads[i].join();
    }
    
    Node *a = anchor->getRight(anchorState);
    
    int n,m;
    while (a && a != a->left) {
//        std::cout << a->data << ":";
        n = a->data % SIZE;
        m = a->data / SIZE;
        matrix[m*SIZE+n] = 1;
        a = a->left;
    }
    
    int errors = 0;
    
    for (int i = 0; i < THREADS * SIZE; i++) {
        if (amatrix[i] != 0) {
            errors++;
//            std::cout << "wrong element: "<< amatrix[i] << std::endl;
        }
    }
    
    if (!errors) {
        std::cout << "All ok!" << std::endl;
    } else {
        std::cout << "mismatched " << errors << " of " << THREADS * SIZE <<  " elements" << std::endl;
    }
    
    
    errors = 0;
    
    for (m = 0; m < THREADS; m++) {
        for (n = 0; n < SIZE; n++) {
            if (!matrix[m*SIZE+n]) {
                std::cout << "wrong at: "<< m << "x" << n;
                if (!amatrix[m*SIZE+n]) std::cout << " + ";
                std::cout << std::endl;
                errors++;
            }
        }
    }
    
    
    
    if (!errors) {
        std::cout << "All ok!" << std::endl;
    } else {
        std::cout << "mismatched " << errors << " of " << THREADS * SIZE <<  " elements" << std::endl;
    }
    
    a = anchor->getLeft(anchorState);
    

    while (a && a != a->left) {
        n = a->data % SIZE;
        m = a->data / SIZE;
        matrix[m*SIZE+n] = 1;
        a = a->right;
    }
    
    errors = 0;
    
    for (m = 0; m < THREADS; m++) {
        for (n = 0; n < SIZE; n++) {
            if (!matrix[m*SIZE+n]) {
                errors++;
            }
        }
    }
    
    if (!errors) {
        std::cout << "All ok!" << std::endl;
    } else {
        std::cout << "mismatched " << errors << " of " << THREADS * SIZE <<  " elements" << std::endl;
    }

}