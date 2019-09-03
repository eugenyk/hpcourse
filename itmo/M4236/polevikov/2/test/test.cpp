#include <vector>
#include <random>
#include <thread>
#include <cassert>
#include <iostream>

#include <cds/init.h>
#include <cds/gc/hp.h>

#include "../src/lockfree_priority_queue.h"

void producer(lockfree_priority_queue<int> &queue, int num_push)
{
    cds::threading::Manager::attachThread();
    for (int i = 0; i < num_push; ++i)
    {
        queue.push(rand());
    }
    cds::threading::Manager::detachThread();
}

void consumer(lockfree_priority_queue<int> &queue, int num_pop)
{
    cds::threading::Manager::attachThread();
    for (int i = 0; i < num_pop; ++i)
    {
        queue.pop();
    }
    cds::threading::Manager::detachThread();
}

void test_push()
{
    std::cout << "Started \"push\" testing" << std::endl;
    cds::threading::Manager::attachThread();

    lockfree_priority_queue<int> queue;
    std::vector<std::thread> threads;
    int num_push = 5 * 100 * 100;
    int num_threads = 4;

    std::cout << "Threads: " << num_threads << std::endl;
    std::cout << "Push operations: " << num_push << std::endl;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(producer, std::ref(queue), num_push);
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    queue_stats stats = queue.stats();
    stats.print();

    cds::threading::Manager::detachThread();
    std::cout << "Finished \"push\" testing" << std::endl;
}

void test_pop()
{
    std::cout << std::endl << "Started \"pop\" testing" << std::endl;
    cds::threading::Manager::attachThread();

    lockfree_priority_queue<int> queue;
    std::vector<std::thread> threads;
    int num_pop = 4 * 100 * 100;
    int num_push = 3 * 100 * 100;
    int num_threads = 4;

    std::cout << "Initial queue size: " << num_pop << std::endl;
    std::cout << "Threads: " << num_threads << std::endl;
    std::cout << "Pop operations: " << num_push << std::endl;

    for (int i = 0; i < num_pop; ++i)
    {
        queue.push(rand());
    }

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(consumer, std::ref(queue), num_push);
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    queue_stats stats = queue.stats();
    stats.print();

    cds::threading::Manager::detachThread();
    std::cout << "Finished \"pop\" testing" << std::endl;
}

void test_pushpop()
{
    std::cout << std::endl << "Started \"pushpop\" testing" << std::endl;
    cds::threading::Manager::attachThread();

    lockfree_priority_queue<int> queue;
    std::vector<std::thread> threads;
    int num_pop = 40 * 100 * 100;
    int num_push = 30 * 100 * 100;
    int num_producers = 4;
    int num_consumers = 4;

    std::cout << "Producers: " << num_producers << std::endl;
    std::cout << "Consumers: " << num_consumers << std::endl;
    std::cout << "Push operations: " << num_push << std::endl;
    std::cout << "Pop operations: " << num_pop << std::endl;

    for (int i = 0; i < num_producers; ++i)
    {
        threads.emplace_back(producer, std::ref(queue), num_push);
    }
    for (int i = 0; i < num_consumers; ++i)
    {
        threads.emplace_back(consumer, std::ref(queue), num_pop);
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    queue_stats stats = queue.stats();
    stats.print();

    cds::threading::Manager::detachThread();
    std::cout << "Finished \"pushpop\" testing" << std::endl;
}

void run_tests()
{
    test_push();
    test_pop();
    test_pushpop();
}

int main()
{
    srand(time(0));
    cds::Initialize();
    {
        cds::gc::HP hpGC;
        run_tests();
    }
    cds::Terminate();
    return 0;
}
