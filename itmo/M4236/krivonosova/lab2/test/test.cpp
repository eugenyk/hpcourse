#include <iostream>

#include <cds/gc/hp.h>
#include <cds/details/marked_ptr.h>
#include <cds/init.h>
#include <vector>
#include <zconf.h>
#include "../src/queue.h"

void producer(lock_free_priority_queue<int> &queue, int n)
{
    cds::threading::Manager::attachThread();
    for (int i = 0; i < n; ++i)
        queue.enqueue(i);

    cds::threading::Manager::detachThread();
}

void consumer(lock_free_priority_queue<int> &queue, int n)
{
    cds::threading::Manager::attachThread();
    int cur_res = 0;
    for (int i = 0; i < n; ++i)
        queue.dequeue(cur_res);

    cds::threading::Manager::detachThread();
}

void test_enqueue()
{
    std::cout << "== test_enqueue ==" << std::endl;

    lock_free_priority_queue<int> my_queue;
    std::vector<std::thread> threads;
    int enq_count = 10000;
    int threads_count = 4;

    for (int i = 0; i < threads_count; ++i)
        threads.emplace_back(producer, std::ref(my_queue), enq_count);

    for (auto& thread : threads)
        thread.join();

    std::cout << "enqueues: " << enq_count << std::endl;
    std::cout << "result size of my_queue: " << my_queue.size() << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << std::endl;
}

void test_dequeue()
{
    std::cout << "== test_dequeue ==" << std::endl;

    lock_free_priority_queue<int> my_queue;
    std::vector<std::thread> threads;
    int enq_count = 5 * 10000;
    int deq_count = 10000;
    int threads_count = 4;

    for (int i = 0; i < enq_count; ++i) {
        int val = rand();
        my_queue.enqueue(val);

    }

    std::cout << "equeues elements: " << my_queue.size() << std::endl;

    for (int i = 0; i < threads_count; ++i)
        threads.emplace_back(consumer, std::ref(my_queue), deq_count);

    for (auto& thread : threads)
        thread.join();

    std::cout << "dequeues elements: " << deq_count << std::endl;
    std::cout << "consumers: " << threads_count << std::endl;
    std::cout << "remaining elements:" << my_queue.size() << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << std::endl;
}

void test_enq_deq()
{
    std::cout << "== test_enq_deq ==" << std::endl;

    lock_free_priority_queue<int> my_queue;
    std::vector<std::thread> threads;
    int enq_elements = 3 * 10000 ;
    int deq_elements = 4 * 10000 ;
    int producers_count = 2;
    int consumers_count = 2;

    std::cout << "producers: " << producers_count << std::endl;
    std::cout << "consumers: " << consumers_count << std::endl;
    std::cout << "enqueues elements: " << enq_elements << std::endl;
    std::cout << "dequeues elements: " << deq_elements << std::endl;

    for (int i = 0; i < producers_count; ++i)
        threads.emplace_back(producer, std::ref(my_queue), enq_elements);

    for (int i = 0; i < consumers_count; ++i)
        threads.emplace_back(consumer, std::ref(my_queue), deq_elements);

    for (auto& thread : threads)
        thread.join();

    std::cout << "remaining elements: " << my_queue.size() << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << std::endl;
}


int main()
{
    cds::Initialize();
    {
        cds::gc::HP hpGC;
        cds::threading::Manager::attachThread();
        test_enqueue();
        test_dequeue();
        test_enq_deq();
    }
    cds::Terminate();
    return 0;
}
