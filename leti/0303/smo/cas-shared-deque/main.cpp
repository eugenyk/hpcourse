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
#include <thread>
#include <atomic>
#include <vector>

#include "michael_deque.h"

#define SIZE 10000 // count of elements by each thread
#define THREADS 50 // count of threads


std::atomic<Anchor> anchor;

MichaelDeque deque;

enum testAlgoritm{
    TEST_ONLY_BACK,
    TEST_ONLY_FRONT,
    TEST_RANDOM
};


void Pusher(int th_id, int size, testAlgoritm algorithm) {
    int data;
    for (int i = 0; i < size; i++) {
        data = th_id * SIZE + i;
        
        switch (algorithm) {
            case TEST_ONLY_BACK:
                 deque.push_back(data);
                break;
            case TEST_ONLY_FRONT:
                 deque.push_front(data);
                break;
            case TEST_RANDOM:
                (rand() % 2 == 0) ? deque.push_front(data) : deque.push_back(data);
                break;
        }
       
    }
}

void test_push(testAlgoritm algorithm){
    switch (algorithm) {
        case TEST_ONLY_BACK:
            std::cout << "push-back test: ";
            break;
        case TEST_ONLY_FRONT:
            std::cout << "push-front test: ";
            break;
        case TEST_RANDOM:
            std::cout << "push-random test: ";
            break;
    }
    
    std::thread threads[THREADS];
    
    for (int i = 0; i < THREADS; i++) {
        threads[i] = std::thread(Pusher, i, SIZE, algorithm);
    }
    
    for (int i = 0; i < THREADS; i++) {
        threads[i].join();
    }
    
    Node* node = deque.get_right();
    
    int matrix[THREADS*SIZE];
    
    for (int i = 0; i < THREADS*SIZE; i++) {
        matrix[i] = 0;
    }
    
    int n,m, errors = 0;
    
    while (node) {
        n = node->data % SIZE;
        m = node->data / SIZE;
        matrix[m*SIZE+n] = 1;
//        std::cout << node->data << ":";
        node = (Node*) (node->m_Links.load(std::memory_order::memory_order_acquire).idxLeft & c_nIndexMask);
    }
    
    for (m = 0; m < THREADS; m++) {
        for (n = 0; n < SIZE; n++) {
            if (!matrix[m*SIZE+n]) {
//                std::cout << "wrong at: "<< m << "x" << n << std::endl;
                errors++;
            }
        }
    }
    
    node = deque.get_left();
    
    for (m = 0; m < THREADS * SIZE; m++)
        matrix[m] = 0;
    
    while (node) {
        n = node->data % SIZE;
        m = node->data / SIZE;
        matrix[m*SIZE+n] = 1;
//        std::cout << node->data << ":";
        node = (Node*) (node->m_Links.load(std::memory_order::memory_order_acquire).idxRight & c_nIndexMask);
    }
    
    for (m = 0; m < THREADS; m++) {
        for (n = 0; n < SIZE; n++) {
            if (!matrix[m*SIZE+n]) {
//                std::cout << "wrong at: "<< m << "x" << n << std::endl;
                errors++;
            }
        }
    }
    
    
    if (!errors)
        std::cout << "PASSED"<< std::endl;
    else
        std::cout << "FAILED (" << errors << " errors)" << std::endl;

}




std::vector<int> popResults;
std::mutex popMutex;
void Popper(testAlgoritm algorithm)
{
    
    int res = 0, i = 0;
    std::vector<int> results;
    
    while(1) {
        switch (algorithm) {
            case TEST_ONLY_BACK:
                res = deque.pop_back();
                break;
            case TEST_ONLY_FRONT:
                res = deque.pop_front();
                break;
            case TEST_RANDOM:
                res = (rand() % 2 == 0) ? deque.pop_front() : deque.pop_back();
                break;
        }
        
        if (res == -1) {
            break;
        } else {
            results.push_back(res);
        }
    }
    
    popMutex.lock();
    for (i = 0; i < results.size(); i++)
        popResults.push_back(results.at(i));
    popMutex.unlock();
}

void test_pop(testAlgoritm algorithm)
{
    switch (algorithm) {
        case TEST_ONLY_BACK:
            std::cout << "pop-back test: ";
            break;
        case TEST_ONLY_FRONT:
            std::cout << "pop-front test: ";
            break;
        case TEST_RANDOM:
            std::cout << "pop-random test: ";
            break;
    }
    
    std::thread threads[THREADS];
    
    for (int i = 0; i < THREADS; i++) {
        threads[i] = std::thread(Popper, algorithm);
    }
    
    for (int i = 0; i < THREADS; i++) {
        threads[i].join();
    }
    
    int matrix[THREADS*SIZE];
    
    for (int i = 0; i < THREADS*SIZE; i++) {
        matrix[i] = 0;
    }
    
    int n,m, errors = 0, res;
    
    for (int i = 0; i < popResults.size(); i++)
    {
        res = popResults.at(i);
        n = res % SIZE;
        m = res / SIZE;
        matrix[m*SIZE+n] = 1;
    }
    
    for (m = 0; m < THREADS; m++) {
        for (n = 0; n < SIZE; n++) {
            if (!matrix[m*SIZE+n]) {
                //                std::cout << "wrong at: "<< m << "x" << n << std::endl;
                errors++;
            }
        }
    }
    
    if (!errors)
        std::cout << "PASSED"<< std::endl;
    else
        std::cout << "FAILED (" << errors << " errors)" << std::endl;
}

int main() {
    test_push(TEST_ONLY_BACK);
    test_pop(TEST_ONLY_BACK);
    
    test_push(TEST_ONLY_FRONT);
    test_pop(TEST_ONLY_FRONT);
    
    test_push(TEST_ONLY_BACK);
    test_pop(TEST_RANDOM);
    
    test_push(TEST_RANDOM);
    test_pop(TEST_ONLY_BACK);
    
    test_push(TEST_RANDOM);
    test_pop(TEST_ONLY_FRONT);
    
    test_push(TEST_RANDOM);
    test_pop(TEST_RANDOM);
}
